### Распараллелить составление множества простых чисел

В данном задании требуется реализовать класс `PrimeNumbersSet` -- множество простых чисел в каком-то диапазоне.

Методы, которые необходимо реализовать, описаны в файле `task.h`, реализацию нужно поместить в `task.cpp`.
Работу с объектом `std::set<uint64_t> primes_` нужно проводить под локом мьютекса `set_mutex_` (см. объявление класса).
