#ifndef RYU_TCP_CLIENT_HPP
#define RYU_TCP_CLIENT_HPP


#include <ryulib/Worker.hpp>
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>


using namespace std;

#define PACKET_LIMIT 65536

enum TaskType {ttConnect, ttDisconnect, ttSendData, ttSendText, ttReceive};

typedef function<void(int, char*)> ErrorEvent;
typedef function<void()> SocketEvent;
typedef function<void(void*, int)> ReceivedEvent;

class TcpClient
{
public:
	TcpClient() {
		worker_.setOnRepeat(
			[&]() {
				if (socket_ == 0) {
					worker_.Sleep(5);
				} else {
					do_receive();
				}
			}
		);
		worker_.setOnTask(
			[&](int task, void* data, int size, int tag) {
				switch (task) {
                    case ttConnect: {
                        string* str = (string*) data;
                        do_connect((char*) str->c_str(), size);
                        delete str;
                    } break;
                        
					case ttDisconnect: do_disconnect(task, data, size, tag); break;
                        
                    case ttSendData: {
                         do_sendData(data, size);
                        delete data;
                    } break;

                    case ttSendText: {
                        string* str = (string*) data;
                        do_sendData((char*) str->c_str(), str->size());
                        delete str;
                    } break;
				}
			}
		);
		worker_.start();
	}

	void connect(const char* host, int port) {
        string* str = new string(host);
		worker_.add(ttConnect, str, port, 0);
	}

	void disconnect() {
		worker_.add(ttDisconnect, nullptr, 0, 0);
	}

    void sendData(const void* data, int size) {
        if (size > PACKET_LIMIT) {
            if (on_error_ != nullptr) on_error_(-3, "Packet size is limited to exec(_codes_) bytes.");
            return;
        }
        
        void *buffer = new char[PACKET_LIMIT];
        memcpy(buffer, data, size);
        worker_.add(ttSendData, buffer, size, 0);
    }
    
    void sendText(const char* text) {
        string* str = new string(text);
        worker_.add(ttSendText, str, 0, 0);
    }
    
	bool isConnected() { return socket_ != 0;  }

	void setOnConnected(SocketEvent event) { on_connected_ = event; }
	void setOnDisconnected(SocketEvent event) { on_disconnected_ = event; }
	void setOnReceived(ReceivedEvent event) { on_received_ = event; }
	void setOnError(ErrorEvent event) { on_error_ = event; }

private:
	int socket_ = 0;
	fd_set fd_read_;
	Worker worker_;

	SocketEvent on_connected_ = nullptr;
	SocketEvent on_disconnected_ = nullptr;
	ReceivedEvent on_received_ = nullptr;
	ErrorEvent on_error_ = nullptr;

	void do_connect(char* host, int port) {
		struct sockaddr_in address;
		memset(&address, 0, sizeof(address));
		address.sin_family = AF_INET;
		address.sin_port = htons(port);

		// TODO
		if ((host[0] >= '0') && (host[0] <= '9')) {
			address.sin_addr.s_addr = inet_addr(host);
		} else {
			struct hostent* hostent = gethostbyname(host);
			if (hostent == NULL) {
				if (on_error_ != nullptr) on_error_(-1, "gethostbyname, can't find domain.");
				return;
			}
			address.sin_addr = *((struct in_addr*) hostent->h_addr_list[0]);
		}
        
		socket_ = socket(PF_INET, SOCK_STREAM, 0);
		FD_SET(socket_, &fd_read_);
		long arg = fcntl(socket_, F_GETFL, NULL);
		arg = arg | O_NONBLOCK;
		fcntl(socket_, F_SETFL, arg);
        
		bool connected = ::connect(socket_, (struct sockaddr*) & address, sizeof(address)) >= 0;

		if ((!connected) && (errno == EINPROGRESS)) {
			struct timeval timeout;
			timeout.tv_sec = 5000;
			timeout.tv_usec = 0;
			connected = select(socket_ + 1, NULL, &fd_read_, NULL, &timeout) > 0;
		}
        
		if (!connected) {
			do_disconnect(0, nullptr, 0, 0);
			if (on_error_ != nullptr) on_error_(-2, "can't connect to server.");
		} else {
			arg = arg & (~(O_NONBLOCK));
			fcntl(socket_, F_SETFL, arg);
			if (on_connected_ != nullptr) on_connected_();
		}
	}

	void do_disconnect(int task, void* data, int size, int tag) {
		if (socket_ == 0) return;

		close(socket_);
		socket_ = 0;
		FD_ZERO(&fd_read_);
	}

	void do_sendData(void* text, int size) {
		if (socket_ == 0) return;

		if (send(socket_, text, size, 0) <= 0) {
            do_disconnect(0, nullptr, 0, 0);
            if (on_disconnected_ != nullptr) on_disconnected_();
        }
	}

	void do_receive() {
		if (socket_ == 0) return;

		// 문자열의 터미널 문자까지 4097크기여도 되지만 개인 취향임 (4로 나눠떨어지는 수)
		char buffer[4096 + 4];
		int received_bytes = recv(socket_, buffer, 4096, MSG_DONTWAIT);
		if (received_bytes <= 0) {
			do_disconnect(0, nullptr, 0, 0);
			if (on_disconnected_ != nullptr) on_disconnected_();
		}

		while (received_bytes > 0) {
			if (on_received_ != nullptr) on_received_(buffer, received_bytes);
			received_bytes = recv(socket_, buffer, 4096, MSG_DONTWAIT);
		}
	}
};


#endif  // RYU_TCP_CLIENT_HPP
