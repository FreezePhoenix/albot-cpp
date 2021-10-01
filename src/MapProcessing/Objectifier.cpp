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
    // Immediately reserve enough memory for the lines.
    this->lines.reserve(this->info->x_lines.size() + this->info->y_lines.size());

    // Insert all of our x_lines into the list of lines.
    for (const MapProcessing::AxisLineSegment& line : this->info->x_lines) {
        this->lines.emplace_back(line.axis, line.range_start, line.axis, line.range_end);
    }

    // Insert all of our y_lines into the list of lines.
    for (const MapProcessing::AxisLineSegment& line : this->info->y_lines) {
        this->lines.emplace_back(line.range_start, line.axis, line.range_end, line.axis);
    }

    // Pass controll over to ShapeManipulator.
    // ShapeManipualtor will call this lambda every time it detects a line intersection.
    ShapeManipulator::handle_intersections(lines, [this](const MapProcessing::Line& line_one, const MapProcessing::Line& line_two) {
        const std::unordered_map<MapProcessing::Line, std::shared_ptr<std::vector<MapProcessing::Line>>, MapProcessing::LineHash>::iterator find1 = this->lines_to_object.find(line_one);
        const std::unordered_map<MapProcessing::Line, std::shared_ptr<std::vector<MapProcessing::Line>>, MapProcessing::LineHash>::iterator find2 = this->lines_to_object.find(line_two);
        const std::unordered_map<MapProcessing::Line, std::shared_ptr<std::vector<MapProcessing::Line>>, MapProcessing::LineHash>::iterator end = this->lines_to_object.end();
        if(find1 != end) {
            if (find2 != end) {
                // They are both associated with objects.
                std::shared_ptr<std::vector<MapProcessing::Line>> obj1 = find1->second;
                std::shared_ptr<std::vector<MapProcessing::Line>> obj2 = find2->second;

                // If they're the same line, abort early. This really shouldn't happen.
                if (obj1 == obj2) {
                    return;
                }

                // Make the first line's object grow it's bounding box to include the second line's object.
                this->object_sizes.at(obj1).adopt(this->object_sizes.at(obj2));

                // Insert all of the lines in the second object into the first object.
                for (std::vector<MapProcessing::Line>::iterator it3 = obj2->begin(); it3 != obj2->end(); it3++) {
                    // Also remap the lines in it to the first object.
                    this->lines_to_object.insert_or_assign(*it3, obj1);
                    obj1->push_back(*it3);
                }

                // Remove the second object from the list of objects.
                std::vector<std::shared_ptr<std::vector<MapProcessing::Line>>>::iterator find3 = std::find(this->objects.begin(), this->objects.end(), obj2);
                if(find3 != this->objects.end()) {
                    this->objects.erase(find3);
                }

                // Also remove it's size data.
                this->object_sizes.erase(obj2);
            } else {
                // Only line_one is associated with an object.
                std::shared_ptr<std::vector<MapProcessing::Line>> obj = find1->second;

                // Insert the new line into the first line's object.
                obj->push_back(line_two);

                // Grow the object's bounding box to include the new line.
                this->object_sizes.at(obj).adopt(line_two);

                // Map the new line to the object.
                this->lines_to_object.emplace(line_two, obj);
            }
        } else {
            if(find2 != end) {
                // Only line_two is associated with an object.
                std::shared_ptr<std::vector<MapProcessing::Line>> obj = find2->second;

                // Insert the new line into the second line's object.
                obj->push_back(line_one);

                // Grow the object's bounding box to include the new line.
                this->object_sizes.at(obj).adopt(line_one);

                // Map the new line to the object.
                this->lines_to_object.emplace(line_one, obj);
            } else {
                // Neither of them are associated with an object.

                // Create a new object (really just a vector)
                std::shared_ptr<std::vector<MapProcessing::Line>> obj = std::shared_ptr<std::vector<MapProcessing::Line>>(new std::vector<MapProcessing::Line>());

                // Map both lines to the new object.
                this->lines_to_object.emplace(line_one, obj);
                this->lines_to_object.emplace(line_two, obj);

                // Insert them both into the new object
                obj->push_back(line_one);
                obj->push_back(line_two);

                // Add the object to the list of objects.
                this->objects.push_back(obj);

                // Create size information for the new object
                Object size = Object();

                // And grow it to include both lines.
                size.adopt(line_one).adopt(line_two);

                // Add the size information to the known list of objects.
                this->object_sizes.emplace(obj, size);
            }
        }
    });

    // Sort the objects by area.
    std::sort(this->objects.begin(), this->objects.end(), [this](const std::shared_ptr<std::vector<MapProcessing::Line>> first, std::shared_ptr<std::vector<MapProcessing::Line>> second) {
        Object& first_obj = this->object_sizes.at((std::shared_ptr<std::vector<MapProcessing::Line>>) first);
        Object& second_obj = this->object_sizes.at((std::shared_ptr<std::vector<MapProcessing::Line>>) second);
        return std::abs(((first_obj.max_x - first_obj.min_x) * (first_obj.max_y - first_obj.min_y))) > std::abs(((second_obj.max_x - second_obj.min_x) * (second_obj.max_y - second_obj.min_y)));
    });
};
