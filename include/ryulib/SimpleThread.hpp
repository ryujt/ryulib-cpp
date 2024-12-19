#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>

using namespace std;
using namespace ryulib;

class SimpleThread;

typedef function<void(SimpleThread*)> SimpleThreadEvent;

class SimpleThread {
public:
    SimpleThread(const SimpleThreadEvent& event)
        : is_terminated_(false), on_execute_(event)
    {
        thread_ = std::thread([this, event]() {  // this와 event를 명시적으로 캡처
            event(this);
            if (on_terminated_) on_terminated_(this);
            });
    }

    ~SimpleThread() {
        thread_.detach();
    }

    void sleep(int millis) {
        if (is_terminated_) return;

        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait_for(lock, std::chrono::milliseconds(millis), [this]() { return is_terminated_.load(); });
    }

    void sleepTight() {
        if (is_terminated_) return;

        std::unique_lock<std::mutex> lock(mutex_);
        condition_.wait(lock, [this]() { return is_terminated_.load(); });
    }

    void wakeUp() {
        condition_.notify_all();
    }

    void terminate() {
        is_terminated_.store(true);
        wakeUp();
    }

    void terminateAndWait() {
        terminate();
        if (thread_.joinable()) {
            thread_.join();
        }
    }

    void terminateNow()
    {
        is_terminated_.store(true);  // atomic store 사용

#ifdef __linux__ 
        pthread_cancel(thread_.native_handle());
        thread_.detach();  // 리소스 누수 방지
#elif _WIN32
        TerminateThread(thread_.native_handle(), 0);
        thread_.detach();  // 리소스 누수 방지
#endif    

        std::lock_guard<std::mutex> lock(mutex_);  // callback 호출 전 락 확보
        if (on_terminated_ != nullptr) on_terminated_(this);
    }

    bool isTerminated() const {
        return is_terminated_.load();
    }

    void setOnTerminated(NotifyEvent event) {
        std::lock_guard<std::mutex> lock(mutex_);
        on_terminated_ = event;
    }

private:
    std::thread thread_;
    std::mutex mutex_;
    std::condition_variable condition_;
    std::atomic<bool> is_terminated_;
    SimpleThreadEvent on_execute_;
    NotifyEvent on_terminated_;
};
