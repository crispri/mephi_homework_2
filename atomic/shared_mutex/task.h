#pragma once
#include <atomic>

class SharedMutex {
public:
    void lock() {
        if(Exclusive_Count_.load() + Shared_Count_.load() == 0){
            Exclusive_Count_.fetch_add(1);
            Available_.store(1);
        }
    }

    void unlock() {
        Exclusive_Count_.fetch_sub(1);
        Available_.store(0);
    }

    void lock_shared() {
        if(Exclusive_Count_.load() == 0){
            Shared_Count_.fetch_add(1);
            Available_.store(1);
        }
    }

    void unlock_shared() {
        Shared_Count_.fetch_sub(1);
        if(Shared_Count_.load() == 0)
            Available_.store(0);
    }

private:
    std::atomic<int> Available_{0};
    std::atomic<int> Exclusive_Count_{0};
    std::atomic<int> Shared_Count_{0};
};
//fetch_add <=> +=1
//exchange -> возвращает старое значени, переменную заменяет на новое
// store -> заменяет старое значение на новое
// load -> выгрузка значения из atomic
//compare_exchange_storg -> сравнивает object и old_val, если равны то object = new_val
