#pragma once

#ifndef ALBOT_MAPPROCESSING_HPP_
#define ALBOT_MAPPROCESSING_HPP_

#include <map>
#include "PointLocation.hpp"
#include <vector>
#include <math.h>
#include <nlohmann/json.hpp>
#include <any>

namespace MapProcessing {
    struct AxisLineSegment {
        short axis;
        short range_start;
        short range_end;
    };
    void to_json(nlohmann::json& j, const AxisLineSegment& value);
    void from_json(const nlohmann::json& j, AxisLineSegment& value);
    struct MapInfo {
        std::string name;
        std::vector<AxisLineSegment> x_lines;
        std::vector<AxisLineSegment> y_lines;
        std::vector<std::pair<double, double>> spawns; 
    };
    typedef PointLocation::Point Point;
    typedef PointLocation::PointHash PointHash;
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
    // Accepts the "G.maps[<map_name>].data" property."
    // Read: Actually accepts "G.geometry[<map_name>]"
    // Outputs a MapInfo, which contains all the lines in the map.
    std::shared_ptr<MapInfo> parse_map(nlohmann::json& json);
    void process(std::shared_ptr<MapInfo> json);
    // Accepts a map info, and simplifies it, remove unnecessary lines.
    std::shared_ptr<MapInfo> simplify_lines(std::shared_ptr<MapInfo> info);
}

#endif /* MAPPROCESSING_HPP_ */