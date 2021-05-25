

#include "DoubleLinkedList.hpp"
int main() {
    DoubleLinkedList<int>* list = new DoubleLinkedList<int>();
    list->push(1);
    list->push(2);
    std::cout << list->head->value << std::endl;
    std::cout << list->head->next->value;
}