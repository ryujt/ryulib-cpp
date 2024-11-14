#pragma once
#include <string>
#include <memory>
#include <atomic>
#include <iostream>
#include <ryulib/WebSocket.hpp>
#include <ryulib/Worker.hpp>
#include <ryulib/Timer.hpp>

class ZombieSocket {
public:
    enum Task {
        TASK_CONNECT = 1,
        TASK_RECONNECT = 2,
        TASK_DISCONNECT = 3,
        TASK_SEND = 4
    };

    ZombieSocket() : _isRunning(false), _isConnected(false), _healthCount(0) {
        _healthTimer.setOnTimer([this](const void* sender) {
            _healthCount++;

            if (_healthCount >= 6) {
                _healthCount = 0;
                _isConnected = false;
                if (_onErrorHandler) {
                    _onErrorHandler(-104, "No messages received for 30 seconds");
                }
                _worker.add(TASK_RECONNECT);
            }
            });
        _healthTimer.start(5000);

        _worker.setOnTask([this](int task, string text, void* data, int size, int tag) {
            switch (task) {
            case TASK_CONNECT: doConnect(); break;
            case TASK_RECONNECT: doReconnect(); break;
            case TASK_DISCONNECT: doDisconnect(); break;
            case TASK_SEND:
                if (data != nullptr) {
                    doSend(data, size);
                }
                break;
            }
            });

        _reconnectWorker.setOnTask([this](int task, string text, void* data, int size, int tag) {
            _healthCount = 0;
            _reconnectWorker.sleep(3000);

            // 여기서 상태를 다시 한번 체크
            if (_isRunning) {
                _worker.add(TASK_CONNECT);
            }
            });
    }

    ~ZombieSocket() {
        disconnect();
        _healthTimer.terminate();
    }

    void connect(const std::string& uri) {
        _uri = uri;
        _isRunning = true;
        _isConnected = false;
        _healthCount = 0;
        _worker.add(TASK_CONNECT);
    }

    void disconnect() {
        _isRunning = false;
        _worker.add(TASK_DISCONNECT);
    }

    void send(const void* data, size_t size) {
        if (!_isConnected) return;

        try {
            auto buffer = std::make_unique<uint8_t[]>(size);
            std::memcpy(buffer.get(), data, size);
            _worker.add(TASK_SEND, buffer.release(), static_cast<int>(size));
        }
        catch (const std::exception& e) {
            if (_onErrorHandler) {
                _onErrorHandler(-103, std::string("Memory allocation error: ") + e.what());
            }
        }
    }

    void setOnConnected(std::function<void()> handler) {
        _onConnectedHandler = handler;
    }

    void setOnError(std::function<void(int, std::string)> handler) {
        _onErrorHandler = handler;
    }

    void setOnDisconnected(std::function<void()> handler) {
        _onDisconnectedHandler = handler;
    }

    void setOnReceived(std::function<void(const void*, size_t)> handler) {
        _onReceivedHandler = handler;
    }

private:
    void createWebSocket() {
        _ws = std::make_unique<WebSocket>();

        _ws->setOnConnected([this]() {
            _isConnected = true;
            _healthCount = 0;
            if (_onConnectedHandler) {
                _onConnectedHandler();
            }
            });

        _ws->setOnError([this](int code, std::string message) {
            if (_onErrorHandler) {
                _onErrorHandler(code, message);
            }

            // 연결 실패 시 즉시 재접속 시도
            if (_isRunning) {
                _isConnected = false;
                _worker.add(TASK_RECONNECT);
            }
            });

        _ws->setOnDisconnected([this]() {
            _isConnected = false;
            if (_onDisconnectedHandler) {
                _onDisconnectedHandler();
            }

            // 연결이 끊어진 경우에도 재접속 시도
            if (_isRunning) {
                _worker.add(TASK_RECONNECT);
            }
            });

        _ws->setOnReceived([this](const void* data, size_t size) {
            _healthCount = 0;
            if (_onReceivedHandler) {
                _onReceivedHandler(data, size);
            }
            });
    }

    void doConnect() {
        if (_isConnected) return;

        try {
            _ws.reset();
            createWebSocket();
            _ws->connect(_uri);
        }
        catch (const std::exception& e) {
            _isConnected = false;
            if (_onErrorHandler) {
                _onErrorHandler(-100, std::string("Connection error: ") + e.what());
            }
            _worker.add(TASK_RECONNECT);
        }
    }

    void doReconnect() {
        _reconnectWorker.add(0);
    }

    void doDisconnect() {
        _isConnected = false;
        try {
            if (_ws) {
                _ws->disconnect();
                _ws.reset();
            }
        }
        catch (...) {
			// do nothing
        }
    }

    void doSend(const void* data, int size) {
        std::unique_ptr<uint8_t[]> buffer(static_cast<uint8_t*>(const_cast<void*>(data)));

        if (!_isConnected || !_ws) {
            return;
        }

        try {
            _ws->send(buffer.get(), size);
        }
        catch (const std::exception& e) {
            if (_onErrorHandler) {
                _onErrorHandler(-102, std::string("Send error: ") + e.what());
            }
        }
    }

private:
    std::unique_ptr<WebSocket> _ws;
    Worker _worker;
    Worker _reconnectWorker;
    Timer _healthTimer;
    std::string _uri;
    bool _isRunning;
    bool _isConnected;
    std::atomic<int> _healthCount;
    std::function<void()> _onConnectedHandler;
    std::function<void(int, std::string)> _onErrorHandler;
    std::function<void()> _onDisconnectedHandler;
    std::function<void(const void*, size_t)> _onReceivedHandler;
};