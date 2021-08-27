#include <map>
#include <set>
#include <vector>
#include <iostream>
#include "Objectifier.hpp"
#include "ShapeManipulator.hpp"


Objectifier::Objectifier(std::shared_ptr<MapProcessing::MapInfo> info) {
    this->info = info;
    this->points = std::vector<MapProcessing::Point>();
    this->lines = std::vector<MapProcessing::Line>();
    this->lines_to_object = std::unordered_map<MapProcessing::Line, std::shared_ptr<std::vector<MapProcessing::Line>>, MapProcessing::LineHash>();
    this->objects = std::vector<std::shared_ptr<std::vector<MapProcessing::Line>>>();
    this->object_sizes = std::unordered_map<std::shared_ptr<std::vector<MapProcessing::Line>>, Object>();
};


MapProcessing::Line normalize(MapProcessing::Line& inp) {
    return MapProcessing::Line(std::min(inp.first.x, inp.second.x), std::max(inp.first.x, inp.second.x), std::min(inp.first.y, inp.second.y), std::max(inp.first.y, inp.second.y));
}

// inline bool intersects(const MapProcessing::Line& line_one, const MapProcessing::Line& line_two) {
//     if(line_one.first.x == line_one.second.x) {
//         // line_one is vertical.
//         if(line_two.first.x == line_two.second.x) {
//             // line_two is also vertical? This isn't looking that good...
//             return line_two.first.x == line_one.first.x && line_one.first.y <= line_two.second.y && line_one.second.y >= line_two.first.y;
//         } else {
//             // line_two is horizontal.
//             return line_two.first.x <= line_one.first.x && line_two.second.x >= line_one.first.x && line_one.first.y <= line_two.first.y && line_one.second.y >= line_two.first.y;
//         }
//     } else {
//         // line_one is horizontal.
//         if(line_two.first.x == line_two.second.x) {
//             // line_two is vertical.
//             return line_two.first.y <= line_one.first.y && line_two.second.y >= line_one.first.y && line_one.first.x <= line_two.first.x && line_one.second.x >= line_two.first.x;
//         } else {
//             if(line_one.first.y == line_two.first.y) {
//                 // line_two is horizontal as well... treat as normal range overlap
//                 return line_one.first.x <= line_two.second.x && line_one.second.x >= line_two.first.x;
//             }
//         }
//     }
//     return false;
// }

void Objectifier::run() {
    this->lines.reserve(this->info->x_lines.size() + this->info->y_lines.size());
    for(const MapProcessing::AxisLineSegment& line : this->info->x_lines) {
        this->lines.emplace_back(line.axis, line.range_start, line.axis, line.range_end);
    }
    for(const MapProcessing::AxisLineSegment& line : this->info->y_lines) {
        this->lines.emplace_back(line.range_start, line.axis, line.range_end, line.axis);
    }
    ShapeManipulator::handle_intersections(lines, [this](const MapProcessing::Line& line_one, const MapProcessing::Line& line_two) {
        const std::unordered_map<MapProcessing::Line, std::shared_ptr<std::vector<MapProcessing::Line>>, MapProcessing::LineHash>::iterator find1 = this->lines_to_object.find(line_one);
        const std::unordered_map<MapProcessing::Line, std::shared_ptr<std::vector<MapProcessing::Line>>, MapProcessing::LineHash>::iterator find2 = this->lines_to_object.find(line_two);
        const std::unordered_map<MapProcessing::Line, std::shared_ptr<std::vector<MapProcessing::Line>>, MapProcessing::LineHash>::iterator end = this->lines_to_object.end();
                if(find1 != this->lines_to_object.end()) {
                    if(find2 != this->lines_to_object.end()) {
                        // They're both in the array? Huh.
                        std::shared_ptr<std::vector<MapProcessing::Line>> obj1 = find1->second;
                        std::shared_ptr<std::vector<MapProcessing::Line>> obj2 = find2->second;;
                        if(obj1 == obj2) {
                            return;
                        }
                        this->object_sizes.at(obj1).adopt(this->object_sizes.at(obj2));
                        for(std::vector<MapProcessing::Line>::iterator it3 = obj2->begin(); it3 != obj2->end(); it3++) {
                            this->lines_to_object.insert_or_assign(*it3, obj1);
                            obj1->push_back(*it3);
                        }
                        std::vector<std::shared_ptr<std::vector<MapProcessing::Line>>>::iterator find3 = std::find(this->objects.begin(), this->objects.end(), obj2);
                        if(find3 != this->objects.end()) {
                            this->objects.erase(find3);
                        } else {
                            std::cout << "UHH" << std::endl;
                        }
                        this->object_sizes.erase(obj2);
                    } else {
                        // Only it1 is in the list.
                        std::shared_ptr<std::vector<MapProcessing::Line>> obj = find1->second;
                        obj->push_back(line_two);
                        this->object_sizes.at(obj).adopt(line_two);
                        this->lines_to_object.emplace(line_two, obj);
                    }
                } else {
                    if(find2 != this->lines_to_object.end()) {
                        // Only it2 is in the list.
                        std::shared_ptr<std::vector<MapProcessing::Line>> obj = find2->second;
                        obj->push_back(line_one);
                        this->object_sizes.at(obj).adopt(line_one);
                        this->lines_to_object.emplace(line_one, obj);
                    } else {
                        // None of them are in the list... wow.
                        std::shared_ptr<std::vector<MapProcessing::Line>> obj = std::shared_ptr<std::vector<MapProcessing::Line>>(new std::vector<MapProcessing::Line>());
                        this->lines_to_object.emplace(line_one, obj);
                        this->lines_to_object.emplace(line_two, obj);
                        obj->push_back(line_one);
                        obj->push_back(line_two);
                        this->objects.push_back(obj);
                        Object size = Object();
                        size.adopt(line_one).adopt(line_two);
                        this->object_sizes.emplace(obj, size);
                    }
                }
    });
    // for (int i = 0, size = this->lines.size(); i < size; i++) {
    //     MapProcessing::Line& line_one = this->lines[i];
    //     for(int j = i; j < size; j++) {
    //         MapProcessing::Line& line_two = this->lines[j];
    //         if(line_one == line_two) {
    //             continue;
    //         }
    //         if(intersects(line_one, line_two)) {
               
    //         }
    //     }
    // }
    // for (const auto &i: this->objects) {
    // std::cout << this->objects.size() << std::endl;
        std::sort(this->objects.begin(), this->objects.end(), [this](const std::shared_ptr<std::vector<MapProcessing::Line>> first, std::shared_ptr<std::vector<MapProcessing::Line>> second) {
            Object& first_obj = this->object_sizes.at((std::shared_ptr<std::vector<MapProcessing::Line>>) first);
            Object& second_obj = this->object_sizes.at((std::shared_ptr<std::vector<MapProcessing::Line>>) second);
            return std::abs(((first_obj.max_x - first_obj.min_x) * (first_obj.max_y - first_obj.min_y))) > std::abs(((second_obj.max_x - second_obj.min_x) * (second_obj.max_y - second_obj.min_y)));
        });
    // }
};
