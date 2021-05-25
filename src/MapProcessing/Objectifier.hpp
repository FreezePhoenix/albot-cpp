#pragma once

#ifndef ALBOT_OBJECTIFIER_HPP_
#define ALBOT_OBJECTIFIER_HPP_

#include "MapProcessing.hpp"
#include "../Utils/DoubleLinkedList.hpp"
#include <set>

class Objectifier {
    typedef DoubleLinkedList<MapProcessing::Tuple> TupleList;
    public:
        std::set<TupleList*> objects;
        std::map<MapProcessing::TupleHash, TupleList::Node*> tuple_to_node;
        MapProcessing::MapInfo* info;
        Objectifier(MapProcessing::MapInfo* info);
        void chain(MapProcessing::Tuple first, MapProcessing::Tuple second);
        void run();
};

#endif /* ALBOT_OBJECTIFIER_HPP_ */