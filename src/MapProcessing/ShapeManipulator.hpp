#include "TriangleManipulator.hpp"
#include "MapProcessing.hpp"
#include "../Utils/DoubleLinkedList.hpp"
namespace ShapeManipulator {
    typedef DoubleLinkedList<MapProcessing::Tuple> TupleList;
    void from_list(DoubleLinkedList<MapProcessing::Tuple>* list, triangulateio* output);
    void from_list(std::vector<DoubleLinkedList<MapProcessing::Tuple>*> list_of_lists, triangulateio* output);
}