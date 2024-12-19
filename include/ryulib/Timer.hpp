#pragma once
#include <chrono>
#include <ryulib/base.hpp>
#include <ryulib/Scheduler.hpp>

using namespace std::chrono;

namespace ryulib {
    class Timer {
    public:
        Timer() {
            scheduler_.setOnRepeat([&]() {
                auto new_tick = std::chrono::system_clock::now();
                if (new_tick > old_tick_) {
                    // microseconds를 milliseconds로 변환
                    auto delta = std::chrono::duration_cast<std::chrono::milliseconds>(new_tick - old_tick_);
                    count_ms_ = count_ms_ + delta.count();
                }
                old_tick_ = new_tick;

                if (count_ms_ >= interval_) {
                    if (on_timer_ != nullptr) on_timer_(this);
                    count_ms_ = count_ms_ - interval_;
                }

                // sleep 시간을 더 길게 설정하여 CPU 사용률 감소
                scheduler_.sleep(100);  // 10ms 간격으로 체크
                });
        }

        void terminate() {
            stop();
            scheduler_.terminate();
        }

        void terminateNow() {
            stop();
            scheduler_.terminateNow();
        }

        void terminateAndWait() {
            stop();
            scheduler_.terminateAndWait();
        }

        void start(int interval) {
            interval_ = interval;
            count_ms_ = 0;
            old_tick_ = std::chrono::system_clock::now();
            scheduler_.start();
        }

        void stop() {
            scheduler_.stop();
        }

        void setOnTimer(NotifyEvent event) {
            on_timer_ = event;
        }

    private:
        Scheduler scheduler_;
        int64_t interval_ = 0;        // int64_t로 변경하여 큰 간격도 처리
        int64_t count_ms_ = 0;        // int64_t로 변경
        system_clock::time_point old_tick_;
        NotifyEvent on_timer_ = nullptr;
    };
}