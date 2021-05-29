#pragma once

#ifndef ALBOT_COMMON_HPP_
#define ALBOT_COMMON_HPP_

#include <vector>
#include "Utils/DoubleLinkedList.hpp"

struct pair_hash {
    template <class T1, class T2>
    inline std::size_t operator () (const std::pair<T1,T2> &p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);

        // Mainly for demonstration purposes, i.e. works but is overly simple
        // In the real world, use sth. like boost.hash_combine
        return h1 << sizeof(T2) + h2;
    }
    template <class T1, class T2>
    inline std::size_t operator () (const std::pair<T1,T2> &p, const std::pair<T1,T2> &p2) const {
        return this->operator()(p) - this->operator()(p2);
    }
};

extern template class std::pair<short, short>;
extern template class std::vector<short>;
extern template class DoubleLinkedList<MapProcessing::Tuple>;
extern template class std::vector<DoubleLinkedList<MapProcessing::Tuple>*>;
extern template class std::allocator<DoubleLinkedList<MapProcessing::Tuple>*>;
extern template class std::unordered_map<unsigned int, DoubleLinkedList<MapProcessing::Tuple>::Node*>;

#endif /* ALBOT_COMMON_HPP_ */