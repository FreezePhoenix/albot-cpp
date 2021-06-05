#pragma once

#ifndef ALBOT_SHAPEMANIPULATOR_HPP_
#define ALBOT_SHAPEMANIPULATOR_HPP_

#include "TriangleManipulator.hpp"
#include "MapProcessing.hpp"
#include "Objectifier.hpp"

namespace ShapeManipulator {
    void from_list(const std::vector<MapProcessing::Line>& list, triangulateio* output);
    void from_list(std::vector<MapProcessing::Line>* list, triangulateio* output);
    void from_list(const std::vector<std::vector<MapProcessing::Line>*>& list_of_lists, triangulateio* output);
    std::vector<std::pair<REAL, REAL>> find_points_inside(const std::vector<MapProcessing::Line>& object, triangulateio* output);
    std::vector<std::pair<REAL, REAL>> find_points_inside(triangulateio* input);
}

#endif /* ALBOT_SHAPEMANIPULATOR_HPP_ */