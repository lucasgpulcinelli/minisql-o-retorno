#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__

#include <cinttypes>
#include <vector>
#include <utility>
#include <map>

#ifndef EMPTY_VALUE
    #define EMPTY_VALUE -1
#endif

class Node {

private:
    bool is_empty;
    int32_t id;

public:
    int32_t idKey() const;
    bool isEmpty() const;

    Node(int32_t id);
    Node();
};

bool operator<(const Node& left_arg, const Node& right_arg);

bool operator==(const Node& left_arg, const Node& right_arg);

std::ostream& operator<<(std::ostream& os, const Node& node);


class Edge {
private:
    int32_t id_from;
    int32_t id_to;

public:
    int32_t idFrom() const;
    int32_t idTo() const;

    void reverse();
    Edge operator=(const Edge& right_arg);

    Edge(int32_t id_from, int32_t id_to);
    Edge(const Edge& edge);
    Edge();
};

bool operator==(const Edge& left_arg, const Edge& right_arg);

bool operator<(const Edge& left_arg, const Edge& right_arg);

template<class Node, class Edge>
class Graph;

template<class Node, class Edge>
std::ostream& operator<<(std::ostream& os, const Graph<Node, Edge>& g);

template<class Node, class Edge>
class Graph {
    friend std::ostream& operator<<<Node, Edge>(std::ostream& os, 
                                                const Graph<Node, Edge>& g);

private:
    std::map<int32_t, std::vector<Edge>> adjacencies;
    std::map<int32_t, Node> node_list;

    void insertAdjacency(const Edge& new_edge);

public:
    void insertNode(const Node& new_node);
    void insertEdge(Edge& new_edge);
};

#endif