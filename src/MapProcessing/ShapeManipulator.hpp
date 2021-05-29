#pragma once

#ifndef ALBOT_SHAPEMANIPULATOR_HPP_
#define ALBOT_SHAPEMANIPULATOR_HPP_

#include "../Common.hpp"
#include "TriangleManipulator.hpp"
#include "MapProcessing.hpp"
#include "../Utils/DoubleLinkedList.hpp"

namespace ShapeManipulator {
    void from_list(DoubleLinkedList<std::pair<short, short>>* list, triangulateio* output);
    void from_list(const std::vector<DoubleLinkedList<std::pair<short, short>>*>& list_of_lists, triangulateio* output);
}

#endif /* ALBOT_SHAPEMANIPULATOR_HPP_ */