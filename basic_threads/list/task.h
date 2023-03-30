#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <set>
#include <atomic>
#include <vector>
#include <iostream>
/*
 * Потокобезопасный связанный список.
 */
template<typename T>
struct TNode{
    TNode() = default;
    TNode(TNode* p, TNode* n, const T& v):prev(p), next(n), val(v){}
    TNode* prev;
    TNode* next;
    T val;
    mutable std::mutex Mutex_;
};
template<typename T>
class ThreadSafeList {
private:
    TNode<T>* Head;
    TNode<T>* Tail; //элемент после последнего в списке
public:
    ThreadSafeList():Head(nullptr),Tail(nullptr){}
    /*
     * Класс-итератор, позволяющий обращаться к элементам списка без необходимости использовать мьютекс.
     * При этом должен гарантироваться эксклюзивный доступ потока, в котором был создан итератор, к данным, на которые
     * он указывает.
     * Итератор, созданный в одном потоке, нельзя использовать в другом.
     */
    class Iterator {
    public:
        using pointer = T*;
        using value_type = T;
        using reference = T&;
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::bidirectional_iterator_tag;
        Iterator(TNode<T>* node):node(node){}
        T& operator *() {
            std::unique_lock<std::mutex> lock(node->Mutex_);
            return node->val;
        }

        T operator *() const {
            std::unique_lock<std::mutex> lock(node->Mutex_);
            return node->val;
        }

        T* operator ->() {
            std::unique_lock<std::mutex> lock(node->Mutex_);
            return &(node->val);
        }

        const T* operator ->() const {
            std::unique_lock<std::mutex> lock(node->Mutex_);
            return &(node->val);
        }

        Iterator& operator ++() {
            std::unique_lock<std::mutex> lock(node->Mutex_);
            node = node->next;
            return *this;
        }

        Iterator operator ++(int) {
            std::unique_lock<std::mutex> lock(node->Mutex_);
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Iterator& operator --() {
            std::unique_lock<std::mutex> lock(node->Mutex_);
            node= node->prev;
            return *this;
        }

        Iterator operator --(int) {
            std::unique_lock<std::mutex> lock(node->Mutex_);
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator ==(const Iterator& rhs) const {
            return node == rhs.node;
        }

        bool operator !=(const Iterator& rhs) const {
            return node!=rhs.node;
        }
        TNode<T>* Get() {
            std::unique_lock<std::mutex> lock(node->Mutex_);
            return node;
        }

    private:
        TNode<T>* node;
    };

    /*
     * Получить итератор, указывающий на первый элемент списка
     */
    Iterator begin() {
        if(Head!= nullptr){
            std::unique_lock<std::mutex> lock(Head->Mutex_);
            return Iterator(Head);}
        else
            return Iterator(nullptr);
    }

    /*
     * Получить итератор, указывающий на "элемент после последнего" элемента в списке
     */
    Iterator end() {
        if(Tail!= nullptr){
            std::unique_lock<std::mutex> lock(Tail->Mutex_);
            return Iterator(Tail);}
        else
            return Iterator(nullptr);
    }

    /*
     * Вставить новый элемент в список перед элементом, на который указывает итератор `position`
     */
    void insert(Iterator position, const T& value) {
        if(Head == nullptr){ //список пустой
            Tail = new TNode<T>();
            std:: lock_guard<std::mutex> TailLock(Tail->Mutex_);
            Head = new TNode<T>(nullptr,Tail, value);
            std:: lock_guard<std::mutex> HeadLock(Head->Mutex_);
            Tail->prev = Head;
        }
        else{
            //вставка в начало
            if(position == Head){
                std:: lock_guard<std::mutex> HeadLock(Head->Mutex_);
                TNode<T>* cur = new TNode<T>(nullptr, Head,value);
                std:: lock_guard<std::mutex> CurrentLock(cur->Mutex_);
                Head->prev = cur;
                Head = cur;
            }
                //вставка в середину или конец
            else{

                TNode<T>* curNode = position.Get();
                std:: lock_guard<std::mutex>CurrentLock(curNode->Mutex_);
                TNode<T>* newNode = new TNode<T>(curNode->prev, curNode, value);
                std:: lock_guard<std::mutex>NewNodeLock(newNode->Mutex_);
                curNode->prev->next = newNode;
                curNode->prev = newNode;
            }
        }
    }

    /*
     * Стереть из списка элемент, на который указывает итератор `position`
     */
    void erase(Iterator position) {
        TNode<T>* curNode = position.Get();
        std::lock_guard<std::mutex> lock(curNode->Mutex_);
        //удаление из начала
        if(curNode == Head){
            Head = Head->next;
            Head->prev = nullptr;
        }
            //удаление из середины или конца
        else{
            curNode->prev->next = curNode->next;
            curNode->next->prev = curNode->prev;
        }
    }
};
