#include "albot-cpp/Utils/DoubleLinkedList.hpp"
#include "albot-cpp/MapProcessing/MapProcessing.hpp"
template<typename T>
DoubleLinkedList<T>::DoubleLinkedList() {
    this->head = nullptr;
    this->tail = nullptr;
}

template<typename T>
void DoubleLinkedList<T>::empty() {
    DoubleLinkedList<T>::Node* node = this->pop();
    while(node != nullptr) {
        node = this->pop();
    }
}

template<typename T>
void DoubleLinkedList<T>::append(DoubleLinkedList<T>* other_list) {
    DoubleLinkedList<T>::Node* node = other_list->pop();
    while(node != nullptr) {
        this->push(node);
        node = other_list->pop();
    }
}

template<typename T>
void DoubleLinkedList<T>::push(DoubleLinkedList<T>::Node* new_node) {
    if(this->length == 0) {
        this->head = new_node;
        this->tail = new_node;
    } else {
        this->tail->next = new_node;
        new_node->previous = this->tail;
        this->tail = new_node;
    }
    new_node->list = this;
    this->length++;
}

template<typename T>
typename DoubleLinkedList<T>::Node* DoubleLinkedList<T>::push(T data) {
    DoubleLinkedList<T>::Node* new_node = new DoubleLinkedList<T>::Node(data);
    this->push(new_node);
    return new_node;
}

template<typename T>
typename DoubleLinkedList<T>::Node* DoubleLinkedList<T>::pop() {
    if(this->length == 0) {
        return nullptr;
    }
    DoubleLinkedList<T>::Node* node = this->tail;
    if(this->length == 1) {
        this->head = nullptr;
        this->tail = nullptr;
    } else {
        this->tail = this->tail->previous;
        this->tail->next = nullptr;
        node->previous = nullptr;
    }
    node->list = nullptr;
    this->length--;
    return node;
}

template<typename T>
typename DoubleLinkedList<T>::Node* DoubleLinkedList<T>::shift() {
    if(this->length == 0) {
        return nullptr;
    }
    DoubleLinkedList<T>::Node* node = this->head;
    if(this->length == 1) {
        this->head = nullptr;
        this->tail = nullptr;    
    } else {
        this->head = node->next;
        this->head->previous = nullptr;
        node->next = nullptr;
    }
    node->list = nullptr;
    this->length--;
    return node;
}

template<typename T>
typename DoubleLinkedList<T>::Node* DoubleLinkedList<T>::unshift(T data) {
    DoubleLinkedList<T>::Node* new_node = new DoubleLinkedList<T>::Node(data);
    if(this->length == 0) {
        this->head = new_node;
        this->tail = new_node;
    } else {
        new_node->next = this->head;
        this->head->previous = new_node;
        this->head = new_node;
    }
    new_node->list = this;
    this->length++;
    return new_node;
}

template<typename T>
DoubleLinkedList<T>::Node::Node() {
    this->value = T();
    this->next = nullptr;
    this->previous = nullptr;
    this->list = nullptr;
}

template<typename T>
DoubleLinkedList<T>::Node::Node(T value) {
    this->value = value;
    this->next = nullptr;
    this->previous = nullptr;
    this->list = nullptr;
}

template<typename T>
typename DoubleLinkedList<T>::Node* DoubleLinkedList<T>::Node::push(DoubleLinkedList<T>::Node* node) {
    node->remove();
    node->previous = this;
    node->next = this->next;
    if(this->next != nullptr) {
        this->next->previous = node;
    } else {
        this->list->tail = node;
    }
    this->next = node;
    this->list->length++;
    node->list = this->list;
    return node;
}

template<typename T>
typename DoubleLinkedList<T>::Node* DoubleLinkedList<T>::Node::copy() {
    return new DoubleLinkedList<T>::Node(this->value);
}

template<typename T>
void DoubleLinkedList<T>::Node::remove() {
    if(this->list == nullptr) {
        return;
    }
    if(this->previous != nullptr) {
        this->previous->next = this->next;
    } else {
        this->list->head = this->next;
    }
    if(this->next != nullptr) {
        this->next->previous = this->previous;
    } else {
        this->list->tail = this->previous;
    }
    this->list->length--;
    this->list = nullptr;
}

template<typename T>
typename DoubleLinkedList<T>::Node* DoubleLinkedList<T>::Node::unshift(DoubleLinkedList<T>::Node* node) {
    node->remove();
    node->next = this;
    node->previous = this->previous;
    if(this->previous != nullptr) {
        this->previous->next = node;
    } else {
        this->list->head = node;
    }
    this->previous = node;
    this->list->length++;
    node->list = this->list;
    return node;
}