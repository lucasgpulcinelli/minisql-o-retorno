#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__

#include <cinttypes>
#include <vector>
#include <utility>

#include "table.hpp"

template <class Node, class Edge> class Graph {
    private:

    std::vector<std::vector<std::pair<size_t, Edge>>> adjacencies;
    std::vector<Node> node_list;
    size_t num_nodes;

    size_t insertNode(Node new_node);

    public:

    print();

    void insertNewEdge(Node exit_node, Edge new_edge);
    void insertNewNode(Node new_node);

    Graph(const Table& topology);
    ~Graph();

};

#endif