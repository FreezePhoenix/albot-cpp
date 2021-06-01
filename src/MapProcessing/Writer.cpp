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
    triangulateio* input = TriangleManipulator::create_instance();
    triangulateio* output = TriangleManipulator::create_instance();
    triangulateio* voutput = TriangleManipulator::create_instance();
    std::cout << info->name << "(" << this->objectifier->objects.size() << ")" << std::endl;
    int index = 0;
    int num_holes = 0;
    std::vector<std::pair<REAL, REAL>> holes = std::vector<std::pair<REAL, REAL>>();
    ShapeManipulator::from_list(this->objectifier->objects, input, this->objectifier);
    if(input->numberofsegments < 3) {
        std::cout << "Problem: " << info->name << std::endl;
        return;
    }
    bool first = true;
    for(std::vector<MapProcessing::Line>* obj : this->objectifier->objects) {
        if(first) {
            first = false;
            continue;
        }
        triangulateio* _input = TriangleManipulator::create_instance();
        triangulateio* _output = TriangleManipulator::create_instance();
        triangulateio* _voutput = TriangleManipulator::create_instance();
        ShapeManipulator::from_list(obj, _input, this->objectifier);
        triangulate("pvDQPE", _input, _output, _voutput);
        REAL hole_x = _voutput->pointlist[0];
        REAL hole_y = _voutput->pointlist[1];
        num_holes++;
        holes.push_back(std::pair<REAL, REAL>(hole_x, hole_y));
    }
    input->numberofholes = num_holes;
    input->holelist = (REAL *) malloc(num_holes * 2 * sizeof(REAL));
    int hole_index = 0;
    for(std::pair<REAL, REAL> hole : holes) {
        input->holelist[hole_index * 2] = hole.first;
        input->holelist[hole_index * 2 + 1] = hole.second;
        hole_index++;
    }
    triangulate("pvDQqa1000", input, output, voutput);
    triangulateio* temp = TriangleManipulator::create_instance();
    TriangleManipulator::filter_edges(voutput, temp, [](int p1, int p2, REAL norm1, REAL norm2) {
        return p2 != -1;
    });
    TriangleManipulator::write_poly_file("Maps/" + info->name + ".poly", output);
    TriangleManipulator::write_edge_file("Maps/" + info->name + ".v.edge", temp);
    TriangleManipulator::write_node_file("Maps/" + info->name + ".v.node", voutput);

    // TriangleManipulator::cleanup(trian); 
    // TriangleManipulator::cleanup(outstuff);
    // TriangleManipulator::cleanup(voutstuff);
}