#ifndef __GRAPH_HPP__
#define __GRAPH_HPP__

#include <cinttypes>
#include <vector>
#include <utility>
#include <map>

#ifndef EMPTY_VALUE
    #define EMPTY_VALUE -1
#endif

/*
 * class Node is a generic class for a node in a Graph. 
 * It is effectively an interface that every node class
 * passed to template<Node, Edge> Graph must implement. 
 */
class Node {

private:
    bool is_empty; //whether the node has a valid or invalid address.
    int32_t id;    //node key/address.

public:
    int32_t idKey() const; //idKey returns node adress.
    bool isEmpty() const;  //isEmpty returns whether address is valid.

    Node(int32_t id);      //Node constructs a node with adress id.
    Node();                //Empty constructor initializes an empty node.
};

/*
 * operator< overloads the operator '<' to compare instances of
 * the class Node. Node instances are ordered by their key value (address).
 */
bool operator<(const Node& left_arg, const Node& right_arg);

/*
 * operator== overloads the operator '==' to compare elements of the class
 * Node. Node instances are equal iff their keys are equal.
 */
bool operator==(const Node& left_arg, const Node& right_arg);

/*
 * operator<< overloads the operator '<<' to print elements of Node in
 * streams. It prints the Node address idKey() on ostream os.
 */
std::ostream& operator<<(std::ostream& os, const Node& node);


/*
 * class Edge is a generic class for an edge in a graph.
 * It is effectively an interface that every edge class passed
 * to template<Node, edge> Graph must implement.
 */
class Edge {
private:
    /*
     * Edge instances 'go' from node id_from to no id_to.
     * Therefore, this interface supports edges for directed graphs.
     */
    int32_t id_from; //key/address from the node in the first end of the edge.
    int32_t id_to;   //address from the node in the second end of the edge.

public:
    int32_t idFrom() const; //idFrom returns from which node edge is coming.
    int32_t idTo() const;   //idTo returns to which node edge is going.

    void reverse();         //reverse swaps id_from and id_to values.

    Edge operator=(const Edge& right_arg); //Copy assignment
    /*
     * Edge(int id_from, int id_to) constructs an edge that comes from
     * id_from and goes to id_to.
     */
    Edge(int32_t id_from, int32_t id_to);
    Edge(const Edge& edge);                //Copy constructor
    Edge();                                //Constructs empty edge.
};

/*
 * operator== overloads the operator '==' to compare Edge instances.
 * Edges are equal if they come from the same node and go to the same node.
 */
bool operator==(const Edge& left_arg, const Edge& right_arg);

/*
 * operator< overloads the operator '<' to compare Edge instances.
 * Edges are ordered by the address of the Node to which they are going.
 */
bool operator<(const Edge& left_arg, const Edge& right_arg);

/*
 *operator<< overloads the operator '<' to print Edge edge in 
 * ostream os. It only prints idTo, because it is suited for the
 * implementation of operator<<(ostream, Graph).
 */
std::ostream& operator<<(std::ostream& os, const Edge& edge);

template<class Node, class Edge>
class Graph;

/*
 * operator<< overloads the operator '<<' to print Graphs in ostreams.
 * For each node Ni and each edge Ej adjacent to it, it prints the node
 * followed by the edge, then goes to next line in ostream os.
 */
template<class Node, class Edge>
std::ostream& operator<<(std::ostream& os, const Graph<Node, Edge>& g);

/*
 * template<Node, Edge> Graph is a generic class for a non-directed graph.
 * In order to instantiate a working concrete class from the template,
 * 'class Node' must implement interface Node and 'class Edge' must 
 * implement interface 'Edge' (i.e., they must publicly inherit from
 * 'Node' and 'Edge', respectively).
 */
template<class Node, class Edge>
class Graph {
    //operator<< must access protected methods.
    friend std::ostream& operator<<<Node, Edge>(std::ostream& os, 
                                                const Graph<Node, Edge>& g);
private:
    /*
     * insertAdjacency inserts the Edge instance 'new_edge' in the adjancency list
     * of the Node with address new_edge.idFrom(). If the node does not exist, it
     * inserts an empty node, i.e., inserts an adjacency list with key idFrom(),
     * but does not insert a new node in the map of nodes.
     * 
     * Adjancency lists are kept ordered by the order relation 
     * operator<(Edge, Edge). The insertion algorithm is essentially a binary
     * search in the adjacency list, which inserts the Edge if no equivalent edge
     * is found (equivalence as defined by operator==(Edge, Edge)), but does 
     * nothing otherwise.
     */
    void insertAdjacency(const Edge& new_edge);

protected:
    /*
     * adjacencies is a map that takes the address/key of the node (as given by
     * .idKey()), and returns its adjacency list - the ordered vector of edges
     * adjacent to that node.
     */
    std::map<int32_t, std::vector<Edge>> adjacencies;
    /*
     * node_list is a map that takes the address/key of the node (as given by
     * .idKey()) and returns the node to which it corresponds.
     */
    std::map<int32_t, Node> node_list;

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
    /*
     * insertNode inserts a new_node in the graph. If an equivalent node
     * already exists (equivalence as defined in operator==(Node, Node)), 
     * it only inserts new_node if this node is empty. If the node is empty,
     * i.e., its address is invalid, it throws a runtime_error() exception.
     */
    void insertNode(const Node& new_node);
    /*
     * insertEdge inserts a new_edge in graph. If an equivalent edge already
     * exists (equivalence as defined in operator==(Edge, Edge)), it does not
     * insert new_edge. If the edge has an invalid idFrom() or idTo(), it
     * throws a runtime_error() exception. 
     * 
     * If new_edge is connected to a node that was not yet inserted in the
     * graph, insertEdge inserts an empty node mapped to the respective address.
     * 
     * insertEdge leaves new_edge unchanged when it returns, but it changes
     * the state of new_edge during execution by calling .reverse(), so it
     * is NOT thread-safe. 
     */
    void insertEdge(Edge& new_edge);

    /*
     * getNumCicles calculates all cicles in a graph. This is done by calling,
     * for every node in the graph, the private overload of getNumCicles for
     * every connection, and subtracting the connections that span only two 
     * nodes.
     */
    int32_t getNumCicles(void);

    /*
     * getLen calculates the minimum distance between nodes a and b.
     * The method initializes a map to keep track of recursions and calls the
     * private version of getLen().
     */
    int32_t getLen(int32_t node_a_id, int32_t node_b_id);
};

#endif