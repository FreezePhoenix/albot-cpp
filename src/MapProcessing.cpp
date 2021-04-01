#include "MapProcessing.hpp"

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

    typedef std::map<short, std::vector<Tuple>> shortTupleVector;
    MapInfo* parseMap(const nlohmann::json& json) {
        MapInfo* info = new MapInfo();
        std::cout << "Parsing map..." << std::endl;
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
                    std::vector<short>* line = new std::vector<short>();
                    line->push_back(_line[0]);
                    line->push_back(_line[1]);
                    line->push_back(_line[2]);
                    info->y_lines.push_back(*line);
                }
            }
        }
        return info;
    }
    MapInfo* simplify_lines(MapInfo* info) {
        std::cout << "Simplifying..." << std::endl;
        short before, after;
        shortTupleVector x_lines_temp = std::map<short, std::vector<Tuple>>();
        std::vector<Tuple> new_ys = std::vector<Tuple>(); 
        before = info->x_lines.size();
        for(int i = 0, size = info->x_lines.size(); i < size; i++) {
            std::vector<short> _line = info->x_lines[i];
            Tuple tup = Tuple(_line[1], _line[2]);
            if(x_lines_temp.find(_line[0]) != x_lines_temp.end()) {
                x_lines_temp[_line[0]].push_back(tup);
            } else {
                x_lines_temp[_line[0]] = std::vector<Tuple>();
                x_lines_temp[_line[0]].push_back(tup);
            }
        }
        info->x_lines.clear();
        for(shortTupleVector::iterator x_it = x_lines_temp.begin(); x_it != x_lines_temp.end(); x_it++) {
            short x = x_it->first;
            std::vector<Tuple> ys = x_it->second;
            for(int i = 0, size = ys.size(); i < size; i++) {
                Tuple first = ys[i];
                for(int j = 0; j < size; j++) {
                    Tuple second = ys[j];
                    if(first.hash != second.hash) {
                        short a = first.first,
                            b = first.second,
                            c = second.first,
                            d = second.second;
                        if(overlaps(a, b, c, d)) {
                            short new_start = MapProcessing::min(a, b, c, d);
                            short new_end = MapProcessing::max(a, b, c, d);
                            first = Tuple(new_start, new_end);
                            i = j;
                        }
                    }
                }
                new_ys.push_back(first);
            }
            for(int i = 0, size = new_ys.size(); i < size; i++) {
                Tuple tup = new_ys[i];
                std::vector<short> line = std::vector<short>();
                line.push_back(x);
                line.push_back(tup.first);
                line.push_back(tup.second);
                info->x_lines.push_back(line);
            }
            new_ys.clear();
        }
        after = info->x_lines.size();
        std::cout << "x_lines before: " << std::to_string(before) << " after: " << std::to_string(after) << std::endl;
        before = after = 0;
        shortTupleVector y_lines_temp = std::map<short, std::vector<Tuple>>();
        std::vector<Tuple> new_xs = std::vector<Tuple>();
        before = info->y_lines.size();
        for(int i = 0, size = info->y_lines.size(); i < size; i++) {
            std::vector<short> _line = info->y_lines[i];
            Tuple tup = Tuple(_line[1], _line[2]);
            if(y_lines_temp.find(_line[0]) != y_lines_temp.end()) {
                y_lines_temp[_line[0]].push_back(tup);
            } else {
                y_lines_temp[_line[0]] = std::vector<Tuple>();
                y_lines_temp[_line[0]].push_back(tup);
            }
        }
        info->y_lines.clear();
        for(shortTupleVector::iterator y_it = y_lines_temp.begin(); y_it != y_lines_temp.end(); ++y_it) {
            short y = y_it->first;
            std::vector<Tuple> xs = y_it->second;
            for(int i = 0, size = xs.size(); i < size; i++) {
                Tuple first = xs[i];
                for(int j = 0; j < size; j++) {
                    Tuple second = xs[j];
                    if(first.hash != second.hash) {
                        short a = first.first,
                            b = first.second,
                            c = second.first,
                            d = second.second;
                        if(overlaps(a, b, c, d)) {
                            short new_start = MapProcessing::min(a, b, c, d);
                            short new_end = MapProcessing::max(a, b, c, d);
                            first = Tuple(new_start, new_end);
                            i = j;
                        }
                    }
                }
                new_xs.push_back(first);
            }
            for(int i = 0, size = new_xs.size(); i < size; i++) {
                Tuple tup = new_xs[i];
                std::vector<short> line = std::vector<short>();
                line.push_back(y);
                line.push_back(tup.first);
                line.push_back(tup.second);
                info->y_lines.push_back(line);
            }
            new_xs.clear();
        }
        after = info->y_lines.size();
        std::cout << "y_lines before: " << std::to_string(before) << " after: " << std::to_string(after) << std::endl;
        return info;
    }
}