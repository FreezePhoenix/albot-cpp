#pragma once

#ifndef ALBOT_MAPPROCESSING_HPP_
#define ALBOT_MAPPROCESSING_HPP_

#include <map>
#include <vector>
#include <math.h>
#include <nlohmann/json.hpp>
#include <any>

namespace MapProcessing {
    struct MapInfo {
        std::string name;
        std::vector<std::vector<short>> x_lines;
        std::vector<std::vector<short>> y_lines;
    };
    struct Point {
        short x;
        short y;
        unsigned int hash;
        Point() {
            this->x = this->y = 0;
            this->hash = 0;
        }
        Point(short x, short y) {
            this->x = x;
            this->y = y;
            this->hash = ((unsigned int) x << 16) + (unsigned int) y;
        };
        bool operator==(const Point& other) {
            return other.hash == this->hash;
        }
    };
    struct PointHash {
        inline unsigned int operator () (const Point &p) const {
            return p.hash;
        }
        inline std::size_t operator () (const Point &p, const Point &p2) const {
            return p.hash - p2.hash;
        }
    };
    struct Line {
        Point first;
        Point second;
        unsigned long int hash;
        Line() {
            this->first = Point(0, 0);
            this->second = Point(0, 0);
            this->hash = 0;
        }
        Line(Point& first, Point& second) {
            if(first.x < second.x || first.y < second.y) {
                this->first = first;
                this->second = second;
            } else {
                this->first = second;
                this->second = first;
            }
            this->hash = ((unsigned long int) this->first.hash << 32) + (unsigned long int) this->second.hash;
        }
        Line(const Point& first, const Point& second) {
            if(first.x < second.x || first.y < second.y) {
                this->first = first;
                this->second = second;
            } else {
                this->first = second;
                this->second = first;
            }
            this->hash = ((unsigned long int) this->first.hash << 32) + (unsigned long int) this->second.hash;
        }
        bool operator==(const Line& other) {
            return other.hash == this->hash;
        }
        Line(short x1, short x2, short y1, short y2): Line(Point(x1, y1), Point(x2, y2)) {}
    };

    struct LineHash {
        inline unsigned long int operator () (const Line &l) const {
            return l.hash;
        }
        inline std::size_t operator () (const Line &p, const Line &p2) const {
            return p.hash - p2.hash;
        }
    };

    inline bool overlaps(short a, short b, short c, short d);

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
    void process(MapInfo* json);
    // Accepts a map info, and simplifies it, remove unnecessary lines.
    MapInfo* simplify_lines(MapInfo* info);
}
namespace std {
    inline bool operator==(const MapProcessing::Point& first, const MapProcessing::Point& second) {
        return first.hash == second.hash;
    }
    inline bool operator==(const MapProcessing::Line& first, const MapProcessing::Line& second) {
        return first.hash == second.hash;
    }
}
#endif /* MAPPROCESSING_HPP_ */