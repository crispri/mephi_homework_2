#pragma once
#include <atomic>
class SharedMutex {
public:
    void lock() {
        int status = 0;
        while(!Shared_Count_.compare_exchange_strong(status,-1) == 0){
            status = 0;
        }
    }

    void unlock() {
        Shared_Count_.store(0);
    }

    void lock_shared() {
        int status = Shared_Count_.load();
        while(!Shared_Count_.compare_exchange_strong(status,status+1)){
            while(status == -1){
                status = Shared_Count_.load();
            }
        }
    }
    void unlock_shared() {
        Shared_Count_.fetch_sub(1);
    }

private:
    std::atomic<int> Shared_Count_{0};
};
//fetch_add <=> +=1
//exchange -> возвращает старое значени, переменную заменяет на новое
// store -> заменяет старое значение на новое
// load -> выгрузка значения из atomic
//compare_exchange_storg -> сравнивает object и old_val, если равны то object = new_val
