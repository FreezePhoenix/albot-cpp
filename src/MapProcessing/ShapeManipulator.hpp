#pragma once

#ifndef ALBOT_SHAPEMANIPULATOR_HPP_
#define ALBOT_SHAPEMANIPULATOR_HPP_

#include "../Common.hpp"
#include "TriangleManipulator.hpp"
#include "MapProcessing.hpp"
#include "Objectifier.hpp"
#include "../Utils/DoubleLinkedList.hpp"

namespace ShapeManipulator {
    void from_list(std::vector<MapProcessing::Line>* list, triangulateio* output, Objectifier* objectifier);
    void from_list(std::vector<std::vector<MapProcessing::Line>*>& list_of_lists, triangulateio* output, Objectifier* objectifier);
}

#endif /* ALBOT_SHAPEMANIPULATOR_HPP_ */