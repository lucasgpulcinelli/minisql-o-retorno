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

protected:
    std::map<int32_t, std::vector<Edge>> adjacencies;
    std::map<int32_t, Node> node_list;

    void insertAdjacency(const Edge& new_edge);

    /*
     * getNumCicles uses recursion for every edge of node_id in order to find 
     * all cicles with increasing indicies numbers (except for the last node to
     * node_start) that start at node_start. The increasing indices order is
     * done to have no duplicates in the final count.
     */
    int32_t getNumCicles(Node& node_start, int32_t node_id);

public:
    void insertNode(const Node& new_node);
    void insertEdge(Edge& new_edge);

    /*
     * getNumCicles calculates all cicles in a graph. This is done by calling,
     * for every node in the graph, the private overload of getNumCicles for
     * every connection, and subtracting the connections that span only two 
     * nodes.
     */
    int32_t getNumCicles(void);
};

#endif