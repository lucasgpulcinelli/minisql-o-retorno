#ifndef __NODE_HPP__
#define __NODE_HPP__

#include <cinttypes>
#include <string>
#include <ostream>
#include <map>

#include "Graph.hxx"
#include "table.hpp"

extern "C" {
#include "entries.h"
}

#define ACRONYM_SIZE 2
#define CONVERSION_FACTOR 1024

/*
 * class NetworkNode implements interface Node. It encapsulates the nodes of
 * a computer network, i.e., the computers and points of presence in this 
 * network.
 */
class NetworkNode : public Node {
    //operator<< needs to access NetworkNode's private atributes to print them.
    friend std::ostream& operator<<(std::ostream& os, const NetworkNode& node);

    private:
    //Acronym of the country in which it's based.
    char countryAcronym[ACRONYM_SIZE];
    std::string POPsName;              //Point of Presence's name
    std::string originCountryName;     //Country in which it's based.

    public:
    NetworkNode(entry* es);            //Contructs Network Node from entry es.
    NetworkNode(){};                   //Constructs empty NetworkNode
};

/*
 * operator<< overloads operator '<<' to print instance of NetworkNode 'node'
 * in ostream 'os'. It prints the address of the node, followed by its POP's
 * name, followed by its country of origin's name, and finally by its country
 * of origin's acronym.
 */
std::ostream& operator<<(std::ostream& os, const NetworkNode& node);

/*
 * class Connection implements interface Edge. It is a class that encapsulates
 * connections between computers in a computer network (with NetworkNodes as
 * the ends of these connections).
 */
class Connection : public Edge {
    //operator<< needs to access private atributes in order to print them.
    friend std::ostream& operator<<(std::ostream& os, const Connection& conn);
    
    private:
    double connectionSpeed; //connection speed between two network nodes.

    public:
    double getSpeed();
    Connection(entry* es);  //Contructs a Connection instance from entry es.
    Connection();           //Contructs empty Connection instance.
};

/*
 * operator<< overloads the operator '<<' to print the connection instance
 * conn in the ostream os. It prints the NetworkNode address idTo() 
 * followed by the connection speed in megabytes per second.
 */
std::ostream& operator<<(std::ostream& os, const Connection& conn);

/*
 * class NetworkGraph extends Graph<NetworkNode, Connection>. It is a
 * class that models computer network topology as a non-directed graph.
 */
class NetworkGraph : public Graph<NetworkNode, Connection> {
    //operator<< must access protected methods in order to print them.
    friend std::ostream& operator<<(std::ostream& os, 
                                    const NetworkGraph& graph);

    private:
    /*
     * getLen uses a map of marked nodes in the current path, maximim plausable
     * len (a result we already have) and a minimum possible len (the current 
     * lenght in the current path up to now) to calculate the minimum distance
     * between a starting node and an ending node. This algorithm is made by
     * the creators of the program, and is based on a similar method used in
     * the discrete version of the simplex method: the branch-and-bound
     * algorithm.
     */
    double getLen(std::map<int32_t, bool>& marks, int32_t node_start_id,
                   int32_t node_end_id, double max_plausable_len, 
                   double min_possible_len);

    public:
    /*
     * Constructs a NetworkGraph instance from table, which stores
     * a network topology that can be modeled as a non-directed graph.
     */
    NetworkGraph(const Table& table);

    double getMaxSpeed(int32_t node_a_id, int32_t node_b_id);

    /*
     * getLen calculates the minimum distance between nodes a and b.
     * The method initializes a map to keep track of recursions and calls the
     * private version of getLen().
     */
    double getLen(int32_t node_a_id, int32_t node_b_id);
};

/*
 * operator<< overloads the operator '<<' to print NetworkGraphs in ostreams.
 * For each NetworkNode Ni and each Connection Ej adjacent to it, it prints 
 * the Network followed by the edge, then goes to the next line.
 */
std::ostream& operator<<(std::ostream& os, const NetworkGraph& graph);

#endif
