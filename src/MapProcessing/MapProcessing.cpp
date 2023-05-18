#include "albot/MapProcessing/MapProcessing.hpp"

namespace MapProcessing {
    std::shared_ptr<MapInfo> parse_map(const nlohmann::json& json) {

        // Create a new info smart pointer.
        std::shared_ptr<MapInfo> info = std::shared_ptr<MapInfo>(new MapInfo());

        // Grab references to the x_lines and y_lines json.
        const nlohmann::json& x_lines = json["x_lines"];
        const nlohmann::json& y_lines = json["y_lines"];

        // If they're both arrays
        if (x_lines.is_array() && y_lines.is_array()) {
            // Reserve enough memory for the x_lines
            info->x_lines.reserve(x_lines.size());
            // info->x_lines.emplace_back(min_x, min_y, max_y);
            // info->x_lines.emplace_back(max_x, min_y, max_y);
            // Iterate over all of the x_lines json
            for (const nlohmann::json& line : x_lines) {
                // Push each x_line into the info's array for x_lines
                AxisLineSegment _line = line.get<AxisLineSegment>();
                if (_line.range_end != _line.range_start) {
                    info->x_lines.emplace_back(_line);
                }
            }
            
            // Reserve enough memory for the y_lines
            info->y_lines.reserve(y_lines.size());
            // info->y_lines.emplace_back(min_y, min_x, max_x);
            // info->y_lines.emplace_back(max_y, min_x, max_x);
            // Iterate over all of the y_lines json
            for (const nlohmann::json& line : y_lines) {
                // Push each y_line into the info's array for y_lines
                AxisLineSegment _line = line.get<AxisLineSegment>();
                if (_line.range_end != _line.range_start) {
                    info->y_lines.emplace_back(_line);
                }
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

        // Iterate over all of the x_lines in the info.
        for (const AxisLineSegment& _line : info->x_lines) {
            x_lines_temp[_line.axis].emplace_back(_line.range_start, _line.range_end);
        }

        // Clear the existing x_lines
        info->x_lines.clear();

        // Iterate over the x_lines map
        for (std::pair<const short, std::vector<std::pair<short, short>>>& pair : x_lines_temp) {
            // We want to know what X we are dealing with
            short x = pair.first;

            // We need the array that is the list of Y ranges associated with that X
            std::vector<std::pair<short, short>>& ys = pair.second;
            std::sort(ys.begin(), ys.end(), [](const std::pair<short, short>& first, const std::pair<short, short>& second) {
                return std::min(first.first, first.second) < std::min(second.first, second.second);
            });
            // Iterate over the Y ranges, in a controlled manner.
            for (size_t i = 0, size = ys.size(); i < size; i++) {
                // Get the first Y range.
                std::pair<short, short>& first = ys[i];
                const short a = first.first;
                short b = first.second;
                // Iterate over them again, in a controlled manner.    
                for (size_t j = i + 1; j < size; j++) {
                    // Get the second Y range.
                    const std::pair<short, short>& second = ys[j];
                    const short c = second.first,
                                d = second.second;
                    // If the two lines overlap
                    // Preconditions: a must be less than c; because we sorted it.
                    // if b > c and c < d then they overlap.
                    if (b > c && b < d) {
                        // Make the first line a new line encompassing both of them.
                        b = d;
                        // And then skip to the second line's index.
                        i = j;
                    }
                }

                // Push the new Y range into the list.
                info->x_lines.emplace_back(x, a, b);
            }
        }

        // Rinse, repeat for Y lines.
        std::unordered_map<short, std::vector<std::pair<short, short>>> y_lines_temp = std::unordered_map<short, std::vector<std::pair<short, short>>>();
        for (const AxisLineSegment& _line : info->y_lines) {
            y_lines_temp[_line.axis].emplace_back(_line.range_start, _line.range_end);
        }
        info->y_lines.clear();
        for(std::pair<const short, std::vector<std::pair<short, short>>>& pair : y_lines_temp) {
            short y = pair.first;
            std::vector<std::pair<short, short>>& xs = pair.second;
            std::sort(xs.begin(), xs.end(), [](const std::pair<short, short>& first, const std::pair<short, short>& second) {
                return std::min(first.first, first.second) < std::min(second.first, second.second);
            });
            for (size_t i = 0, size = xs.size(); i < size; i++) {
                const std::pair<short, short>& first = xs[i];
                const short a = first.first;
                      short b = first.second;
                for(size_t j = i + 1; j < size; j++) {
                    const std::pair<short, short>& second = xs[j];
                    const short c = second.first,
                                d = second.second;
                    if (b > c && b < d) {
                        b = d;
                        i = j;
                    }
                }
                info->y_lines.emplace_back(y, a, b);
            }
        }
        return info;
    }
    void to_json(nlohmann::json& j, const AxisLineSegment& value) {
        j = nlohmann::json::array({value.axis, value.range_start, value.range_end});
    }
    void from_json(const nlohmann::json& j, AxisLineSegment& value) {
        j[0].get_to<short>(value.axis);
        j[1].get_to<short>(value.range_start);
        j[2].get_to<short>(value.range_end);
    }
}