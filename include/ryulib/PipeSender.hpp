#pragma once

#include <Windows.h>
#include <string>
#include <ryulib/base.hpp>
#include <ryulib/strg.hpp>
#include <ryulib/Worker.hpp>

using namespace std;
using namespace ryulib;

class PipeSender {
public:
    PipeSender()
    {
    }

    ~PipeSender()
    {
        close();
    }

    bool open(string name)
    {
        if (worker_ != nullptr) return false;

        LPCWSTR ws_name = StringToWideChar(name);

        HANDLE pipe = CreateNamedPipe(ws_name,
            PIPE_ACCESS_OUTBOUND,
            PIPE_TYPE_BYTE,
            1, 0, 0, 0, NULL);
        if (pipe == INVALID_HANDLE_VALUE) {
            return false;
        }

        if (ConnectNamedPipe(pipe, NULL) == FALSE) {
            CloseHandle(pipe);
            pipe_ = INVALID_HANDLE_VALUE;
            return false;
        }

        worker_ = new Worker();
        worker_->setOnTask([&](int task, const string text, const void* data, int size, int tag) {
            do_send((Memory*) data);
        });

        return true;
    }

    void close()
    {
        if (worker_ == nullptr) return;

        worker_->terminateAndWait();
        delete worker_;
        worker_ = nullptr;

        if (pipe_ != INVALID_HANDLE_VALUE) {
            FlushFileBuffers(pipe_);
            CloseHandle(pipe_);
        }

        pipe_ = INVALID_HANDLE_VALUE;
    }

    void send(const void* data, int size)
    {
        if (worker_ != nullptr) worker_->add(0, new Memory(data, size), 0, 0);
    }

private:
    HANDLE pipe_ = INVALID_HANDLE_VALUE;
    Worker* worker_ = nullptr;

    void do_send(Memory* data)
    {
        if (pipe_ == INVALID_HANDLE_VALUE) return;

        DWORD dwWritten;
        WriteFile(pipe_, data->getData(), data->getSize(), &dwWritten, NULL);
        delete data;
    }
};
