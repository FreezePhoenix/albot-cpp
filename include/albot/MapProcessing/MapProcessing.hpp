#pragma once

#ifndef ALBOT_MAPPROCESSING_HPP_
#define ALBOT_MAPPROCESSING_HPP_

#include <vector>
#include <nlohmann/json.hpp>
#include "TriangleManipulator/PointLocation.hpp"

namespace MapProcessing {
    /**
     * @brief A structure describing a line that is not bound to a specific axis.
     */
    struct AxisLineSegment {
        short axis;
        short range_start;
        short range_end;
    };
    
    // Utility methods to turn JSON to/from AxisLineSegments
    void to_json(nlohmann::json& j, const AxisLineSegment& value);
    void from_json(const nlohmann::json& j, AxisLineSegment& value);
    
    /**
     * @brief A structure containing the name of a map, and the x_lines as well as the y_lines for it, and the spawns.
     */
    struct MapInfo {
        std::string name;
        std::vector<AxisLineSegment> x_lines;
        std::vector<AxisLineSegment> y_lines;
        std::vector<std::pair<double, double>> spawns; 
    };

    // Typedefs for ease of use.
    typedef PointLocation::Point Point;
    typedef PointLocation::Line Line;
    typedef PointLocation::LineHash LineHash;
    typedef PointLocation::Triangle Triangle;
    
    // I really hate using templates but god do they look cool
    template<typename _Tp>
    inline const _Tp& min(const _Tp& a, const _Tp& b, const _Tp& c, const _Tp& d) {
        return std::min<_Tp>(std::min<_Tp>(a, b), std::min<_Tp>(c, d));
    }
    template<typename _Tp>
    inline const _Tp& max(const _Tp& a, const _Tp& b, const _Tp& c, const _Tp& d) {
        return std::max<_Tp>(std::max<_Tp>(a, b), std::max<_Tp>(c, d));
    }
    /**
     * @brief Turns a map json into a MapInfo shared pointer. Accepts G.geometry[<map_name>]
     * 
     * @param json 
     * @return std::shared_ptr<MapInfo> 
     */
    std::shared_ptr<MapInfo> parse_map(const nlohmann::json& json);
    
    /**
     * @brief Accepts a MapInfo, simplifies it by removing unecessary lines, and then returns it.
     * 
     * @param info 
     * @return std::shared_ptr<MapInfo> 
     */
    std::shared_ptr<MapInfo> simplify_lines(std::shared_ptr<MapInfo> info);
}

#endif /* MAPPROCESSING_HPP_ */