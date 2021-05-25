#include <map>
#include <set>
#include <vector>
#include "Objectifier.hpp"

Objectifier::Objectifier(MapProcessing::MapInfo* info) {
    this->objects = std::set<TupleList*>();
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
        objects.insert(list);
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
                        if(temp != nullptr) {
                            chain(second, temp->value);
                        }
                        if(old_list->length == 0) {
                            objects.erase(old_list);
                        }
                    } else if(second_node->previous == nullptr) {
                        TupleList::Node* temp = second_node->next;
                        first_node->push(second_node);
                        chain(second, temp->value);

                        if(old_list->length == 0) {
                            objects.erase(old_list);
                        }
                    } else {
                        first_node->push(second_node->copy());
                    }
                } else if(first_node->previous == nullptr) {
                    // Hecc... opposite directions again?
                    if(second_node->previous == nullptr) {
                        TupleList::Node* temp = second_node->next;
                        first_node->unshift(second_node);
                        if(temp != nullptr) {
                            chain(second, temp->value);
                        }
                        if(old_list->length == 0) {
                            objects.erase(old_list);
                        }
                    } else if(second_node->next == nullptr) {
                        TupleList::Node* temp = second_node->previous;\
                        first_node->unshift(second_node);
                        chain(second, temp->value);
                        if(old_list->length == 0) {
                            objects.erase(old_list);
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
    std::cout << this->objects.size() << " : " << this->info->name << std::endl ;
};