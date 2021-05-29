#include <map>
#include <set>
#include <vector>
#include <iostream>
#include "Objectifier.hpp"

Objectifier::Objectifier(MapProcessing::MapInfo* info) {
    this->list_to_objects = std::unordered_map<DoubleLinkedList<std::pair<short, short>>*, Object*>();
    this->objects = std::vector<DoubleLinkedList<std::pair<short, short>>*>();

    this->tuple_to_node = std::unordered_map<std::pair<short, short>, DoubleLinkedList<std::pair<short, short>>::Node*, pair_hash>();
    this->info = info;
}
void Objectifier::chain(const std::pair<short, short>& first, const std::pair<short, short>& second) {
    bool first_found = tuple_to_node.find(first) != tuple_to_node.end();
    bool second_found = tuple_to_node.find(second) != tuple_to_node.end();
    if(!first_found && !second_found) {
        DoubleLinkedList<std::pair<short, short>>* list = new DoubleLinkedList<std::pair<short, short>>();
        DoubleLinkedList<std::pair<short, short>>::Node* first_node = list->push(first);
        DoubleLinkedList<std::pair<short, short>>::Node* second_node = list->push(second);
        tuple_to_node.insert(std::pair<std::pair<short, short>, DoubleLinkedList<std::pair<short, short>>::Node*>(first, first_node));
        tuple_to_node.insert(std::pair<std::pair<short, short>, DoubleLinkedList<std::pair<short, short>>::Node*>(second, second_node));
        objects.push_back(list);
        short min_x = std::min(first.first, second.first);
        short max_x = std::max(first.first, second.first);
        short min_y = std::min(first.second, second.second);
        short max_y = std::max(first.second, second.second);
        Object* obj = new Object(min_x, max_x, min_y, max_y);
        list_to_objects.insert(std::pair<DoubleLinkedList<std::pair<short, short>>*, Object*>(list, obj));
    } else {
        if(first_found && second_found) {
            DoubleLinkedList<std::pair<short, short>>::Node* first_node = tuple_to_node[first];
            DoubleLinkedList<std::pair<short, short>>::Node* second_node = tuple_to_node[second];
            DoubleLinkedList<std::pair<short, short>>* old_list = second_node->list;
            if(first_node->list != second_node->list) {
                if(first_node->next == nullptr) {
                    // Hecc, they're running in opposite directions.
                    if(second_node->next == nullptr) {
                        DoubleLinkedList<std::pair<short, short>>::Node* temp = second_node->previous;
                        first_node->push(second_node);
                        list_to_objects[first_node->list]->adopt(second_node->value);
                        if(temp != nullptr) {
                            chain(second, temp->value);
                        } else if(old_list->length == 0) {
                            std::vector<DoubleLinkedList<std::pair<short, short>>*>::iterator find = std::find(objects.begin(), objects.end(), old_list);
                            objects.erase(find);
                            list_to_objects.erase(old_list);
                        }
                    } else if(second_node->previous == nullptr) {
                        DoubleLinkedList<std::pair<short, short>>::Node* temp = second_node->next;
                        first_node->push(second_node);
                        list_to_objects[first_node->list]->adopt(second_node->value);
                        chain(second, temp->value);
                    } else {
                        first_node->push(second_node->copy());
                    }
                } else if(first_node->previous == nullptr) {
                    // Hecc... opposite directions again?
                    if(second_node->previous == nullptr) {
                        DoubleLinkedList<std::pair<short, short>>::Node* temp = second_node->next;
                        first_node->unshift(second_node);
                        list_to_objects[first_node->list]->adopt(second_node->value);
                        if(temp != nullptr) {
                            chain(second, temp->value);
                        } else if(old_list->length == 0) {
                            std::vector<DoubleLinkedList<std::pair<short, short>>*>::iterator find = std::find(objects.begin(), objects.end(), old_list);
                            objects.erase(find);
                            list_to_objects.erase(old_list);
                        }
                    } else if(second_node->next == nullptr) {
                        DoubleLinkedList<std::pair<short, short>>::Node* temp = second_node->previous;
                        first_node->unshift(second_node);
                        list_to_objects[first_node->list]->adopt(second_node->value);
                        chain(second, temp->value);
                    } else {
                        first_node->unshift(second_node->copy());
                    }
                }
            } else {
                if(first_node->next == nullptr) {
                    first_node->push(second_node->copy());
                } else if(first_node->previous == nullptr) {
                    first_node->unshift(second_node->copy());
                }
            }
        } else {
            if(first_found) {
                DoubleLinkedList<std::pair<short, short>>::Node* first_node = tuple_to_node[first];
                DoubleLinkedList<std::pair<short, short>>::Node* second_node = nullptr;
                if(first_node->next == nullptr) {
                    second_node = first_node->push(new DoubleLinkedList<std::pair<short, short>>::Node(second));
                } else if(first_node->previous == nullptr) {
                    second_node = first_node->unshift(new DoubleLinkedList<std::pair<short, short>>::Node(second));
                }
                tuple_to_node.insert(std::pair<std::pair<short, short>, DoubleLinkedList<std::pair<short, short>>::Node*>(second, second_node));
            } else if(second_found) {
                DoubleLinkedList<std::pair<short, short>>::Node* first_node = nullptr;
                DoubleLinkedList<std::pair<short, short>>::Node* second_node = tuple_to_node[second];
                if(second_node->next == nullptr) {
                    first_node = second_node->push(new DoubleLinkedList<std::pair<short, short>>::Node(first));
                } else if(second_node->previous == nullptr) {
                    first_node = second_node->unshift(new DoubleLinkedList<std::pair<short, short>>::Node(first));
                }
                tuple_to_node.insert(std::pair<std::pair<short, short>, DoubleLinkedList<std::pair<short, short>>::Node*>(first, first_node));
            }
        }
    }
};

void Objectifier::run() {
    for(int i = 0; i < this->info->x_lines.size(); i++) {
        std::vector<short> line = this->info->x_lines[i];
        int x = line[0];
        int y1 = line[1];
        int y2 = line[2];
        chain(std::pair<short, short>(x, y1), std::pair<short, short>(x, y2));
    }
    for(int i = 0; i < this->info->y_lines.size(); i++) {
        std::vector<short> line = this->info->y_lines[i];
        int y = line[0];
        int x1 = line[1];
        int x2 = line[2];
        chain(std::pair<short, short>(x1, y), std::pair<short, short>(x2, y));
    }
    std::cout << this->objects.size() << " : " << this->info->name << std::endl;
    for (const auto &i: this->objects) {
        std::sort(this->objects.begin(), this->objects.end(), [this](const DoubleLinkedList<std::pair<short, short>>* lhs, const DoubleLinkedList<std::pair<short, short>>* rhs) {
            Object left_object = *list_to_objects[(DoubleLinkedList<std::pair<short, short>>*) lhs];
            Object right_object = *list_to_objects[(DoubleLinkedList<std::pair<short, short>>*) rhs];
            return (left_object.max_x - left_object.min_x) * (left_object.max_y - left_object.min_y) > (right_object.max_x - right_object.min_x) * (right_object.max_y - right_object.min_y);
        });
    }
};