#include <vector>
#include "Utils/DoubleLinkedList.hpp"

template class std::vector<short>;
template class std::pair<short, short>;
template class DoubleLinkedList<MapProcessing::Tuple>;
template class std::vector<DoubleLinkedList<MapProcessing::Tuple>*>;
template class std::allocator<DoubleLinkedList<MapProcessing::Tuple>*>;
template class std::unordered_map<unsigned int, DoubleLinkedList<MapProcessing::Tuple>::Node*>;