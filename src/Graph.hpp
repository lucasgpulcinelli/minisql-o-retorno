#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__

#include <cinttypes>
#include <vector>
#include <utility>


template <class Node, class Edge> class Graph {
    private:
    std::vector<std::vector<std::pair<size_t, Edge>>> adjacencies;
    std::vector<Node> node_list;
    size_t num_nodes;

    public:

    print();

    Graph();
    ~Graph();

};

#endif