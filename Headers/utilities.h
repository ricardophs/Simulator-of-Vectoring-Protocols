#ifndef UTILITIES_H
#define UTILITIES_H

#include <unordered_map>
#include <vector>

#include "nodeOneDest.h"

template <class A>
std::vector<A> EvDijkstra(std::vector<NodeOneDest<A>> &, int, A, bool);

template <class A>
std::vector<NodeOneDest<A>> createGraph(std::string,
                           std::vector<std::unordered_map<int, int>> &,
                           std::vector<std::unordered_map<int, int>> &,
                           int,
                           bool=false);

template <class A> bool isConnected(std::vector<NodeOneDest<A>> &);


#endif