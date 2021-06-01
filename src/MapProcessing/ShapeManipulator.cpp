#include "ShapeManipulator.hpp"
#include "../Common.hpp"

namespace ShapeManipulator {
    unsigned int hash(std::pair<short, short> pair) {
        return static_cast<unsigned int>(pair.first) << 16 | static_cast<unsigned short int>(pair.second);;
    }
    void from_list(std::vector<MapProcessing::Line>* list, triangulateio* output, Objectifier* objectifier) {
        std::unordered_map<unsigned int, int> points = std::unordered_map<unsigned int, int>();
        output->numberofsegments = list->size();
        output->segmentlist = (int *) malloc(output->numberofsegments * 2 * sizeof(int));
        output->segmentmarkerlist = (int *) malloc(output->numberofsegments * sizeof(int));
        int point_count = 0;
        for(std::vector<MapProcessing::Line>::iterator iter = list->begin(); iter != list->end(); iter++) {
            if(points.find(iter->first.hash) == points.end()) {
                point_count++;
                points.insert(std::pair<unsigned int, int>(iter->first.hash, point_count));
            }
            if(points.find(iter->second.hash) == points.end()) {
                point_count++;
                points.insert(std::pair<unsigned int, int>(iter->second.hash, point_count));
            }
        };
        points.clear();
        output->numberofpoints = point_count;
        output->pointlist = (REAL *) malloc(point_count * 2 * sizeof(REAL));
        output->pointmarkerlist = (int *) malloc(point_count * sizeof(int));
        int point_index = 0;
        int segment_index = 0;
        for(std::vector<MapProcessing::Line>::iterator iter = list->begin(); iter != list->end(); iter++) {
            if(points.find(iter->first.hash) == points.end()) {
                output->pointlist[point_index * 2] = iter->first.x;
                output->pointlist[point_index * 2 + 1] = iter->first.y;
                output->pointmarkerlist[point_index] = 1;
                point_index++;
                points.insert(std::pair<unsigned int, int>(iter->first.hash, point_index));
            }
            if(points.find(iter->second.hash) == points.end()) {
                output->pointlist[point_index * 2] = iter->second.x;
                output->pointlist[point_index * 2 + 1] = iter->second.y;
                output->pointmarkerlist[point_index] = 1;
                point_index++;
                points.insert(std::pair<unsigned int, int>(iter->second.hash, point_index));
            }
            output->segmentlist[segment_index * 2] = points[iter->first.hash];
            output->segmentlist[segment_index * 2 + 1] = points[iter->second.hash];
            output->segmentmarkerlist[segment_index] = 1;
            segment_index++;
        }
    }
    void from_list(std::vector<std::vector<MapProcessing::Line>*>& list_of_lists, triangulateio* output, Objectifier* objectifier) {
        std::map<unsigned int, int> points = std::map<unsigned int, int>();
        int num_segments = 0;
        for(const std::vector<MapProcessing::Line>* list : list_of_lists) {
            num_segments += list->size();
        }
        output->numberofsegments = num_segments;
        output->segmentlist = (int *) malloc(num_segments * 2 * sizeof(int));
        output->segmentmarkerlist = (int *) malloc(num_segments * sizeof(int));

        int point_count = 0;
        for(std::vector<MapProcessing::Line>* list : list_of_lists) {
            for(std::vector<MapProcessing::Line>::iterator iter = list->begin(); iter != list->end(); iter++) {
                if(points.find(iter->first.hash) == points.end()) {
                    point_count++;
                    points.insert(std::pair<unsigned int, int>(iter->first.hash, point_count));
                }
                if(points.find(iter->second.hash) == points.end()) {
                    point_count++;
                    points.insert(std::pair<unsigned int, int>(iter->second.hash, point_count));
                }
            };
        }
        points.clear();
        output->numberofpoints = point_count;
        output->pointlist = (REAL *) malloc(point_count * 2 * sizeof(REAL));
        output->pointmarkerlist = (int *) malloc(point_count * sizeof(int));
        int point_index = 0;
        int segment_index = 0;
        for(std::vector<MapProcessing::Line>* list : list_of_lists) {
            for(std::vector<MapProcessing::Line>::iterator iter = list->begin(); iter != list->end(); iter++) {
            if(points.find(iter->first.hash) == points.end()) {
                output->pointlist[point_index * 2] = iter->first.x;
                output->pointlist[point_index * 2 + 1] = iter->first.y;
                output->pointmarkerlist[point_index] = 1;
                point_index++;
                points.insert(std::pair<unsigned int, int>(iter->first.hash, point_index));
            }
            if(points.find(iter->second.hash) == points.end()) {
                output->pointlist[point_index * 2] = iter->second.x;
                output->pointlist[point_index * 2 + 1] = iter->second.y;
                output->pointmarkerlist[point_index] = 1;
                point_index++;
                points.insert(std::pair<unsigned int, int>(iter->second.hash, point_index));
            }
            output->segmentlist[segment_index * 2] = points[iter->first.hash];
            output->segmentlist[segment_index * 2 + 1] = points[iter->second.hash];
            output->segmentmarkerlist[segment_index] = 1;
            segment_index++;
        }
        }
    }
}