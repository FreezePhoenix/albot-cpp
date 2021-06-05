#include "TriangleManipulator.hpp"
#include "ShapeManipulator.hpp"
#include "MapProcessing.hpp"
#include "Writer.hpp"

Writer::Writer(Objectifier& objectifier): objectifier(objectifier) {
}

std::string pad_right(std::string const& str, size_t s) {
    if ( str.size() < s ) {
        return str + std::string(s-str.size(), ' ');
    } else {
        return str;
    };
}

void Writer::write() {
    MapProcessing::MapInfo* info = this->objectifier.info;
    triangulateio* input = TriangleManipulator::create_instance();
    triangulateio* output = TriangleManipulator::create_instance();
    triangulateio* voutput = TriangleManipulator::create_instance();
    int index = 0;
    int num_holes = 0;
    std::vector<std::pair<REAL, REAL>> holes = std::vector<std::pair<REAL, REAL>>();
    
    ShapeManipulator::from_list(this->objectifier.objects, input);

    if(input->numberofsegments < 3) {
        std::cout << "Problem: " << info->name << std::endl << std::endl;
        return;
    }
    bool first = true;
    for(std::vector<MapProcessing::Line>* obj : this->objectifier.objects) {
        if(first) {
            first = false;
            triangulateio* objecto = TriangleManipulator::create_instance();
            ShapeManipulator::from_list(*obj, objecto);
            objecto->numberofholes = 1;
            objecto->holelist = (REAL *) malloc(objecto->numberofholes * 2 * sizeof(REAL));
            objecto->holelist[0] = objecto->holelist[1] = 0;
            TriangleManipulator::write_poly_file("Maps/" + info->name + ".object." + std::to_string(++index) + ".poly", objecto);
            triangulateio* output = TriangleManipulator::create_instance();
            triangulate("pDQ", objecto, output, nullptr);
            std::vector<std::pair<REAL, REAL>> shouldbe_holes = ShapeManipulator::find_points_inside(output);
            num_holes += shouldbe_holes.size();
            holes.insert(holes.end(), shouldbe_holes.begin(), shouldbe_holes.end());
        } else {
            triangulateio* objecto = TriangleManipulator::create_instance();
            ShapeManipulator::from_list(*obj, objecto);
            std::vector<std::pair<REAL, REAL>> new_holes = ShapeManipulator::find_points_inside(*obj, objecto);

            for(const std::pair<REAL, REAL>& hole : new_holes) {
                holes.push_back(hole);
                num_holes++;
            }
        }
    }
    input->numberofholes = num_holes;
    input->holelist = (REAL *) malloc(num_holes * 2 * sizeof(REAL));
    int hole_index = 0;
    for(std::pair<REAL, REAL> hole : holes) {
        input->holelist[hole_index * 2] = hole.first;
        input->holelist[hole_index * 2 + 1] = hole.second;
        hole_index++;
    }
    triangulate("pvjDQq30", input, output, voutput);
    triangulateio* temp = TriangleManipulator::create_instance();
    TriangleManipulator::filter_edges(voutput, temp, [](int p1, int p2, REAL norm1, REAL norm2) {
        return p2 != -1;
    });
    TriangleManipulator::write_poly_file("Maps/" + info->name + ".poly", output);
    TriangleManipulator::write_edge_file("Maps/" + info->name + ".v.edge", temp);
    TriangleManipulator::write_node_file("Maps/" + info->name + ".v.node", voutput);
    TriangleManipulator::write_node_file("Maps/" + info->name + ".node", output);
    TriangleManipulator::write_ele_file("Maps/" + info->name + ".ele", output);

    // TriangleManipulator::cleanup(trian); 
    // TriangleManipulator::cleanup(outstuff);
    // TriangleManipulator::cleanup(voutstuff);
}