#pragma once

#include <mutex>
#include <condition_variable>
#include <optional>
#include <chrono>

using namespace std::chrono_literals;

class TimeOut : public std::exception {
    const char *what() const noexcept override {
            return "Timeout";
    }
};

template<typename T>
class UnbufferedChannel {
public:
    void Put(const T &data) {
        std::unique_lock lock(Mutex_);
        while (Count_ == 1) {
            Cv_Full_.wait(lock);
        }
        Cv_Empty_.notify_one();
        Count_ = 1;
        Data_ = data;
    }

    T Get(std::chrono::milliseconds timeout = std::chrono::milliseconds(0)) {
        auto start = std::chrono::steady_clock::now();
        std::unique_lock lock(Mutex_);
        while (Count_ == 0) {
            if (timeout == std::chrono::milliseconds(0)) {
                Cv_Empty_.wait(lock);
            } else {
                if (Cv_Empty_.wait_until(lock, timeout + start) == std::cv_status::timeout) {
                    throw TimeOut();
                }
            }
        }
        Cv_Full_.notify_one();
        Count_ = 0;
        return Data_;
    }
private:
    std::condition_variable Cv_Full_;
    std::condition_variable Cv_Empty_;
    std::mutex Mutex_;
    int Count_ = 0;
    T Data_;
};
