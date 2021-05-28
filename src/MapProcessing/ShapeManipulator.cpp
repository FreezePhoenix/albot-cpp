#include "ShapeManipulator.hpp"

namespace ShapeManipulator {
    void from_list(DoubleLinkedList<MapProcessing::Tuple>* list, triangulateio* output) {
        std::map<MapProcessing::TupleHash, int> points = std::map<MapProcessing::TupleHash, int>();
        output->numberofsegments = list->length - 1;
        output->segmentlist = (int *) malloc((list->length - 1) * 2 * sizeof(int));
        output->segmentmarkerlist = (int *) malloc((list->length - 1) * sizeof(int));
        TupleList::Node* current = list->head;
        int point_count = 0;
        bool first = true;
        do {
            if(!first) {
                current = current->next;
            }
            if(points.find(current->value.hash) == points.end()) {
                point_count++;
                points.insert(std::pair<MapProcessing::TupleHash, int>(current->value.hash, point_count));
            }
            first = false;
        } while(current->next != nullptr);
        points.clear();
        output->numberofpoints = point_count;
        output->pointlist = (REAL *) malloc(point_count * 2 * sizeof(REAL));
        output->pointmarkerlist = (int *) malloc(point_count * sizeof(int));
        current = list->head;
        first = true;
        int point_index = 0;
        int segment_index = 0;
        do {
            if(!first) {
                current = current->next;
            }
            if(points.find(current->value.hash) == points.end()) {
                output->pointlist[point_index * 2] = current->value.first;
                output->pointlist[point_index * 2 + 1] = current->value.second;
                output->pointmarkerlist[point_index] = 1;
                point_index++;
                points.insert(std::pair<MapProcessing::TupleHash, int>(current->value.hash, point_index));
            }
            first = false;
        } while(current->next != nullptr);
        current = list->head;
        first = true;
        do {
            if(!first) {
                current = current->next;
            }
            output->segmentlist[segment_index * 2] = points[current->value.hash];
            output->segmentlist[segment_index * 2 + 1] = points[current->next->value.hash];
            output->segmentmarkerlist[segment_index] = 1;
            segment_index++;
            first = false;
        } while(current->next != nullptr && current->next->next != nullptr);
        
    }
    void from_list(std::vector<DoubleLinkedList<MapProcessing::Tuple>*> list_of_lists, triangulateio* output) {
        std::map<MapProcessing::TupleHash, int> points = std::map<MapProcessing::TupleHash, int>();
        int num_segments = 0;
        for(const DoubleLinkedList<MapProcessing::Tuple>* list : list_of_lists) {
            num_segments += list->length - 1;
        }
        output->numberofsegments = num_segments;
        output->segmentlist = (int *) malloc(num_segments * 2 * sizeof(int));
        output->segmentmarkerlist = (int *) malloc(num_segments * sizeof(int));

        int point_count = 0;
        for(const DoubleLinkedList<MapProcessing::Tuple>* list : list_of_lists) {
            TupleList::Node* current = list->head;
            bool first = true;
            do {
                if(!first) {
                    current = current->next;
                }
                if(points.find(current->value.hash) == points.end()) {
                    point_count++;
                    points.insert(std::pair<MapProcessing::TupleHash, int>(current->value.hash, point_count));
                }
                first = false;
            } while(current->next != nullptr);
        }
        points.clear();
        output->numberofpoints = point_count;
        output->pointlist = (REAL *) malloc(point_count * 2 * sizeof(REAL));
        output->pointmarkerlist = (int *) malloc(point_count * sizeof(int));
        int point_index = 0;
        for(const DoubleLinkedList<MapProcessing::Tuple>* list : list_of_lists) {
            TupleList::Node* current = list->head;
            bool first = true;
            do {
                if(!first) {
                    current = current->next;
                }
                if(points.find(current->value.hash) == points.end()) {
                    output->pointlist[point_index * 2] = current->value.first;
                    output->pointlist[point_index * 2 + 1] = current->value.second;
                    output->pointmarkerlist[point_index] = 1;
                    point_index++;
                    points.insert(std::pair<MapProcessing::TupleHash, int>(current->value.hash, point_index));
                }
                first = false;
            } while(current->next != nullptr);
        }
        int segment_index = 0;
        for(const DoubleLinkedList<MapProcessing::Tuple>* list : list_of_lists) {
            TupleList::Node* current = list->head;
            bool first = true;
            do {
                if(!first) {
                    current = current->next;
                }
                output->segmentlist[segment_index * 2] = points[current->value.hash];
                output->segmentlist[segment_index * 2 + 1] = points[current->next->value.hash];
                output->segmentmarkerlist[segment_index] = 1;
                segment_index++;
                first = false;
            } while(current->next != nullptr && current->next->next != nullptr);
        }
    }
}