#pragma once

#ifndef ALBOT_DOUBLE_LINKED_LIST_HPP_
#define ALBOT_DOUBLE_LINKED_LIST_HPP_

#include <memory>
#include "albot-cpp/MapProcessing/MapProcessing.hpp"

#define NodePointer typename DoubleLinkedList<T>::Node*

template<typename T>
class DoubleLinkedList {
    public:
        class Node {
            public:
                T value;
                DoubleLinkedList<T>::Node* next = nullptr;
                DoubleLinkedList<T>::Node* previous = nullptr;
                DoubleLinkedList<T>* list = nullptr;
                // Constructors
                Node();
                Node(T value);
                // Methods
                typename DoubleLinkedList<T>::Node* unshift(typename DoubleLinkedList<T>::Node* node);
                typename DoubleLinkedList<T>::Node* push(DoubleLinkedList<T>::Node* node);
                typename DoubleLinkedList<T>::Node* copy();
                void remove();
        };
        Node* head;
        Node* tail;
        unsigned int length = 0;
        DoubleLinkedList<T>();
        void empty();
        void append(DoubleLinkedList<T>* other_list);
        Node* unshift(T data);
        typename DoubleLinkedList<T>::Node* push(T data);
        Node* shift();
        Node* pop();
    private:
        void push(Node* new_node);
};
#endif /* ALBOT_DOUBLE_LINKED_LIST_HPP_ */