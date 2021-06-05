#pragma once

#ifndef ALBOT_OBJECTIFIER_HPP_
#define ALBOT_OBJECTIFIER_HPP_

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
            inline Object* adopt(Object& other) {
                if(this->min_x == NULL) {
                    this->min_x = other.min_x;
                    this->min_y = other.min_y;
                    this->max_x = other.max_x;
                    this->max_y = other.max_y;
                } else {
                    if(this->min_x > other.min_x) {
                        this->min_x = other.min_x;
                    }
                    if(this->max_x < other.max_x) {
                        this->max_x = other.max_x;
                    }
                    if(this->min_y > other.min_y) {
                        this->min_y = other.min_y;
                    }
                    if(this->max_y < other.max_y) {
                        this->max_y = other.max_y;
                    }
                }
                return this;
            }
            inline Object* adopt(MapProcessing::Point& point) {
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
            inline Object* adopt(MapProcessing::Line& line) {
                if(this->min_x == NULL) {
                    this->min_x = std::min(line.first.x, line.second.x);
                    this->max_x = std::max(line.first.x, line.second.x);
                    this->min_y = std::min(line.first.x, line.second.y);
                    this->max_y = std::max(line.first.x, line.second.y);
                } else {
                    short min_x = std::min(line.first.x, line.second.x);
                    short max_x = std::max(line.first.x, line.second.x);
                    short min_y = std::min(line.first.y, line.second.y);
                    short max_y = std::max(line.first.y, line.second.y);
                    if(this->min_x > min_x) {
                        this->min_x = min_x;
                    }
                    if(this->max_x < max_x) {
                        this->max_x = max_x;
                    }
                    if(this->min_y > min_y) {
                        this->min_y = min_y;
                    }
                    if(this->max_y < max_y) {
                        this->max_y = max_y;
                    }
                }
                return this;
            }
        };
        std::unordered_map<MapProcessing::Line, std::vector<MapProcessing::Line>*, MapProcessing::LineHash> lines_to_object;
        std::unordered_map<std::vector<MapProcessing::Line>*, Object*> object_sizes;
        std::vector<MapProcessing::Point> points;
        std::vector<std::vector<MapProcessing::Line>*> objects;
        std::vector<MapProcessing::Line> lines;
        MapProcessing::MapInfo* info;
        Objectifier(MapProcessing::MapInfo* info);
        void chain(const std::pair<short, short>& first, const std::pair<short, short>& second);
        void run();
};

#endif /* ALBOT_OBJECTIFIER_HPP_ */