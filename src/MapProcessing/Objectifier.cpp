#include <map>
#include <set>
#include <vector>
#include <iostream>
#include "Objectifier.hpp"

Objectifier::Objectifier(MapProcessing::MapInfo* info) {
    this->info = info;
    this->points = std::vector<MapProcessing::Point>();
    this->lines = std::vector<MapProcessing::Line>();
    this->lines_to_object = std::unordered_map<MapProcessing::Line, std::vector<MapProcessing::Line>*, MapProcessing::LineHash>();
    this->objects = std::vector<std::vector<MapProcessing::Line>*>();
    this->object_sizes = std::unordered_map<std::vector<MapProcessing::Line>*, Object*>();
};
std::string format(MapProcessing::Line lin) {
    return "{first:{x:" + std::to_string(lin.first.x) + ",y:" + std::to_string(lin.first.y) + "},second:{x:" + std::to_string(lin.second.x) + ",y:" + std::to_string(lin.second.y) + "}}";
}
MapProcessing::Line normalize(MapProcessing::Line& inp) {
    return MapProcessing::Line(std::min(inp.first.x, inp.second.x), std::max(inp.first.x, inp.second.x), std::min(inp.first.y, inp.second.y), std::max(inp.first.y, inp.second.y));
}

bool intersects(MapProcessing::Line& line_one, MapProcessing::Line& line_two) {
    // line_one = normalize(line_one);
    // line_two = normalize(line_two);
    if(line_one.first.x == line_one.second.x) {
        // line_one is vertical.
        if(line_two.first.x == line_two.second.x) {
            // line_two is also vertical? This isn't looking that good...
            if(line_two.first.x == line_one.first.x) {
                // All on same X. Treat as range overlaping problem.
                if(line_one.first.y <= line_two.second.y) {
                    // line_one's bottom point is below line_two's top point
                    if(line_one.second.y >= line_two.first.y) {
                        // line_one's top point is above line_two's bottom point
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            } else {
                // Line segments are parallel but not on same X. Cannot cross.
                return false;
            }
        } else {
            // line_two is horizontal.
            if(line_two.first.x <= line_one.first.x && line_two.second.x >= line_one.first.x) {
                // line_two contains line_one's x...
                if(line_one.first.y <= line_two.first.y && line_one.second.y >= line_two.first.y) {
                    // And line_one contains line_two's y!
                    return true;
                } else {
                    return false;
                }
            } else {
                // No chance of anything working.
                return false;
            }
        }
    } else {
        // line_one is horizontal.
        if(line_two.first.x == line_two.second.x) {
            // line_two is vertical.
            if(line_two.first.y <= line_one.first.y && line_two.second.y >= line_one.first.y) {
                // line_two contains line_one's Y
                if(line_one.first.x <= line_two.first.x && line_one.second.x >= line_two.first.x) {
                    // And line_one contains line_two's X
                    return true;
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else {
            if(line_one.first.y == line_two.first.y) {
                // line_two is horizontal as well... treat as normal range overlap
                if(line_one.first.x <= line_two.second.x) {
                    // line_one's left point is below line_two's top point
                    if(line_one.second.x >= line_two.first.x) {
                        // line_one's 
                        return true;
                    } else {
                        return false;
                    }
                } else {
                    return false;
                }
            } else {
                return false;
            }
        }
    }
    return false;
}


int summnation(short input) {
    return (input * (input - 1)) / 2;
}

unsigned int unique(short a, short b) {
    return a * b + (a + b) / 2 + (a - b) * (a - b) / 2 - summnation(std::min(a, b));
}

void Objectifier::run() {
    std::unordered_set<unsigned int> checked_pairs = std::unordered_set<unsigned int>();
    for(const std::vector<short>& line : this->info->x_lines) {
        int x = line[0];
        int y1 = line[1];
        int y2 = line[2];
        MapProcessing::Point p1 = MapProcessing::Point(x, y1);
        MapProcessing::Point p2 = MapProcessing::Point(x, y2);
        this->points.push_back(p1);
        this->points.push_back(p2);
        this->lines.push_back(MapProcessing::Line(p1, p2));
    }
    for(const std::vector<short>& line : this->info->y_lines) {
        int y = line[0];
        int x1 = line[1];
        int x2 = line[2];
        MapProcessing::Point p1 = MapProcessing::Point(x1, y);
        MapProcessing::Point p2 = MapProcessing::Point(x2, y);
        this->points.push_back(p1);
        this->points.push_back(p2);
        this->lines.push_back(MapProcessing::Line(p1, p2));
    }
    for(int i = 0, size = this->lines.size(); i < size; i++) {
        MapProcessing::Line& line_one = this->lines[i];
        for(int j = i; j < size; j++) {
            MapProcessing::Line& line_two = this->lines[j];
            if(line_one == line_two) {
                continue;
            }
            if(intersects(line_one, line_two)) {
                std::unordered_map<MapProcessing::Line, std::vector<MapProcessing::Line>*, MapProcessing::LineHash>::iterator find1 = this->lines_to_object.find(line_one);
                std::unordered_map<MapProcessing::Line, std::vector<MapProcessing::Line>*, MapProcessing::LineHash>::iterator find2 = this->lines_to_object.find(line_two);
                if(find1 != this->lines_to_object.end()) {
                    if(find2 != this->lines_to_object.end()) {
                        // They're both in the array? Huh.
                        std::vector<MapProcessing::Line>* obj1 = find1->second;
                        std::vector<MapProcessing::Line>* obj2 = find2->second;;
                        if(obj1 == obj2) {
                            continue;
                        }
                        this->object_sizes.at(obj1)->adopt(*this->object_sizes[obj2]);
                        for(std::vector<MapProcessing::Line>::iterator it3 = obj2->begin(); it3 != obj2->end(); it3++) {
                            this->lines_to_object.insert_or_assign(*it3, obj1);
                            obj1->push_back(*it3);
                        }
                        std::vector<std::vector<MapProcessing::Line>*>::iterator find3 = std::find(this->objects.begin(), this->objects.end(), obj2);
                        if(find3 != this->objects.end()) {
                            this->objects.erase(find3);
                        }
                        this->object_sizes.erase(obj2);
                    } else {
                        // Only it1 is in the list.
                        std::vector<MapProcessing::Line>* obj = find1->second;
                        obj->push_back(line_two);
                        this->object_sizes.at(obj)->adopt(line_two);
                        this->lines_to_object.emplace(line_two, obj);
                    }
                } else {
                    if(find2 != this->lines_to_object.end()) {
                        // Only it2 is in the list.
                        std::vector<MapProcessing::Line>* obj = find2->second;
                        obj->push_back(line_one);
                        this->object_sizes.at(obj)->adopt(line_one);
                        this->lines_to_object.emplace(line_one, obj);
                    } else {
                        // None of them are in the list... wow.
                        std::vector<MapProcessing::Line>* obj = new std::vector<MapProcessing::Line>();
                        this->lines_to_object.emplace(line_one, obj);
                        this->lines_to_object.emplace(line_two, obj);
                        obj->push_back(line_one);
                        obj->push_back(line_two);
                        this->objects.push_back(obj);
                        Object* size = new Object();
                        size->adopt(line_one)->adopt(line_two);
                        this->object_sizes.emplace(obj, size);
                    }
                }
            }
        }
    }
    for (const auto &i: this->objects) {
        std::sort(this->objects.begin(), this->objects.end(), [this](const std::vector<MapProcessing::Line>* first, std::vector<MapProcessing::Line>* second) {
            Object* first_obj = this->object_sizes[(std::vector<MapProcessing::Line>*) first];
            Object* second_obj = this->object_sizes[(std::vector<MapProcessing::Line>*) second];
            return std::abs(((first_obj->max_x - first_obj->min_x) * (first_obj->max_y - first_obj->min_y))) > std::abs(((second_obj->max_x - second_obj->min_x) * (second_obj->max_y - second_obj->min_y)));
        });
    }
};
