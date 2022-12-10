#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__

#include <cinttypes>
#include <vector>
#include <utility>
#include <map>

template<typename NodeMetadata>
class Node {
public:
    Node(int id, NodeMetadata t);
    int32_t id;
    NodeMetadata t;
};

template<typename NodeMetadata>
bool operator<(const Node<NodeMetadata>& left_arg, 
               const Node<NodeMetadata>& right_arg);

template<typename NodeMetadata>
bool operator==(const Node<NodeMetadata>& left_arg, 
                const Node<NodeMetadata>& right_arg);

template<typename NodeMetadata>
std::ostream& operator<<(std::ostream& os, Node<NodeMetadata>& node);


template<typename EdgeMetadata>
class Edge {
public:
    Edge(int id_from, int id_to, EdgeMetadata t);
    int32_t id_from;
    int32_t id_to;
    EdgeMetadata t;
};

template<typename EdgeMetadata>
bool operator==(const Edge<EdgeMetadata>& left_arg, 
                const Edge<EdgeMetadata>& right_arg);

template<typename NodeMetadata, typename EdgeMetadata>
class Graph;

template<typename NodeMetadata, typename EdgeMetadata>
std::ostream& operator<<(std::ostream& os, Graph<NodeMetadata, EdgeMetadata>& g);

template<typename NodeMetadata, typename EdgeMetadata>
class Graph {
    friend std::ostream& operator<<<NodeMetadata, EdgeMetadata>
        (std::ostream& os, Graph<NodeMetadata, EdgeMetadata>& g);

private:
    std::map<int32_t, std::vector<Edge<EdgeMetadata>>> adjacencies;
    std::map<int32_t, Node<NodeMetadata>> node_list;

public:
    size_t insertNode(Node<NodeMetadata> new_node);
    void insertEdge(Edge<EdgeMetadata> new_edge);

    ~Graph();
};

#endif