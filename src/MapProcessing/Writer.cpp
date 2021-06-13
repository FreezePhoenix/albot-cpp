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
    std::shared_ptr<MapProcessing::MapInfo> info = this->objectifier.info;
    std::shared_ptr<triangulateio> input = TriangleManipulator::create_instance();
    std::shared_ptr<triangulateio> output = TriangleManipulator::create_instance();
    std::shared_ptr<triangulateio> voutput = TriangleManipulator::create_instance();
    int index = 0;
    int num_holes = 0;
    std::vector<std::pair<REAL, REAL>> holes = std::vector<std::pair<REAL, REAL>>();
    
    ShapeManipulator::from_list(this->objectifier.objects, input);

    if(input->numberofsegments < 3) {
        std::cout << "Problem: " << info->name << std::endl << std::endl;
        return;
    }

    bool first = true;
    for(std::shared_ptr<std::vector<MapProcessing::Line>> obj : this->objectifier.objects) {
        if(first) {
            first = false;
            std::shared_ptr<triangulateio> objecto = TriangleManipulator::create_instance();
            ShapeManipulator::from_list(*obj, objecto);
            objecto->numberofholes = info->spawns.size();;
            objecto->holelist = (REAL *) malloc(objecto->numberofholes * 2 * sizeof(REAL));
            for(int i = 0; i < info->spawns.size(); i++) {
                objecto->holelist[i * 2] = info->spawns[i].first;
                objecto->holelist[i * 2 + 1] = info->spawns[i].second;
            }
            // TriangleManipulator::write_poly_file("Maps/" + info->name + ".object." + std::to_string(++index) + ".poly", objecto);
            std::shared_ptr<triangulateio> output = TriangleManipulator::create_instance();
            triangulate("pzDQ", objecto.get(), output.get(), nullptr);
            std::vector<std::pair<REAL, REAL>> shouldbe_holes = ShapeManipulator::find_points_inside(output);
            num_holes += shouldbe_holes.size();
            holes.insert(holes.end(), shouldbe_holes.begin(), shouldbe_holes.end());
            // Little bit of manual cleanup.
            objecto->holelist = (double *) NULL;
        } else {
            std::shared_ptr<triangulateio> objecto = TriangleManipulator::create_instance();
            ShapeManipulator::from_list(*obj, objecto);
            std::vector<std::pair<REAL, REAL>> new_holes = ShapeManipulator::find_points_inside(*obj, objecto);

            for(const std::pair<REAL, REAL>& hole : new_holes) {
                holes.push_back(hole);
                num_holes++;
            }
        }
    }
    std::cout << info->name << std::endl;
    input->numberofholes = num_holes;
    input->holelist = (REAL *) malloc(num_holes * 2 * sizeof(REAL));
    int hole_index = 0;
    for(std::pair<REAL, REAL> hole : holes) {
        input->holelist[hole_index * 2] = hole.first;
        input->holelist[hole_index * 2 + 1] = hole.second;
        hole_index++;
    }
    triangulate("pznvejDQq30", input.get(), output.get(), voutput.get());
    std::shared_ptr<triangulateio> temp = TriangleManipulator::create_instance();
    TriangleManipulator::filter_edges(voutput.get(), temp.get(), [](int p1, int p2, REAL norm1, REAL norm2) {
        return p2 != -1;
    });
    TriangleManipulator::write_poly_file("Maps/" + info->name + ".poly", input.get());
    TriangleManipulator::write_edge_file("Maps/" + info->name + ".v.edge", temp.get());
    TriangleManipulator::write_node_file("Maps/" + info->name + ".v.node", voutput.get());
    TriangleManipulator::write_node_file("Maps/" + info->name + ".node", output.get());
    TriangleManipulator::write_ele_file("Maps/" + info->name + ".ele", output.get());
    TriangleManipulator::write_neigh_file("Maps/" + info->name + ".neigh", output.get());
    TriangleManipulator::write_edge_file("Maps/" + info->name + ".edge", output.get());
    // Little bit of manual cleanup.
    input->holelist = (double *) NULL;
}