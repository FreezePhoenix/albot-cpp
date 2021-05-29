#pragma once

#ifndef ALBOT_OBJECTIFIER_HPP_
#define ALBOT_OBJECTIFIER_HPP_

#include "../Common.hpp"
#include "MapProcessing.hpp"
#include <set>

class Objectifier {
    public:
        struct Object {
            short min_x = NULL;
            short max_x = NULL;
            short min_y = NULL;
            short max_y = NULL;
            Object(short minx, short maxx, short miny, short maxy) {
                this->min_x = minx;
                this->max_x = maxx;
                this->min_y = miny;
                this->max_y = maxy;
            };
            Object() {
                this->min_x = this->max_x = this->min_y = this->max_y = NULL;
            };
            void adopt(std::pair<short, short> point) {
                if(this->min_x == NULL) {
                    this->min_x = this->max_x = point.first;
                    this->min_y = this->max_y = point.second;
                } else {
                    if(point.first < this->min_x) {
                        this->min_x = point.first;
                    } else if(point.first > this->max_x) {
                        this->max_x = point.first;
                    }
                    if(point.second < this->min_y) {
                        this->min_y = point.second;
                    } else if(point.second > this->max_y) {
                        this->max_y = point.second;
                    }
                }
            }
        };
        std::vector<DoubleLinkedList<std::pair<short, short>>*> objects;
        std::unordered_map<DoubleLinkedList<std::pair<short, short>>*, Object*> list_to_objects;
        std::unordered_map<std::pair<short, short>, DoubleLinkedList<std::pair<short, short>>::Node*, pair_hash> tuple_to_node;
        MapProcessing::MapInfo* info;
        Objectifier(MapProcessing::MapInfo* info);
        void chain(const std::pair<short, short>& first, const std::pair<short, short>& second);
        void run();
};

#endif /* ALBOT_OBJECTIFIER_HPP_ */