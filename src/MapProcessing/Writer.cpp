#include "TriangleManipulator.hpp"
#include "ShapeManipulator.hpp"
#include "MapProcessing.hpp"
#include <iostream>
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
    int num_holes = 0;
    std::shared_ptr<std::vector<double>> holes = std::shared_ptr<std::vector<double>>(new std::vector<double>());
    
    ShapeManipulator::from_list(this->objectifier.objects, input);
    std::cout << info->name << std::endl;
    if (input->numberofsegments < 3) {
        std::cout << "Problem: " << info->name << std::endl << std::endl;
        return;
    }

    bool first = true;
    int index = 0;
    for (std::shared_ptr<std::vector<MapProcessing::Line>> obj : this->objectifier.objects) {
        if (first) {
            first = false;
            std::shared_ptr<triangulateio> objecto = TriangleManipulator::create_instance();
            ShapeManipulator::from_list(*obj, objecto);
            objecto->numberofholes = info->spawns.size();;
            objecto->holelist = trimalloc<REAL>(objecto->numberofholes * 2);
            const std::size_t max = info->spawns.size();
            const std::pair<double, double>* ptr = info->spawns.data();
            REAL* hole_ptr = objecto->holelist.get();
            for (std::size_t i = 0; i < max; i++) {
                hole_ptr[i * 2] = ptr[i].first;
                hole_ptr[i * 2 + 1] = ptr[i].second;
            }
            std::shared_ptr<triangulateio> output = TriangleManipulator::create_instance();
            triangulate("pzDQ", objecto, output, nullptr);
            output->numberofholes = 0;
            std::shared_ptr<std::vector<double>> new_holes = ShapeManipulator::find_points_inside(output);
            TriangleManipulator::write_poly_file("Maps/" + info->name + ".object." + std::to_string(++index) + ".poly", objecto);
            num_holes += new_holes->size() / 2;
            holes->insert(holes->end(), new_holes->begin(), new_holes->end());
            // Little bit of manual cleanup.
        } else {
            std::shared_ptr<triangulateio> triangle_object = TriangleManipulator::create_instance();
            ShapeManipulator::from_list(*obj, triangle_object);
            std::shared_ptr<std::vector<double>> new_holes = ShapeManipulator::find_points_inside(*obj);
            // triangle_object->holelist = std::shared_ptr<double>(new_holes->data(), [](void*) {});
            // triangle_object->numberofholes = new_holes->size() / 2;
            TriangleManipulator::write_poly_file("Maps/" + info->name + ".object." + std::to_string(++index) + ".poly", triangle_object);
            num_holes += new_holes->size() / 2;
            holes->insert(holes->end(), new_holes->cbegin(), new_holes->cend());
        }
    }
    input->numberofholes = num_holes;
    input->holelist = std::shared_ptr<double>(holes->data(), [](void*) {});
    triangulate("pznvejDQq30", input, output, voutput);
    std::shared_ptr<triangulateio> temp = TriangleManipulator::create_instance();
    TriangleManipulator::filter_edges(voutput, temp, [](int p1, int p2, REAL norm1, REAL norm2) {
        return p2 != -1;
    });
    TriangleManipulator::write_poly_file("Maps/" + info->name + ".poly", input);
    TriangleManipulator::write_edge_file("Maps/" + info->name + ".v.edge", temp);
    TriangleManipulator::write_node_file("Maps/" + info->name + ".v.node", voutput);
    TriangleManipulator::write_node_file("Maps/" + info->name + ".node", input);
    TriangleManipulator::write_ele_file("Maps/" + info->name + ".ele", output);
    TriangleManipulator::write_neigh_file("Maps/" + info->name + ".neigh", output);
    TriangleManipulator::write_edge_file("Maps/" + info->name + ".edge", output);
    // Little bit of manual cleanup.
}