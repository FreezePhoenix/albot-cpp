#include <map>
#include <set>
#include <vector>
#include "Objectifier.hpp"

Objectifier::Objectifier(MapProcessing::MapInfo* info) {
    this->list_to_objects = std::map<TupleList*, Object*>();
    this->objects = std::vector<TupleList*>();
    this->tuple_to_node = std::map<MapProcessing::TupleHash, TupleList::Node*>();
    this->info = info;
}
void Objectifier::chain(MapProcessing::Tuple first, MapProcessing::Tuple second) {
    bool first_found = tuple_to_node.find(first.hash) != tuple_to_node.end();
    bool second_found = tuple_to_node.find(second.hash) != tuple_to_node.end();
    if(!first_found && !second_found) {
        DoubleLinkedList<MapProcessing::Tuple>* list = new TupleList();
        TupleList::Node* first_node = list->push(first);
        TupleList::Node* second_node = list->push(second);
        tuple_to_node.insert(std::pair<MapProcessing::TupleHash, TupleList::Node*>(first.hash, first_node));
        tuple_to_node.insert(std::pair<MapProcessing::TupleHash, TupleList::Node*>(second.hash, second_node));
        objects.push_back(list);
        short min_x = std::min(first.first, second.first);
        short max_x = std::max(first.first, second.first);
        short min_y = std::min(first.second, second.second);
        short max_y = std::max(first.second, second.second);
        Object* obj = new Object(min_x, max_x, min_y, max_y);
        list_to_objects.insert(std::pair<TupleList*, Object*>(list, obj));
    } else {
        if(first_found && second_found) {
            TupleList::Node* first_node = tuple_to_node[first.hash];
            TupleList::Node* second_node = tuple_to_node[second.hash];
            TupleList* old_list = second_node->list;
            if(first_node->list != second_node->list) {
                if(first_node->next == nullptr) {
                    // Hecc, they're running in opposite directions.
                    if(second_node->next == nullptr) {
                        TupleList::Node* temp = second_node->previous;
                        first_node->push(second_node);
                        list_to_objects[first_node->list]->adopt(second_node->value);
                        if(temp != nullptr) {
                            chain(second, temp->value);
                        }
                        if(old_list->length == 0) {
                            std::vector<TupleList*>::iterator find = std::find(objects.begin(), objects.end(), old_list);
                            if(find != objects.end()) {
                                objects.erase(find);
                                std::map<TupleList*, Object*>::iterator object_find = list_to_objects.find(old_list);
                                if(object_find != list_to_objects.end()) {
                                    list_to_objects.erase(object_find);
                                }
                            }
                        }
                    } else if(second_node->previous == nullptr) {
                        TupleList::Node* temp = second_node->next;
                        first_node->push(second_node);
                        list_to_objects[first_node->list]->adopt(second_node->value);
                        chain(second, temp->value);

                        if(old_list->length == 0) {
                            std::vector<TupleList*>::iterator find = std::find(objects.begin(), objects.end(), old_list);
                            if(find != objects.end()) {
                                objects.erase(find);
                                std::map<TupleList*, Object*>::iterator object_find = list_to_objects.find(old_list);
                                if(object_find != list_to_objects.end()) {
                                    list_to_objects.erase(object_find);
                                }
                            }
                        }
                    } else {
                        first_node->push(second_node->copy());
                    }
                } else if(first_node->previous == nullptr) {
                    // Hecc... opposite directions again?
                    if(second_node->previous == nullptr) {
                        TupleList::Node* temp = second_node->next;
                        first_node->unshift(second_node);
                        list_to_objects[first_node->list]->adopt(second_node->value);
                        if(temp != nullptr) {
                            chain(second, temp->value);
                        }
                        if(old_list->length == 0) {
                            std::vector<TupleList*>::iterator find = std::find(objects.begin(), objects.end(), old_list);
                            if(find != objects.end()) {
                                objects.erase(find);
                                std::map<TupleList*, Object*>::iterator object_find = list_to_objects.find(old_list);
                                if(object_find != list_to_objects.end()) {
                                    list_to_objects.erase(object_find);
                                }
                            }
                        }
                    } else if(second_node->next == nullptr) {
                        TupleList::Node* temp = second_node->previous;
                        first_node->unshift(second_node);
                        list_to_objects[first_node->list]->adopt(second_node->value);
                        chain(second, temp->value);
                        if(old_list->length == 0) {
                            std::vector<TupleList*>::iterator find = std::find(objects.begin(), objects.end(), old_list);
                            if(find != objects.end()) {
                                objects.erase(find);
                                std::map<TupleList*, Object*>::iterator object_find = list_to_objects.find(old_list);
                                if(object_find != list_to_objects.end()) {
                                    list_to_objects.erase(object_find);
                                }
                            }
                        }
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
                TupleList::Node* first_node = tuple_to_node[first.hash];
                TupleList::Node* second_node = nullptr;
                if(first_node->next == nullptr) {
                    second_node = first_node->push(new TupleList::Node(second));
                } else if(first_node->previous == nullptr) {
                    second_node = first_node->unshift(new TupleList::Node(second));
                }
                tuple_to_node.insert(std::pair<MapProcessing::TupleHash, TupleList::Node*>(second.hash, second_node));
            } else if(second_found) {
                TupleList::Node* first_node = nullptr;
                TupleList::Node* second_node = tuple_to_node[second.hash];
                if(second_node->next == nullptr) {
                    first_node = second_node->push(new TupleList::Node(first));
                } else if(second_node->previous == nullptr) {
                    first_node = second_node->unshift(new TupleList::Node(first));
                }
                tuple_to_node.insert(std::pair<MapProcessing::TupleHash, TupleList::Node*>(first.hash, first_node));
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
        MapProcessing::Tuple first_tuple = MapProcessing::Tuple(x, y1);
        MapProcessing::Tuple second_tuple = MapProcessing::Tuple(x, y2);
        chain(first_tuple, second_tuple);
    }
    for(int i = 0; i < this->info->y_lines.size(); i++) {
        std::vector<short> line = this->info->y_lines[i];
        int y = line[0];
        int x1 = line[1];
        int x2 = line[2];
        chain(MapProcessing::Tuple(x1, y), MapProcessing::Tuple(x2, y));
    }
    std::cout << this->objects.size() << " : " << this->info->name << std::endl;
    for (const auto &i: this->objects) {
        std::sort(this->objects.begin(), this->objects.end(), [this](const TupleList* lhs, const TupleList* rhs) {
            Object left_object = *list_to_objects[(TupleList*) lhs];
            Object right_object = *list_to_objects[(TupleList*) rhs];
            return (left_object.max_x - left_object.min_x) * (left_object.max_y - left_object.min_y) > (right_object.max_x - right_object.min_x) * (right_object.max_y - right_object.min_y);
        });
    }
};