#pragma once

#ifndef ALBOT_MAPPROCESSING_HPP_
#define ALBOT_MAPPROCESSING_HPP_

#include <map>
#include <vector>
#include <math.h>
#include <nlohmann/json.hpp>

namespace MapProcessing {
    typedef std::pair<short, short> Tuple;
    struct MapInfo {
        std::string name;
        std::vector<std::vector<short>> x_lines;
        std::vector<std::vector<short>> y_lines;
    };

    bool overlaps(short a, short b, short c, short d);

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
    MapInfo* parse_map(nlohmann::json& json);
    // Accepts a map info, and simplifies it, remove unnecessary lines.
    MapInfo* simplify_lines(MapInfo* info);
}

#endif /* MAPPROCESSING_HPP_ */