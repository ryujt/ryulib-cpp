// WebSocket.hpp
#pragma once

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <functional>
#include <thread>
#include <mutex>

class WebSocket {
public:
    typedef websocketpp::client<websocketpp::config::asio_client> client_t;
    typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

    WebSocket() : _isConnected(false) {
        try {
            _client.init_asio();
            _client.clear_access_channels(websocketpp::log::alevel::all);
            _client.set_access_channels(websocketpp::log::alevel::none);

            using websocketpp::lib::placeholders::_1;
            using websocketpp::lib::placeholders::_2;
            _client.set_open_handler(bind(&WebSocket::on_open, this, _1));
            _client.set_fail_handler(bind(&WebSocket::on_fail, this, _1));
            _client.set_close_handler(bind(&WebSocket::on_close, this, _1));
            _client.set_message_handler(bind(&WebSocket::on_message, this, _1, _2));
        }
        catch (const std::exception& e) {
            if (onErrorHandler) {
                onErrorHandler(-1, "WebSocket initialization error: " + std::string(e.what()));
            }
        }
    }

    ~WebSocket() {
        Close();
    }

    void Connect(const std::string& uri) {
        try {
            websocketpp::lib::error_code ec;
            client_t::connection_ptr con = _client.get_connection(uri, ec);

            if (ec) {
                if (onErrorHandler) {
                    onErrorHandler(-2, "Connection creation failed: " + ec.message());
                }
                return;
            }

            _hdl = con->get_handle();
            _client.connect(con);
            _thread = std::thread([this]() { _client.run(); });
        }
        catch (const std::exception& e) {
            if (onErrorHandler) {
                onErrorHandler(-2, "Connection error: " + std::string(e.what()));
            }
        }
    }

    void Send(const void* data, size_t size) {
        try {
            std::lock_guard<std::mutex> lock(_mutex);

            if (!_isConnected) {
                if (onErrorHandler) {
                    onErrorHandler(-3, "Cannot send: Not connected");
                }
                return;
            }

            websocketpp::lib::error_code ec;
            _client.send(_hdl, data, size, websocketpp::frame::opcode::binary, ec);

            if (ec) {
                if (onErrorHandler) {
                    onErrorHandler(-4, "Send error: " + ec.message());
                }
            }
        }
        catch (const std::exception& e) {
            if (onErrorHandler) {
                onErrorHandler(-4, "Send error: " + std::string(e.what()));
            }
        }
    }

    void Close() {
        try {
            std::lock_guard<std::mutex> lock(_mutex);

            if (!_isConnected) {
                return;
            }

            websocketpp::lib::error_code ec;
            _client.close(_hdl, websocketpp::close::status::normal, "Closing connection", ec);

            if (ec) {
                if (onErrorHandler) {
                    onErrorHandler(-5, "Close error: " + ec.message());
                }
            }

            _client.stop();

            if (_thread.joinable()) {
                _thread.join();
            }

            _isConnected = false;
        }
        catch (const std::exception& e) {
            if (onErrorHandler) {
                onErrorHandler(-5, "Close error: " + std::string(e.what()));
            }
        }
    }

    void SetOnConnected(std::function<void()> handler) {
        onConnectedHandler = handler;
    }

    void SetOnError(std::function<void(int, std::string)> handler) {
        onErrorHandler = handler;
    }

    void SetOnDisconnected(std::function<void()> handler) {
        onDisconnectedHandler = handler;
    }

    void SetOnReceived(std::function<void(const void*, size_t)> handler) {
        onReceivedHandler = handler;
    }

private:
    void on_open(websocketpp::connection_hdl hdl) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _isConnected = true;
        }
        if (onConnectedHandler) {
            onConnectedHandler();
        }
    }

    void on_fail(websocketpp::connection_hdl hdl) {
        if (onErrorHandler) {
            onErrorHandler(-6, "Connection failed");
        }
    }

    void on_close(websocketpp::connection_hdl hdl) {
        {
            std::lock_guard<std::mutex> lock(_mutex);
            _isConnected = false;
        }
        if (onDisconnectedHandler) {
            onDisconnectedHandler();
        }
    }

    void on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
        if (onReceivedHandler) {
            auto payload = msg->get_payload();
            onReceivedHandler(payload.data(), payload.size());
        }
    }

    client_t _client;
    websocketpp::connection_hdl _hdl;
    std::thread _thread;
    std::mutex _mutex;
    bool _isConnected;

    std::function<void()> onConnectedHandler;
    std::function<void(int, std::string)> onErrorHandler;
    std::function<void()> onDisconnectedHandler;
    std::function<void(const void*, size_t)> onReceivedHandler;
};