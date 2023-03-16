#include <thread>
#include <mutex>
#include <vector>
#include <queue>
#include <functional>
#include <condition_variable>
#include <atomic>
#include <stdexcept>
#include <cassert>
#include <iostream>
#include <chrono>

/*
 * Требуется написать класс ThreadPool, реализующий пул потоков, которые выполняют задачи из общей очереди.
 * С помощью метода PushTask можно положить новую задачу в очередь
 * С помощью метода Terminate можно завершить работу пула потоков.
 * Если в метод Terminate передать флаг wait = true,
 *  то пул подождет, пока потоки разберут все оставшиеся задачи в очереди, и только после этого завершит работу потоков.
 * Если передать wait = false, то все невыполненные на момент вызова Terminate задачи, которые остались в очереди,
 *  никогда не будут выполнены.
 * После вызова Terminate в поток нельзя добавить новые задачи.
 * Метод IsActive позволяет узнать, работает ли пул потоков. Т.е. можно ли подать ему на выполнение новые задачи.
 * Метод GetQueueSize позволяет узнать, сколько задач на данный момент ожидают своей очереди на выполнение.
 * При создании нового объекта ThreadPool в аргументах конструктора указывается количество потоков в пуле. Эти потоки
 *  сразу создаются конструктором.
 * Задачей может являться любой callable-объект, обернутый в std::function<void()>.
 */

class ThreadPool {
public:
    ThreadPool(size_t threadCount): IsActive_(true), Terminate_without_Wait(false) {
        for(size_t i = 0; i<threadCount;++i){
            Pool_.emplace_back([this](){
                while (true){
                    std::unique_lock<std::mutex> lock(Mutex_);
                    if(Terminate_without_Wait){
                        while(!Tasks_Queue_.empty())
                            Tasks_Queue_.pop();
                        Mutex_.unlock();
                        break;
                    }
                    else {
                        if(!Tasks_Queue_.empty()) {
                            std::function<void()> task = std::move( Tasks_Queue_.front());
                            Tasks_Queue_.pop();
                            Mutex_.unlock();
                            task();

                        }
                        else{
                            if(!IsActive())
                                break;
                        }
                    }
                }

            });
        }
    }

    void PushTask(const std::function<void()>& task) {
        std::unique_lock<std::mutex> lock(Mutex_);
        if(IsActive()){
            Tasks_Queue_.push(task);
        }
        else
            throw std::exception();
    }

    void Terminate(bool wait) {
        std::unique_lock<std::mutex> lock(Mutex_);
        IsActive_ = false;
        if(!wait){
            Terminate_without_Wait = true;
        }
        Mutex_.unlock();
        for(auto& it:Pool_){
            it.join();
        }
    }

    bool IsActive() const {
        return IsActive_.load();
    }

    size_t QueueSize() const {
        std::unique_lock<std::mutex> lock(Mutex_);
        return Tasks_Queue_.size();
    }
private:
    mutable std::mutex Mutex_;
    std::vector<std::thread> Pool_;
    std::queue<std::function<void()>> Tasks_Queue_;
    std::atomic<bool>IsActive_;
    std::atomic<bool>Terminate_without_Wait;
};