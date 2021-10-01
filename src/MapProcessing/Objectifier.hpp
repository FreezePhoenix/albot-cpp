#pragma once

#ifndef ALBOT_OBJECTIFIER_HPP_
#define ALBOT_OBJECTIFIER_HPP_

#include "MapProcessing.hpp"
#include <set>
#include <unordered_set>

class Objectifier {
    public:
        /**
         * @brief A structure to contain the bounding box of an object.
         * 
         */
        struct Object {
            short min_x;
            short max_x;
            short min_y;
            short max_y;
            Object(short minx, short maxx, short miny, short maxy) {
                this->min_x = minx;
                this->max_x = maxx;
                this->min_y = miny;
                this->max_y = maxy;
            };
            Object() {
                this->min_x = this->max_x = this->min_y = this->max_y = 0;
            };
            /**
             * @brief Grow the bounding box to include the other one.
             * 
             * @param other 
             * @return Object& 
             */
            inline Object& adopt(Object& other) {
                if(this->min_x == 0) {
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
                return *this;
            }
            /**
             * @brief Grow the bounding box to include a point.
             * 
             * @param point 
             * @return Object& 
             */
            inline Object& adopt(MapProcessing::Point& point) {
                if(this->min_x == 0) {
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
                return *this;
            }
            /**
             * @brief Grow the bounding box to include a line.
             * 
             * @param line 
             * @return Object& 
             */
            inline Object& adopt(const MapProcessing::Line& line) {
                if(this->min_x == 0) {
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
                return *this;
            }
        };
        std::unordered_map<MapProcessing::Line, std::shared_ptr<std::vector<MapProcessing::Line>>, MapProcessing::LineHash> lines_to_object;
        std::unordered_map<std::shared_ptr<std::vector<MapProcessing::Line>>, Object> object_sizes;
        std::vector<MapProcessing::Point> points;
        std::vector<std::shared_ptr<std::vector<MapProcessing::Line>>> objects;
        std::vector<MapProcessing::Line> lines;
        std::shared_ptr<MapProcessing::MapInfo> info;
        Objectifier(std::shared_ptr<MapProcessing::MapInfo> info);

        /**
         * @brief Run the objectifier on the given data.
         * 
         */
        void run();
};

#endif /* ALBOT_OBJECTIFIER_HPP_ */