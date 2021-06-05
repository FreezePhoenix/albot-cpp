#include "ShapeManipulator.hpp"

namespace ShapeManipulator {
    unsigned int hash(std::pair<short, short> pair) {
        return static_cast<unsigned int>(pair.first) << 16 | static_cast<unsigned short int>(pair.second);;
    }
    std::vector<std::pair<REAL, REAL>> find_points_inside(const std::vector<MapProcessing::Line>& object, triangulateio* triangle_object) {
        std::vector<std::pair<REAL, REAL>> holes = std::vector<std::pair<REAL, REAL>>();
        from_list(object, triangle_object);
        triangulateio* triangle_vobject = TriangleManipulator::create_instance();
        triangulateio* _output = TriangleManipulator::create_instance();
        triangulate("pvPQD", triangle_object, _output, triangle_vobject);
        do {
            REAL x = triangle_vobject->pointlist[0];
            REAL y = triangle_vobject->pointlist[1];
            holes.push_back(std::pair<REAL, REAL>(x, y));
            triangle_object->holelist = (REAL *) NULL;
            triangle_object->numberofholes = holes.size();
            triangle_object->holelist = (REAL *) malloc(holes.size() * 2 * sizeof(REAL));
            int hole_index = 0;
            for(const std::pair<REAL, REAL>& hole : holes) {
                triangle_object->holelist[hole_index * 2] = hole.first;
                triangle_object->holelist[hole_index * 2 + 1] = hole.second;
                hole_index++;
            }
            triangulate("pvPQD", triangle_object, _output, triangle_vobject);
        } while(triangle_vobject->numberofpoints != 0);
        TriangleManipulator::cleanup(triangle_object);
        TriangleManipulator::cleanup(triangle_vobject);
        TriangleManipulator::cleanup(_output);
        return holes;
    }
    std::vector<std::pair<REAL, REAL>> find_points_inside(triangulateio* input) {
        std::vector<std::pair<REAL, REAL>> holes = std::vector<std::pair<REAL, REAL>>();
        triangulateio* triangle_vobject = TriangleManipulator::create_instance();
        triangulateio* _output = TriangleManipulator::create_instance();
        triangulate("pvPQD", input, _output, triangle_vobject);
        do {
            if(triangle_vobject->numberofpoints < 1) {
                continue;
            }
            REAL x = triangle_vobject->pointlist[0];
            REAL y = triangle_vobject->pointlist[1];
            holes.push_back(std::pair<REAL, REAL>(x, y));
            input->numberofholes = holes.size();
            input->holelist = (REAL *) malloc(holes.size() * 2 * sizeof(REAL));
            int hole_index = 0;
            for(const std::pair<REAL, REAL>& hole : holes) {
                input->holelist[hole_index * 2] = hole.first;
                input->holelist[hole_index * 2 + 1] = hole.second;
                hole_index++;
            }
            // TriangleManipulator::cleanup(_output);
            // TriangleManipulator::cleanup(triangle_vobject);
            triangulate("pvPQD", input, _output, triangle_vobject);
        } while(triangle_vobject->numberofpoints != 0);
        // TriangleManipulator::cleanup(triangle_object);
        // TriangleManipulator::cleanup(triangle_vobject);
        // TriangleManipulator::cleanup(_output);
        return holes;
    }
    void from_list(const std::vector<MapProcessing::Line>& list, triangulateio* output) {
        std::unordered_map<unsigned int, unsigned short> points = std::unordered_map<unsigned int, unsigned short>();
        output->numberofsegments = list.size();
        output->segmentlist = (int *) malloc(output->numberofsegments * 2 * sizeof(int));
        output->segmentmarkerlist = (int *) malloc(output->numberofsegments * sizeof(int));
        int point_count = 0;
        std::unordered_set<unsigned int> points_set = std::unordered_set<unsigned int>();
        for(const MapProcessing::Line& line : list) {
            if(points_set.find(line.first.hash) == points_set.end()) {
                point_count++;
                points_set.insert(line.first.hash);
            }
            if(points_set.find(line.second.hash) == points_set.end()) {
                point_count++;
                points_set.insert(line.second.hash);
            }
        }
        points_set.clear();
        output->numberofpoints = point_count;
        output->pointlist = (REAL *) malloc(point_count * 2 * sizeof(REAL));
        output->pointmarkerlist = (int *) malloc(point_count * sizeof(int));
        unsigned short point_index = 0;
        int segment_index = 0;
        for(const MapProcessing::Line& line : list) {
            const MapProcessing::Point& first = line.first;
            const MapProcessing::Point& second = line.second;
            std::unordered_map<unsigned int, unsigned short>::iterator first_find = points.find(first.hash);
            if(first_find == points.end()) {
                output->pointlist[point_index * 2] = first.x;
                output->pointlist[point_index * 2 + 1] = first.y;
                output->pointmarkerlist[point_index] = 1;
                point_index++;
                first_find = points.emplace(first.hash, point_index).first;
            }
            std::unordered_map<unsigned int, unsigned short>::iterator second_find = points.find(second.hash);
            if(second_find == points.end()) {
                output->pointlist[point_index * 2] = second.x;
                output->pointlist[point_index * 2 + 1] = second.y;
                output->pointmarkerlist[point_index] = 1;
                point_index++;
                second_find = points.emplace(second.hash, point_index).first;
            }
            output->segmentlist[segment_index * 2] = first_find->second;
            output->segmentlist[segment_index * 2 + 1] = second_find->second;
            output->segmentmarkerlist[segment_index] = 1;
            segment_index++;
        }
    }
    void from_list(std::vector<MapProcessing::Line>* list, triangulateio* output) {
        std::unordered_map<unsigned int, unsigned short> points = std::unordered_map<unsigned int, unsigned short>();
        output->numberofsegments = list->size();
        output->segmentlist = (int *) malloc(output->numberofsegments * 2 * sizeof(int));
        output->segmentmarkerlist = (int *) malloc(output->numberofsegments * sizeof(int));

        int point_count = 0;
        std::unordered_set<unsigned int> points_set = std::unordered_set<unsigned int>();
        
        for(std::vector<MapProcessing::Line>::iterator iter = list->begin(); iter != list->end(); iter++) {
            if(points_set.find(iter->first.hash) == points_set.end()) {
                point_count++;
                points_set.emplace(iter->first.hash);
            }
            if(points_set.find(iter->second.hash) == points_set.end()) {
                point_count++;
                points_set.emplace(iter->second.hash);
            }
        }
        output->numberofpoints = point_count;
        output->pointlist = (REAL *) malloc(point_count * 2 * sizeof(REAL));
        output->pointmarkerlist = (int *) malloc(point_count * sizeof(int));
        unsigned point_index = 0;
        int segment_index = 0;
        for(std::vector<MapProcessing::Line>::iterator iter = list->begin(); iter != list->end(); iter++) {
            const MapProcessing::Point& first = iter->first;
            const MapProcessing::Point& second = iter->second;
            std::unordered_map<unsigned int, unsigned short>::iterator first_find = points.find(first.hash);
            if(first_find == points.end()) {
                output->pointlist[point_index * 2] = first.x;
                output->pointlist[point_index * 2 + 1] = first.y;
                output->pointmarkerlist[point_index] = 1;
                point_index++;
                first_find = points.emplace(first.hash, point_index).first;
            }
            std::unordered_map<unsigned int, unsigned short>::iterator second_find = points.find(second.hash);
            if(second_find == points.end()) {
                output->pointlist[point_index * 2] = second.x;
                output->pointlist[point_index * 2 + 1] = second.y;
                output->pointmarkerlist[point_index] = 1;
                point_index++;
                second_find = points.emplace(second.hash, point_index).first;
            }
            output->segmentlist[segment_index * 2] = first_find->second;
            output->segmentlist[segment_index * 2 + 1] = second_find->second;
            output->segmentmarkerlist[segment_index] = 1;
            segment_index++;
        }
    }
    void from_list(const std::vector<std::vector<MapProcessing::Line>*>& list_of_lists, triangulateio* output) {
        std::unordered_map<unsigned int, unsigned short> points = std::unordered_map<unsigned int, unsigned short>();
        int num_segments = 0;
        for(const std::vector<MapProcessing::Line>* list : list_of_lists) {
            num_segments += list->size();
        }
        output->numberofsegments = num_segments;
        output->segmentlist = (int *) malloc(num_segments * 2 * sizeof(int));
        output->segmentmarkerlist = (int *) malloc(num_segments * sizeof(int));

        int point_count = 0;
        std::unordered_set<unsigned int> points_set = std::unordered_set<unsigned int>();
        
        for(std::vector<MapProcessing::Line>* list : list_of_lists) {
            for(std::vector<MapProcessing::Line>::iterator iter = list->begin(); iter != list->end(); iter++) {
                if(points_set.find(iter->first.hash) == points_set.end()) {
                    point_count++;
                    points_set.emplace(iter->first.hash);
                }
                if(points_set.find(iter->second.hash) == points_set.end()) {
                    point_count++;
                    points_set.emplace(iter->second.hash);
                }
            };
        }
        points_set.clear();
        output->numberofpoints = point_count;
        output->pointlist = (REAL *) malloc(point_count * 2 * sizeof(REAL));
        output->pointmarkerlist = (int *) malloc(point_count * sizeof(int));
        unsigned short point_index = 0;
        int segment_index = 0;
        for(std::vector<MapProcessing::Line>* list : list_of_lists) {
            for(std::vector<MapProcessing::Line>::iterator iter = list->begin(); iter != list->end(); iter++) {
                const MapProcessing::Point first = iter->first;
                const MapProcessing::Point second = iter->second;
                std::unordered_map<unsigned int, unsigned short>::iterator first_find = points.find(first.hash);
                if(first_find == points.end()) {
                    output->pointlist[point_index * 2] = first.x;
                    output->pointlist[point_index * 2 + 1] = first.y;
                    output->pointmarkerlist[point_index] = 1;
                    point_index++;
                    first_find = points.emplace(first.hash, point_index).first;
                }
                std::unordered_map<unsigned int, unsigned short>::iterator second_find = points.find(second.hash);
                if(second_find == points.end()) {
                    output->pointlist[point_index * 2] = second.x;
                    output->pointlist[point_index * 2 + 1] = second.y;
                    output->pointmarkerlist[point_index] = 1;
                    point_index++;
                    second_find = points.emplace(second.hash, point_index).first;
                }
                output->segmentlist[segment_index * 2] = first_find->second;
                output->segmentlist[segment_index * 2 + 1] = second_find->second;
                output->segmentmarkerlist[segment_index] = 1;
                segment_index++;
            }
        }
    }
}