#include "Writer.hpp"
#include <fstream>
#include<sstream>

Writer::Writer(Objectifier* objectifier) {
    this->objectifier = objectifier;
    this->points = std::map<MapProcessing::TupleHash, int>();
}

std::string pad_right(std::string const& str, size_t s) {
    if ( str.size() < s ) {
        return str + std::string(s-str.size(), ' ');
    } else {
        return str;
    };
}

void Writer::write() {
    MapProcessing::MapInfo* info = this->objectifier->info;
    std::ofstream outfile("Maps/" + info->name);
    std::stringstream first_header;
    std::stringstream second_header;
    std::stringstream content_header;
    int segments = 0;
    int object_number = 1;
    int point_count = 0;
    outfile.clear();
    for(const DoubleLinkedList<MapProcessing::Tuple>* list : this->objectifier->objects) {
        segments += list->length - 1;
        content_header << " # Object " << (object_number++) << std::endl;
        TupleList::Node* current = list->head;
        bool first = true;
        do {
            if(!first) {
                current = current->next;
            }
            if(this->points.find(current->value.hash) != this->points.end()) {
                content_header << "  #";
            } else {
                content_header << "   ";
                point_count++;
                this->points.insert(std::pair<MapProcessing::TupleHash, int>(current->value.hash, point_count));
            }
            content_header << " " << pad_right(std::to_string(point_count), 7) << " " << pad_right(std::to_string(current->value.first), 5) << " " << pad_right(std::to_string(current->value.second), 7) << " " << object_number << std::endl;
            first = false;
        } while(current->next != nullptr);
    }
    content_header << segments << " 1" << std::endl;
    object_number = 1;
    int segment_index = 1;
    for(const DoubleLinkedList<MapProcessing::Tuple>* list : this->objectifier->objects) {
        segments += list->length - 1;
        content_header << " # Object " << (object_number++) << std::endl;
        TupleList::Node* current = list->head;
        bool first = true;
        do {
            if(!first) {
                current = current->next;
            }
            content_header << "    " << pad_right(std::to_string(segment_index++), 7) << " " << pad_right(std::to_string(points[current->value.hash]), 5) << " " << pad_right(std::to_string(points[current->next->value.hash]), 7) << " " << object_number << std::endl;
            first = false;
        } while(current->next != nullptr && current->next->next != nullptr);
    }
    first_header << "# Generated from map: " << info->name << std::endl;
    first_header << points.size() << " 2 0 1" << std::endl;
    outfile << first_header.str();
    outfile << content_header.str();
    outfile << "0";
    outfile.close();
}