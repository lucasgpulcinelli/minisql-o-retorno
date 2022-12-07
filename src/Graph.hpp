#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__

#include <cinttypes>
#include <vector>
#include <utility>

template<typename T>
class Node {
public:
    Node(int id, T t);
    int id;
    T t;
};

template<typename T>
bool operator<(const Node<T>& left_arg, const Node<T>& right_arg);

template<typename T>
bool operator==(const Node<T>& left_arg, const Node<T>& right_arg);

template<typename T>
std::ostream& operator<<(std::ostream& os, Node<T>& node);


template<typename T>
class Edge {
public:
    Edge(int id_from, int id_to, T t);
    int id_from;
    int id_to;
    T t;
};

template<typename T>
bool operator==(const Edge<T>& left_arg, const Edge<T>& right_arg);

template<typename T1, typename T2>
class Graph;

template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, Graph<T1, T2>& g);

template<typename T1, typename T2>
class Graph {
    friend std::ostream& operator<<<T1, T2>(std::ostream& os, Graph<T1, T2>& g);

private:
    std::vector<std::vector<Edge<T2>>> adjacencies;
    std::vector<Node<T1>> node_list;
    size_t num_nodes = 0;

public:
    size_t insertNode(Node<T1> new_node);
    void insertEdge(Edge<T2> new_edge);

    ~Graph();
};

#endif