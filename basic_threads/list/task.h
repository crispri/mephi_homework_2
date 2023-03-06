#pragma once

#include <cstdint>
#include <mutex>
#include <shared_mutex>
#include <set>
#include <atomic>
#include <vector>
#include <iostream>
#include <list>
/*
 * Потокобезопасный связанный список.
 */
template<typename T>
struct Node{
    Node<T>* prev;
    Node<T>* next;
    T val;
    explicit Node(Node<T>* prev, Node<T>* next, const T& val):prev(prev), next(next), val(val){}
    Node() = default;
    mutable std::shared_mutex Mutex_;
};
template<typename T>
class ThreadSafeList {
private:
    Node<T>* Head;
    Node<T>* Tail;
    mutable std::mutex M_;
public:
    ThreadSafeList():Head(nullptr), Tail(nullptr){

    }
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
        Iterator(Node<T>* cur_):node_(cur_){}
        T& operator *() {
            std::shared_lock lock(node_->Mutex_);
            return node_->val;
        }

        T operator *() const {
            std::shared_lock lock(node_->Mutex_);
            return node_->val;
        }

        T* operator ->() {
            std::shared_lock lock(node_->Mutex_);
            return &(node_->val);
        }

        const T* operator ->() const {
            std::shared_lock lock(node_->Mutex_);
            return &(node_->val);
        }

        Iterator& operator ++() {
            std::unique_lock guard(node_->Mutex_);
            node_=node_->next;
            return *this;
        }

        Iterator operator ++(int) {
            std::unique_lock guard(node_->Mutex_);
            Iterator tmp = *this;
            ++(*this);
            return tmp;
        }

        Iterator& operator --() {
            std::unique_lock guard(node_->Mutex_);
            node_ =node_->prev;
            return *this;
        }

        Iterator operator --(int) {
            std::unique_lock guard(node_->Mutex_);
            Iterator tmp = *this;
            --(*this);
            return tmp;
        }

        bool operator ==(const Iterator& rhs) const {
            // std::shared_lock guard(node_->Mutex_);
            return node_ == rhs.node_;
        }

        bool operator !=(const Iterator& rhs) const {
            //std::shared_lock guard(node_->Mutex_);

            return node_!=rhs.node_;
        }
        Node<T>* GetCurrentNode() {
            return  node_;
        }


    private:
        Node<T>* node_;
    };

    /*
     * Получить итератор, указывающий на первый элемент списка
     */
    Iterator begin() {
        if(Head!= nullptr){
            std::lock_guard g(Head->Mutex_);
            return Iterator(Head);}
        else
            return Iterator(nullptr);
    }

    /*
     * Получить итератор, указывающий на "элемент после последнего" элемента в списке
     */
    Iterator end() {
        //std::lock_guard gg(M_);
        if(Tail == nullptr)
            return Iterator(nullptr);
        else{
            std::lock_guard g(Tail->Mutex_);
            return Iterator(Tail->next);
        }
    }

    /*
     * Вставить новый элемент в список перед элементом, на который указывает итератор `position`
     */
    void insert(Iterator position, const T& value) {
        std::lock_guard g(M_);
        Node<T>* cur;
        if(position != nullptr and position.GetCurrentNode()->prev!=Tail)
            cur = position.GetCurrentNode();
        else
            cur = nullptr;
        if(cur!= nullptr){
            std::lock_guard gg(cur-> Mutex_);
            if (cur == Head){ //вставка в начало
                std::lock_guard g_Head(Head->Mutex_);
                Node<T>* newNode = new Node<T>(nullptr, Head,value);
                std::lock_guard g(newNode->Mutex_);
                Head->prev = newNode;
                Head = newNode;
            }
            else{ //вставка в середину
                std::lock_guard g_prev (cur->prev->Mutex_);
                std::lock_guard g_next(cur->Mutex_);
                Node<T>* newNode = new Node<T>(cur->prev, cur, value);
                std::lock_guard g(newNode->Mutex_);
                cur->prev->next = newNode;
                cur->prev = newNode;
            }
        }
        else {
            if(Head == nullptr){ //если список пустой
                Node<T>* newNode = new Node<T>(nullptr, new Node<T>(),value);
                std::lock_guard g (newNode->Mutex_);
                Head = newNode;
                Tail = newNode;
                std::lock_guard gg(Tail->Mutex_);
                //Tail->next = new Node<T>();
                //Tail->next->prev = Tail;
            }//вставка в конец
            else {
                std::lock_guard g_Tail(Tail->Mutex_);
                Node<T> *newNode = new Node<T>(Tail, new Node<T>(), value);
                std::lock_guard g(newNode->Mutex_);
                Tail->next = newNode;
                Tail = newNode;
                std::lock_guard gg(Tail->Mutex_);
                //Tail->next = new Node<T>();
                //Tail->next->prev = Tail;

            }
        }

    }

    /*
     * Стереть из списка элемент, на который указывает итератор `position`
     */
    void erase(Iterator position) {
        std::lock_guard g(M_);
        auto cur = position.GetCurrentNode();
        if(cur == Head and Head!= nullptr){ //удаляем из начала
            Head = Head->next;
            Head->prev = nullptr;
        }
        else if(cur == Tail and Tail!= nullptr){//удаляем из конца
            Tail = Tail->prev;
            Tail->next = nullptr;
        }
        else if(Head!= nullptr and Tail!= nullptr){ //удаляем из середины
            cur->next->prev = cur->prev;
            cur->prev->next = cur->next;
        }
    }
};