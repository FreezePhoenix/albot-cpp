#pragma once

#ifndef ALBOT_WRITER_HPP_
#define ALBOT_WRITER_HPP_

#include "MapProcessing.hpp"
#include "../Utils/DoubleLinkedList.hpp"
#include "Objectifier.hpp"
#include <set>

class Writer {
    private:
        Objectifier* objectifier;
        std::map<MapProcessing::TupleHash, int> points;
        typedef DoubleLinkedList<MapProcessing::Tuple> TupleList;
    public:
        Writer(Objectifier* objectifier);
        void write();
};

#endif /* ALBOT_WRITER_HPP_ */