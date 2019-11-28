#ifndef RYULIB_SOCKETSERVER_HPP
#define RYULIB_SOCKETSERVER_HPP

#include <iostream>
#include <string>
#include <functional>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#define HEADER_SIZE 3
#define PACKET_LIMIT 8192

using namespace std;
using boost::asio::ip::tcp;

#pragma pack(push,1)

typedef struct _PacketHeader {
	unsigned short size;
	char packetType;
} PacketHeader;

typedef struct _Packet {
	unsigned short size;
	char packetType;
	char dataStart;
} Packet;

#pragma pack(pop)

class Connection;

typedef function<void(int, string)> ErrorEvent;
typedef function<void(Connection *)> ServerSocketEvent;
typedef function<void(Connection *, Packet *)> ReceivedEvent;

class Connection : public boost::enable_shared_from_this<Connection> {

	friend class ServerSocket;

public:
	typedef boost::shared_ptr<Connection> pointer;

	static pointer create(boost::asio::io_context& io_context) {
		return pointer(new Connection(io_context));
	}

	tcp::socket& socket() {
		return socket_;
	}

	void write(void* data, int size) {
		boost::asio::async_write(socket_, boost::asio::buffer(data, size),
			boost::bind(&Connection::handle_write, shared_from_this(),
				boost::asio::placeholders::error,
				boost::asio::placeholders::bytes_transferred));
	}

private:
	Connection(boost::asio::io_context& io_context)
		: socket_(io_context) {
	}

	void start() {
		packet_ = (Packet*) data_;
		
		if (on_connected_ != nullptr) on_connected_(this);

		boost::asio::async_read(socket_,
			boost::asio::buffer(data_, HEADER_SIZE),
			boost::bind(&Connection::handle_read_header, shared_from_this(),
				boost::asio::placeholders::error));
	}

	void handle_read_header(const boost::system::error_code& error) {
		if (!error) {
			if (packet_->size > (PACKET_LIMIT - 3)) {
				if (on_error_ != nullptr) on_error_(-1, "packet_->size > (PACKET_LIMIT - 3)");
			}

			boost::asio::async_read(socket_,
				boost::asio::buffer(&packet_->dataStart, packet_->size),
				boost::bind(&Connection::handle_read_body, shared_from_this(),
					boost::asio::placeholders::error));
		} else {
			if (on_disconnected_ != nullptr) on_disconnected_(this);
		}
	}

	void handle_read_body(const boost::system::error_code& error) {
		if (!error) {
			if (on_received_ != nullptr) on_received_(this, packet_);

			boost::asio::async_read(socket_,
				boost::asio::buffer(data_, HEADER_SIZE),
				boost::bind(&Connection::handle_read_header, shared_from_this(),
					boost::asio::placeholders::error));
		} else {
			if (on_disconnected_ != nullptr) on_disconnected_(this);
		}
	}

	void handle_write(const boost::system::error_code& /*error*/, size_t /*bytes_transferred*/) {
	}

	tcp::socket socket_;
	Packet* packet_;
	char data_[PACKET_LIMIT];

	ErrorEvent on_error_ = nullptr;
	ServerSocketEvent on_connected_ = nullptr;
	ServerSocketEvent on_disconnected_ = nullptr;
	ReceivedEvent on_received_ = nullptr;
};

class ServerSocket {
public:
	void start(int port) {
		try {
			acceptor_ = std::make_unique<tcp::acceptor>(io_context_, tcp::endpoint(tcp::v4(), port));
			start_accept();
			io_context_.run();
		} catch (std::exception& e) {
			if (on_error_ != nullptr) on_error_(-1, e.what());
		}
	}

	void setOnError(const ErrorEvent &value) { on_error_ = value; }
	void setOnConnected(const ServerSocketEvent &value) { on_connected_ = value; }
	void setOnDisconnected(const ServerSocketEvent &value) { on_disconnected_ = value; }
	void setOnReceived(const ReceivedEvent &value) { on_received_ = value; }
private:
	void start_accept() {
		Connection::pointer new_connection =
			Connection::create(acceptor_->get_executor().context());
		new_connection->on_error_ = on_error_;
		new_connection->on_connected_ = on_connected_;
		new_connection->on_disconnected_ = on_disconnected_;
		new_connection->on_received_ = on_received_;

		acceptor_->async_accept(new_connection->socket(),
			boost::bind(&ServerSocket::handle_accept, this, new_connection,
				boost::asio::placeholders::error));
	}

	void handle_accept(Connection::pointer new_connection,
		const boost::system::error_code& error) {
		if (!error) {
			new_connection->start();
		}

		start_accept();
	}

	boost::asio::io_context io_context_;
	std::unique_ptr<tcp::acceptor> acceptor_;

	ErrorEvent on_error_ = nullptr;
	ServerSocketEvent on_connected_ = nullptr;
	ServerSocketEvent on_disconnected_ = nullptr;
	ReceivedEvent on_received_ = nullptr;
};

#endif  // RYULIB_SOCKETSERVER_HPP