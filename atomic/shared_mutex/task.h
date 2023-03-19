#pragma once
#include <mutex>
#include <condition_variable>
class SharedMutex {
public:
    void lock() {
        std::unique_lock<std::mutex> lock(Mutex_);
        while(SharedCount_+ExclusiveCount_ !=0)
            Available_.wait(lock);
        ExclusiveCount_+=1;
    }

    void unlock() {
        std::unique_lock<std::mutex> lock(Mutex_);
        ExclusiveCount_ -=1;
        Available_.notify_all();
    }

    void lock_shared() {
        std::unique_lock<std::mutex> lock(Mutex_);
        while (ExclusiveCount_!=0){
            Available_.wait(lock);
        }
        SharedCount_+=1;
    }

    void unlock_shared() {
        std::unique_lock<std::mutex> lock(Mutex_);
        SharedCount_-=1;
        if(SharedCount_ == 0)
            Available_.notify_all();

    }

private:
    std::mutex Mutex_;
    std::condition_variable Available_;
    int ExclusiveCount_ = 0;
    int SharedCount_ = 0;
};
