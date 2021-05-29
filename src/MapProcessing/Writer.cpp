#include <vector>


#include "TriangleManipulator.hpp"
#include "ShapeManipulator.hpp"
#include "MapProcessing.hpp"
#include "Writer.hpp"

Writer::Writer(Objectifier* objectifier) {
    this->objectifier = objectifier;
    this->points = std::unordered_map<std::pair<short, short>, int, pair_hash>();
}

std::string pad_right(std::string const& str, size_t s) {
    if ( str.size() < s ) {
        return str + std::string(s-str.size(), ' ');
    } else {
        return str;
    };
}

void Writer::write() {
    MapProcessing::MapInfo* info = this->objectifier->info;
    triangulateio* output = TriangleManipulator::create_instance();
    ShapeManipulator::from_list(this->objectifier->objects, output);
    TriangleManipulator::write_poly_file("Maps/" + info->name + ".poly", output);
    int index = 0;
    for(const DoubleLinkedList<std::pair<short, short>>* list : this->objectifier->objects) {
        if(list == (*this->objectifier->objects.begin())) {
            continue;
        }
        triangulateio* triangle = TriangleManipulator::create_instance();
        ShapeManipulator::from_list((DoubleLinkedList<std::pair<short, short>>*) list, triangle);
        TriangleManipulator::write_poly_file("Maps/" + info->name + ".object" + std::to_string(index++) + ".poly", triangle);
        TriangleManipulator::cleanup(triangle);
    }
    if(output->numberofpoints < 3) {
        return;
    }
    // triangulateio* trian = output;

    // triangulateio* outstuff = TriangleManipulator::create_instance();
    // triangulateio* voutstuff = TriangleManipulator::create_instance();
    // triangulate("pvDQa1000", trian, outstuff, voutstuff);

    // voutstuff->pointlist = holeoutstuff->pointlist;
    // voutstuff->numberofpoints = holeoutstuff->numberofpoints;
    // voutstuff->numberofpointattributes = holeoutstuff->numberofpointattributes;
    // voutstuff->pointmarkerlist = holeoutstuff->pointmarkerlist;
    // voutstuff->pointattributelist = holeoutstuff->pointattributelist;
    // TriangleManipulator::filter_edges(voutstuff, outstuff, [](int p1, int p2, REAL norm1, REAL norm2) {
    //     return p2 != -1;
    // });
    triangulateio* map = TriangleManipulator::create_instance();
    ShapeManipulator::from_list(*objectifier->objects.begin(), map);
    TriangleManipulator::write_poly_file("Maps/" + info->name + ".poly", map);
    // TriangleManipulator::cleanup(trian);
    // TriangleManipulator::cleanup(outstuff);
    // TriangleManipulator::cleanup(voutstuff);
}