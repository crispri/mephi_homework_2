#include <thread>
#include <vector>
#include "task.h"
using namespace std::chrono_literals;
PrimeNumbersSet::PrimeNumbersSet() = default;
void PrimeNumbersSet::AddPrimesInRange(uint64_t from, uint64_t to) {

    for(uint64_t it = from; it<to; ++it){
        bool flag = true;
        for(int i = 2;i*i<=it; ++i){
            if(it%i ==0){
                flag = false;
                break;}
        }
        if (flag and it!= 0 and it!=1){
            auto start_func = std::chrono::high_resolution_clock::now();
            set_mutex_.lock();
            auto start_lock = std::chrono::high_resolution_clock::now();
            nanoseconds_waiting_mutex_  += (start_lock - start_func).count();
            primes_.insert(it);
            set_mutex_.unlock();
            auto end_lock = std::chrono::high_resolution_clock::now();
            nanoseconds_under_mutex_ += (end_lock - start_lock).count();
        }
    }

}

uint64_t PrimeNumbersSet::GetMaxPrimeNumber() const {
    std::lock_guard g(set_mutex_);
    return *(--primes_.end());
}

uint64_t PrimeNumbersSet::GetNextPrime(uint64_t number) const {
    std::lock_guard g(set_mutex_);
    if (++primes_.find(number)!= primes_.end())
        return *(++primes_.find(number));
    else
        throw std::invalid_argument("Don't know next prime after limit\\n");
}

size_t PrimeNumbersSet::GetPrimesCountInRange(uint64_t from, uint64_t to) const {
    std::lock_guard g(set_mutex_);
    if(primes_.lower_bound(from)!=primes_.end() and primes_.lower_bound(to) != primes_.end())
        return std::distance(primes_.lower_bound(from),primes_.lower_bound(to)) - 1;
    else
        return std::distance(primes_.lower_bound(from), primes_.end());
}

std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeUnderMutex() const {
    return std::chrono::nanoseconds (nanoseconds_under_mutex_);
}

std::chrono::nanoseconds PrimeNumbersSet::GetTotalTimeWaitingForMutex() const {
    return std::chrono::nanoseconds (nanoseconds_waiting_mutex_);
}

bool PrimeNumbersSet::IsPrime(uint64_t number) const {
    return primes_.find(number)!=primes_.end();
}
