#pragma once

#ifndef ALBOT_OBJECTIFIER_HPP_
#define ALBOT_OBJECTIFIER_HPP_

#include "../Common.hpp"
#include "MapProcessing.hpp"
#include <set>
#include <unordered_set>

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
            Object* adopt(MapProcessing::Point& point) {
                if(this->min_x == NULL) {
                    this->min_x = this->max_x = point.x;
                    this->min_y = this->max_y = point.y;
                } else {
                    if(point.x < this->min_x) {
                        this->min_x = point.x;
                    } else if(point.x > this->max_x) {
                        this->max_x = point.x;
                    }
                    if(point.y < this->min_y) {
                        this->min_y = point.y;
                    } else if(point.y > this->max_y) {
                        this->max_y = point.y;
                    }
                }
                return this;
            }
        };
        std::unordered_map<MapProcessing::Line, std::vector<MapProcessing::Line>*, MapProcessing::LineHash> lines_to_object;
        std::unordered_map<std::vector<MapProcessing::Line>*, Object*>* object_sizes;
        std::vector<MapProcessing::Point> points;
        std::vector<std::vector<MapProcessing::Line>*> objects;
        std::vector<MapProcessing::Line> lines;
        MapProcessing::MapInfo* info;
        Objectifier(MapProcessing::MapInfo* info);
        void chain(const std::pair<short, short>& first, const std::pair<short, short>& second);
        void run();
};

#endif /* ALBOT_OBJECTIFIER_HPP_ */