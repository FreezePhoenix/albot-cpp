#include "MapProcessing.hpp"
#include "Objectifier.hpp"
#include "Writer.hpp"
#include <iostream>

namespace MapProcessing {
    void process(std::shared_ptr<MapInfo> info) {
        Objectifier objectifier(info);
        objectifier.run();
        Writer writer(objectifier);
        writer.write();
    }

    std::shared_ptr<MapInfo> parse_map(nlohmann::json& json) {

        // Create a new info smart pointer.
        std::shared_ptr<MapInfo> info = std::shared_ptr<MapInfo>(new MapInfo());

        // Grab references to the x_lines and y_lines json.
        nlohmann::json& x_lines = json["x_lines"];
        nlohmann::json& y_lines = json["y_lines"];

        // If they're both arrays
        if (x_lines.is_array() && y_lines.is_array()) {
            // Reserve enough memory for the x_lines
            info->x_lines.reserve(x_lines.size());
            
            // Iterate over all of the x_lines json
            for (const nlohmann::json& line : x_lines) {
                // Push each x_line into the info's array for x_lines
                info->x_lines.push_back(line.get<AxisLineSegment>());
            }
            // Reserve enough memory for the y_lines
            info->y_lines.reserve(y_lines.size());

            // Iterate over all of the y_lines json
            for (const nlohmann::json& line : y_lines) {
                // Push each y_line into the info's array for y_lines
                info->y_lines.push_back(line.get<AxisLineSegment>());
            }
        }
        return info;
    }
    std::shared_ptr<MapInfo> simplify_lines(std::shared_ptr<MapInfo> info) {
        /**
         * @brief A temporary map to hold x_lines
         * Maps an X value to a list of ranges of Y values.
         */
        std::unordered_map<short, std::vector<std::pair<short, short>>> x_lines_temp = std::unordered_map<short, std::vector<std::pair<short, short>>>();

        /**
         * @brief An array to hold the new ranges of Y values
         * 
         */
        std::vector<std::pair<short, short>> new_ys = std::vector<std::pair<short, short>>();

        // Reserve enough memory.
        new_ys.reserve(info->x_lines.size());

        // Iterate over all of the x_lines in the info.
        for (const AxisLineSegment& _line : info->x_lines) {
            // Create a pair (a tuple with 2 elements) from the line's start and end values.
            std::pair<short, short> tup = std::pair<short, short>(_line.range_start, _line.range_end);

            // See if we have Y values associated with that X
            if (x_lines_temp.count(_line.axis)) {
                // If we do, just add it to the end of the list.
                x_lines_temp.at(_line.axis).push_back(tup);
            } else {
                // If we don't, create it, and add to the end of the list.
                x_lines_temp.emplace(_line.axis, std::vector<std::pair<short, short>>());
                x_lines_temp.at(_line.axis).push_back(tup);
            }
        }

        // Clear the existing x_lines
        info->x_lines.clear();

        // Iterate over the x_lines map
        for (std::pair<const short, std::vector<std::pair<short, short>>>& pair : x_lines_temp) {
            // We want to know what X we are dealing with
            short x = pair.first;

            // We need the array that is the list of Y ranges associated with that X
            std::vector<std::pair<short, short>> ys = pair.second;

            // Iterate over the Y ranges, in a controlled manner.
            for (int i = 0, size = ys.size(); i < size; i++) {
                // Get the first Y range.
                std::pair<short, short>& first = ys.at(i);
                
                // Iterate over them again, in a controlled manner.    
                for (int j = i + 1; j < size; j++) {
                    // Get the second Y range.
                    const std::pair<short, short>& second = ys.at(j);
                    short a = first.first,
                        b = first.second,
                        c = second.first,
                        d = second.second;

                    // If the two lines overlap
                    if (PointLocation::overlaps(a, b, c, d)) {
                        // Make the first line a new line encompassing both of them.
                        short new_start = MapProcessing::min(a, b, c, d);
                        short new_end = MapProcessing::max(a, b, c, d);
                        first.first = new_start;
                        first.second = new_end;

                        // And then skip to the second line's index.
                        i = j;
                    }
                }

                // Push the new Y range into the list.
                new_ys.push_back(first);
            }

            // Iterate over the new Y ranges
            for (const std::pair<short, short>& tup : new_ys) {
                // Insert them into the x_lines.
                info->x_lines.emplace_back(x, tup.first, tup.second);
            }

            // Clear our temporary array.
            new_ys.clear();
        }

        // Rinse, repeat for Y lines.
        std::unordered_map<short, std::vector<std::pair<short, short>>> y_lines_temp = std::unordered_map<short, std::vector<std::pair<short, short>>>();
        std::vector<std::pair<short, short>> new_xs = std::vector<std::pair<short, short>>();
        new_xs.reserve(info->y_lines.size());
        for (const AxisLineSegment& _line : info->y_lines) {
            std::pair<short, short> tup = std::pair<short, short>(_line.range_start, _line.range_end);
            if(y_lines_temp.count(_line.axis)) {
                y_lines_temp.at(_line.axis).push_back(tup);
            } else {
                y_lines_temp.emplace(_line.axis, std::vector<std::pair<short, short>>());
                y_lines_temp.at(_line.axis).push_back(tup);
            }
        }
        info->y_lines.clear();
        for(std::pair<const short, std::vector<std::pair<short, short>>>& pair : y_lines_temp) {
            short y = pair.first;
            std::vector<std::pair<short, short>>& xs = pair.second;
            for(int i = 0, size = xs.size(); i < size; i++) {
                std::pair<short, short>& first = xs.at(i);
                for(int j = i + 1; j < size; j++) {
                    const std::pair<short, short>& second = xs.at(j);
                    short a = first.first,
                        b = first.second,
                        c = second.first,
                        d = second.second;
                    if(PointLocation::overlaps(a, b, c, d)) {
                        short new_start = MapProcessing::min(a, b, c, d);
                        short new_end = MapProcessing::max(a, b, c, d);
                        first.first = new_start;
                        first.second = new_end;
                        i = j;
                    }
                }
                new_xs.push_back(first);
            }
            for(const std::pair<short, short>& tup : new_xs) {

                info->y_lines.emplace_back(y, tup.first, tup.second);
            }
            new_xs.clear();
        }
        return info;
    }
    void to_json(nlohmann::json& j, const AxisLineSegment& value) {
        j = nlohmann::json::array({value.axis, value.range_start, value.range_end});
    }
    void from_json(const nlohmann::json& j, AxisLineSegment& value) {
        j.at(0).get_to<short>(value.axis);
        j.at(1).get_to<short>(value.range_start);
        j.at(2).get_to<short>(value.range_end);
    }
}