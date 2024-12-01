#pragma once

#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>

class WebSocket {
public:
    typedef websocketpp::client<websocketpp::config::asio_client> client_t;
    typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

    WebSocket() : _isConnected(false), _isClosing(false) {
        try {
            initClient();
        }
        catch (const std::exception& e) {
            if (onErrorHandler) {
                onErrorHandler(-1, "Constructor initialization error: " + std::string(e.what()));
            }
        }
    }

    ~WebSocket() {
        try {
            disconnect();
        }
        catch (...) {
            // Suppress any exceptions in destructor
        }
    }

    void connect(const std::string& uri) {
        if (_isConnected || _isClosing) {
            disconnect();  // Ensure clean state before new connection
        }

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

            // Ensure thread cleanup
            if (_thread.joinable()) {
                _thread.join();
            }

            _thread = std::thread([this]() {
                try {
                    _client.run();
                }
                catch (const std::exception& e) {
                    if (onErrorHandler) {
                        onErrorHandler(-7, "Run error: " + std::string(e.what()));
                    }
                }
                });
        }
        catch (const std::exception& e) {
            if (onErrorHandler) {
                onErrorHandler(-2, "Connection error: " + std::string(e.what()));
            }
            throw;
        }
    }

    void disconnect() {
        std::lock_guard<std::mutex> lock(_mutex);

        if (_isClosing) return;  // 이미 종료 중이면 리턴
        _isClosing = true;

        try {
            if (_isConnected) {
                websocketpp::lib::error_code ec;
                _client.close(_hdl, websocketpp::close::status::normal, "Closing connection", ec);
            }

            _client.stop();  // 항상 stop 호출

            if (_thread.joinable()) {
                _thread.join();
            }
        }
        catch (const std::exception& e) {
            if (onErrorHandler) {
                onErrorHandler(-5, "Close error: " + std::string(e.what()));
            }
        }

        _isConnected = false;
        _isClosing = false;
    }

    void send(const void* data, size_t size) {
        try {
            std::lock_guard<std::mutex> lock(_mutex);

            if (!_isConnected || _isClosing) {
                if (onErrorHandler) {
                    onErrorHandler(-3, "Cannot send: Not connected or closing");
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

    void setOnConnected(std::function<void()> handler) {
        onConnectedHandler = handler;
    }

    void setOnError(std::function<void(int, std::string)> handler) {
        onErrorHandler = handler;
    }

    void setOnDisconnected(std::function<void()> handler) {
        onDisconnectedHandler = handler;
    }

    void setOnReceived(std::function<void(const void*, size_t)> handler) {
        onReceivedHandler = handler;
    }

private:
    void initClient() {
        _client.clear_access_channels(websocketpp::log::alevel::all);
        _client.set_access_channels(websocketpp::log::alevel::none);

        _client.init_asio();

        using websocketpp::lib::placeholders::_1;
        using websocketpp::lib::placeholders::_2;

        _client.set_open_handler(std::bind(&WebSocket::on_open, this, _1));
        _client.set_fail_handler(std::bind(&WebSocket::on_fail, this, _1));
        _client.set_close_handler(std::bind(&WebSocket::on_close, this, _1));
        _client.set_message_handler(std::bind(&WebSocket::on_message, this, _1, _2));
    }

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
    std::atomic<bool> _isConnected;
    std::atomic<bool> _isClosing;

    std::function<void()> onConnectedHandler;
    std::function<void(int, std::string)> onErrorHandler;
    std::function<void()> onDisconnectedHandler;
    std::function<void(const void*, size_t)> onReceivedHandler;
};