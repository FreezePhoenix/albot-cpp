#include <map>
#include <math.h>
#include <vector>
#include <nlohmann/json.hpp>
#include <iostream>

namespace MapProcessing {
    struct Tuple {
        short first;
        short second;
        unsigned int hash;
        Tuple(short first, short second) {
            this->first = first;
            this->second = second;
            hash = static_cast<unsigned short int>(first) << 16 | static_cast<unsigned short int>(second);
        }
    };
    struct MapInfo {
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
    // Accepts the "G.maps[<map_name>].data property."
    MapInfo* parseMap(const nlohmann::json& json);

    MapInfo* simplify_lines(MapInfo* info);
}