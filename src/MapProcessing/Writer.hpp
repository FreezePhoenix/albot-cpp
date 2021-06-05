#pragma once
#pragma interface
#ifndef ALBOT_WRITER_HPP_
#define ALBOT_WRITER_HPP_

#include "Objectifier.hpp"

class Writer {
    private:
        Objectifier& objectifier;
    public:
        Writer(Objectifier& objectifier);
        void write();
};

#endif /* ALBOT_WRITER_HPP_ */