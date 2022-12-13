#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__

#include <cinttypes>
#include <vector>
#include <utility>
#include <ostream>
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
    int32_t c_speed;

    int32_t idFrom() const;
    int32_t idTo() const;

    Edge getDual() const;
    Edge operator=(const Edge& right_arg);

    Edge(int32_t id_from, int32_t id_to, int32_t speed);
    Edge(const Edge& edge);
    Edge();
};

bool operator==(const Edge& left_arg, const Edge& right_arg);

bool operator<(const Edge& left_arg, const Edge& right_arg);

template<class Node, class Edge>
class Graph;

template<class Node, class Edge>
std::ostream& operator<<(std::ostream& os, const Graph<Node, Edge>& g);

template <typename Type>
void swap(Type& arg_1, Type& arg_2);

template<class Node, class Edge>
class Graph {
    friend std::ostream& operator<<<Node, Edge>(std::ostream& os, 
                                                const Graph<Node, Edge>& g);

private:
    std::map<int32_t, std::vector<Edge>> adjacencies;
    std::map<int32_t, Node> node_list;

    void insertAdjacency(const Edge& new_edge, int32_t node);

    /*
     * getNumCicles uses recursion for every edge of node_id in order to find 
     * all cicles with increasing indicies numbers (except for the last node to
     * node_start) that start at node_start. The increasing indices order is
     * done to have no duplicates in the final count.
     */
    int32_t getNumCicles(Node& node_start, int32_t node_id);

    /*
     * getLen uses a map of marked nodes in the current path, maximim plausable
     * len (a result we already have) and a minimum possible len (the current 
     * lenght in the current path up to now) to calculate the minimum distance
     * between a starting node and an ending node. This algorithm is made by
     * the creators of the program, and is based on a similar method used in
     * the discrete version of the simplex method: the branch-and-bound
     * algorithm.
     */
    int32_t getLen(std::map<int32_t, bool>& marks, int32_t node_start_id,
                   int32_t node_end_id, int32_t max_plausable_len, 
                   int32_t min_possible_len);

public:
    void insertNode(const Node& new_node);
    void insertEdge(const Edge& new_edge);

    /*
     * getNumCicles calculates all cicles in a graph. This is done by calling,
     * for every node in the graph, the private overload of getNumCicles for
     * every connection, and subtracting the connections that span only two 
     * nodes.
     */
    int32_t getNumCicles(void);

    int32_t getMaxSpeed(int32_t node_a_id, int32_t node_b_id);

    /*
     * getLen calculates the minimum distance between nodes a and b.
     * The method initializes a map to keep track of recursions and calls the
     * private version of getLen().
     */
    int32_t getLen(int32_t node_a_id, int32_t node_b_id);
};

#endif