#pragma once

#ifndef ALBOT_WRITER_HPP_
#define ALBOT_WRITER_HPP_

#include "../Common.hpp"

#include "Objectifier.hpp"

class Writer {
    private:
        Objectifier* objectifier;
        std::unordered_map<std::pair<short, short>, int, pair_hash> points;
    public:
        Writer(Objectifier* objectifier);
        void write();
};

#endif /* ALBOT_WRITER_HPP_ */