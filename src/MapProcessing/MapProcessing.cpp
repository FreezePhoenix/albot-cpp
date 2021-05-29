#include "MapProcessing.hpp"
#include <iostream>
#include "../Common.hpp"

namespace MapProcessing {
    bool  overlaps(short a, short b, short c, short d) {
        short start1 = std::min(a, b);
        short end1 = std::max(a, b);
        short start2 = std::min(c, d);
        short end2 = std::max(c, d);
        if(start1 <= start2 && start2 <= end1) {
            return true;
        }
        if(start1 <= end2 && end2 <= end1) {
            return true;
        }
        if(start2 <= start1 && start1 <= end2) {
            return true;
        }
        if(start2 <= end1 && end1 <= end2) {
            return true;
        }
        return false;
    }
    typedef std::unordered_map<short, std::vector<std::pair<short, short>>> shortTupleVector;
    MapInfo* parse_map(nlohmann::json& json) {
        MapInfo* info = new MapInfo();
        nlohmann::json x_lines = json["x_lines"];
        nlohmann::json y_lines = json["y_lines"];
        if(x_lines.is_array() && y_lines.is_array()) {
            for(nlohmann::json::iterator x_it = x_lines.begin(); x_it != x_lines.end(); ++x_it) {
                if(x_it.value().is_array()) {
                    std::vector<short> _line = x_it.value().get<std::vector<short>>();
                    _line.resize(3);
                    info->x_lines.push_back(_line);
                }
            }
            for(nlohmann::json::iterator y_it = y_lines.begin(); y_it != y_lines.end(); ++y_it) {
                if(y_it.value().is_array()) {
                    std::vector<short> _line = y_it.value().get<std::vector<short>>();
                    _line.resize(3);
                    info->y_lines.push_back(_line);
                }
            }
        }
        return info;
    }
    MapInfo* simplify_lines(MapInfo* info) {
        shortTupleVector x_lines_temp = std::unordered_map<short, std::vector<std::pair<short, short>>>();
        std::vector<std::pair<short, short>> new_ys = std::vector<std::pair<short, short>>(); 
        for(const std::vector<short>& _line : info->x_lines) {
            std::pair<short, short> tup = std::pair<short, short>(_line[1], _line[2]);
            if(x_lines_temp.find(_line[0]) != x_lines_temp.end()) {
                x_lines_temp[_line[0]].push_back(tup);
            } else {
                x_lines_temp[_line[0]] = std::vector<std::pair<short, short>>();
                x_lines_temp[_line[0]].push_back(tup);
            }
        }
        info->x_lines.clear();
        for(const std::pair<short, std::vector<std::pair<short, short>>>& pair : x_lines_temp) {
            short x = pair.first;
            std::vector<std::pair<short, short>> ys = pair.second;
            for(int i = 0, size = ys.size(); i < size; i++) {
                std::pair<short, short> first = ys[i];
                for(int j = 0; j < size; j++) {
                    std::pair<short, short> second = ys[j];
                    if(first == second) {
                        short a = first.first,
                            b = first.second,
                            c = second.first,
                            d = second.second;
                        if(overlaps(a, b, c, d)) {
                            short new_start = MapProcessing::min(a, b, c, d);
                            short new_end = MapProcessing::max(a, b, c, d);
                            first.first = new_start;
                            first.second = new_end;
                            i = j;
                        }
                    }
                }
                new_ys.push_back(first);
            }
            for(const std::pair<short, short>& tup : new_ys) {
                info->x_lines.push_back(std::vector<short> { x, tup.first, tup.second });
            }
            new_ys.clear();
        }
        shortTupleVector y_lines_temp = std::unordered_map<short, std::vector<std::pair<short, short>>>();
        std::vector<std::pair<short, short>> new_xs = std::vector<std::pair<short, short>>();
        for(int i = 0, size = info->y_lines.size(); i < size; i++) {
            std::vector<short> _line = info->y_lines[i];
            std::pair<short, short> tup = std::pair<short, short>(_line[1], _line[2]);
            if(y_lines_temp.find(_line[0]) != y_lines_temp.end()) {
                y_lines_temp[_line[0]].push_back(tup);
            } else {
                y_lines_temp[_line[0]] = std::vector<std::pair<short, short>>();
                y_lines_temp[_line[0]].push_back(tup);
            }
        }
        info->y_lines.clear();
        for(shortTupleVector::iterator y_it = y_lines_temp.begin(); y_it != y_lines_temp.end(); ++y_it) {
            short y = y_it->first;
            std::vector<std::pair<short, short>> xs = y_it->second;
            for(int i = 0, size = xs.size(); i < size; i++) {
                std::pair<short, short> first = xs[i];
                for(int j = 0; j < size; j++) {
                    std::pair<short, short> second = xs[j];
                    if(first != second) {
                        short a = first.first,
                            b = first.second,
                            c = second.first,
                            d = second.second;
                        if(overlaps(a, b, c, d)) {
                            short new_start = MapProcessing::min(a, b, c, d);
                            short new_end = MapProcessing::max(a, b, c, d);
                            first.first = new_start;
                            first.second = new_end;
                            i = j;
                        }
                    }
                }
                new_xs.push_back(first);
            }
            for(const std::pair<short, short>& tup : new_xs) {
                info->y_lines.push_back(std::vector<short> { y, tup.first, tup.second });
            }
            new_xs.clear();
        }
        return info;
    }

}