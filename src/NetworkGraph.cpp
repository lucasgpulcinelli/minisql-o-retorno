#include <limits>
#include <ostream>
#include <stdexcept>
#include <string>

#include "Graph.hxx"
#include "NetworkGraph.hpp"
#include "table.hpp"

extern "C" {
#include "entries.h"
}

std::ostream& operator<<(std::ostream& os, const NetworkNode& node) {
    return os << node.idKey() << " " << node.POPsName << " "
              << node.originCountryName << " " << node.countryAcronym[0]
              << node.countryAcronym[1];
}

NetworkNode::NetworkNode(entry* es)
    : Node(GET_IDCONNECT(es)) {
    if (GET_IDCONNECT(es) == EMPTY_VALUE) {
        throw std::runtime_error(
            "Cannot construct NetworkNode from empty entry");
    }

    POPsName = std::string(GET_POPSNAME(es));
    originCountryName = std::string(GET_COUNTRYNAME(es));

    for (size_t index = 0; index < ACRONYM_SIZE; index++) {
        countryAcronym[index] = GET_COUNTRYACRO(es)[index];
    }
}

std::ostream& operator<<(std::ostream& os, const Connection& conn) {
    return os << conn.idTo() << " " << conn.connectionSpeed << "Mbps";
}

double Connection::getSpeed() { return connectionSpeed; }

Connection::Connection(entry* es)
    : Edge(GET_IDCONNECT(es), GET_CONNPOPSID(es)) {
    if (idFrom() == EMPTY_VALUE) {
        throw std::runtime_error("Cannot construct connection with empty "
                                 "idConnect. It must be a valid address.");
    } else if (idTo() == EMPTY_VALUE) {
        throw std::runtime_error(
            "Cannot construct connnection with empty "
            "Connected POP's ID. It must be a valid address.");
    }

    // Convert speed units to megabytes per second (Mbps).
    switch (GET_MEASUREMENT_UNIT(es)[0]) {
    case 'K':
    case 'k':
        connectionSpeed = GET_SPEED(es) / (CONVERSION_FACTOR);
        break;

    case 'M':
    case 'm':
        connectionSpeed = GET_SPEED(es);
        break;

    case 'G':
    case 'g':
        connectionSpeed = GET_SPEED(es) * CONVERSION_FACTOR;
        break;

    default:
        throw std::runtime_error("Invalid measurement unit");
    }
}

Connection::Connection()
    : Edge() {
    connectionSpeed = -1;
}

NetworkGraph::NetworkGraph(const Table& table) {
    // for each entry
    for (entry* es; (es = table.readNextEntry()) != NULL; deleteEntry(es, 1)) {
        try {
            // if the entry is not empty, insert the corresponding node if
            // needed
            NetworkNode new_pop = NetworkNode(es);
            Graph::insertNode(new_pop);
        } catch (std::runtime_error& except) {
            continue; // Do not insert empty nodes and edges.
        }

        try {
            // if the connection is not empty, insert it in the graph
            Connection new_connection = Connection(es);
            Graph::insertEdge(new_connection);
        } catch (std::runtime_error& except) { }
    }
}

std::ostream& operator<<(std::ostream& os, const NetworkGraph& graph) {
    // printing a full graph is the same as printing each node with each edge in
    // separated lines
    for (auto node_it = graph.node_list.begin();
         node_it != graph.node_list.end(); node_it++) {

        if (node_it->second.isEmpty()) {
            continue; // do not print empty NetworkNodes.
        }

        std::vector<Connection> connections;

        try {
            // get all connections for a node
            connections = graph.adjacencies.at(node_it->second.idKey());
        } catch (std::out_of_range& e) {
            continue;
        }

        for (auto edge_it = connections.begin(); edge_it != connections.end();
             edge_it++) {
            // print both the node and connection together
            os << (*node_it).second << " " << *edge_it << std::endl;
        }
    }

    return os;
}

bool NetworkGraph::findPathWithFlow(std::vector<Connection>& path,
    std::map<Edge, int32_t>& flow_used, std::map<int32_t, bool>& marks,
    int32_t node_a_id, int32_t node_b_id) {

    // for every connection in the current node
    for (auto conn : adjacencies[node_a_id]) {
        // if the connection is marked, we already gone there
        if (marks[conn.idTo()]) {
            continue;
        }
        // if there is no bandwidth avaliable, we cannot use the connection
        if (conn.getSpeed() <= flow_used[conn]) {
            continue;
        }

        // mark the node we will travel to and add to the path list
        marks[conn.idTo()] = true;
        path.push_back(conn);

        // if the connected node is the destination, we found a path!
        if (conn.idTo() == node_b_id) {
            return true;
        }

        // travel to the next node trying to find a full path
        bool found_path
            = findPathWithFlow(path, flow_used, marks, conn.idTo(), node_b_id);

        // if we found a path, just return, no point in traveling to other nodes
        if (found_path) {
            return true;
        }

        // pop the connection and unmark the node, we did not find a path
        path.pop_back();
        marks[conn.idTo()] = false;
    }

    // if no connections found any path, we did not find either
    return false;
}

std::vector<Connection> NetworkGraph::findPathWithFlow(
    std::map<Edge, int32_t>& flow_used, int32_t node_a_id, int32_t node_b_id) {

    std::vector<Connection> path {};

    // to keep track of recursion, we mark nodes as we travel along them
    std::map<int32_t, bool> marks {};
    for (auto node : node_list) {
        // initialize all nodes as unmarked
        marks[node.first] = false;
    }

    // mark the first node
    marks[node_a_id] = true;
    // and try to find a path!
    findPathWithFlow(path, flow_used, marks, node_a_id, node_b_id);

    return path;
}

double NetworkGraph::getMaxSpeed(int32_t node_a_id, int32_t node_b_id) {
    try {
        // if the origin or destination nodes do not exist, or do not have any
        // connections, no path exists
        if (adjacencies.at(node_a_id).size() == 0
            || adjacencies.at(node_b_id).size() == 0) {

            return -1;
        }
    } catch (std::out_of_range& e) {
        return -1;
    }

    // the amount of bandwidth used for each edge in the network
    std::map<Edge, int32_t> flow_used {};
    for (auto node : node_list) {
        for (auto edge : adjacencies[node.first]) {
            // initialize all to zero, no bandwidth is being used
            flow_used[edge] = 0;
        }
    }

    double speed = 0;

    // while we cannot find a possible path to give more speed to the connection
    while (true) {
        /*
         * do a depth first search to find a path from node_a to node_b that
         * still has some flow available
         */
        std::vector<Connection> path
            = findPathWithFlow(flow_used, node_a_id, node_b_id);

        if (path.size() == 0) {
            // if there are no more paths, there is nothing left to do: all
            // possible flow was already used to the maximum
            break;
        }

        // the actual speed we will add is the minimum of the full path's
        // connection speeds
        double speed_add = std::numeric_limits<double>::infinity();
        for (auto conn : path) {
            speed_add = std::min(speed_add, conn.getSpeed() - flow_used[conn]);
        }

        // now, with the speed calculated, add the value to all connections
        // used bandwidth, both for the original connection and it's reverse
        for (auto conn : path) {
            flow_used[conn] += speed_add;
            conn.reverse();
            flow_used[conn] += speed_add;
            conn.reverse();
        }

        speed += speed_add;
    }

    return speed;
}

double NetworkGraph::getLen(std::map<int32_t, bool>& marks,
    int32_t node_start_id, int32_t node_end_id, double max_plausable_len,
    double min_possible_len) {

    if (min_possible_len >= max_plausable_len) {
        /*
         * if we already have a solution (max_plausable_len) that has a smaller
         * distance than what we can ever hope to achieve in this recursion
         * (min_possible_len), we can just ignore this recursion: even if we get
         * a solution it will be worst than what we already have
         */
        return max_plausable_len;
    }

    // for all connections in our starting node
    for (auto connection : adjacencies[node_start_id]) {
        // if the connected node is marked, return: we have already gone there
        if (marks[connection.idTo()]) {
            continue;
        }
        // if the connected node is our destiny
        if (connection.idTo() == node_end_id) {
            // get the better solution: what we already have or the new solution
            max_plausable_len = std::min(
                max_plausable_len, connection.getSpeed() + min_possible_len);
            continue;
        }

        // if the connected node is not marked and not the ending node
        // mark it, we will go there now
        marks[connection.idTo()] = true;

        // recusion: start at the connected node with a new min_possible_len
        // to include the distance from the current node to the connected one
        max_plausable_len = getLen(marks, connection.idTo(), node_end_id,
            max_plausable_len, min_possible_len + connection.getSpeed());

        // unmark the node: other paths might use it
        marks[connection.idTo()] = false;
    }

    // return our best solution yet
    return max_plausable_len;
}

double NetworkGraph::getLen(int32_t node_a_id, int32_t node_b_id) {
    try {
        // if the origin or destination nodes do not exist, or do not have any
        // connections, no path exists
        if (adjacencies.at(node_a_id).size() == 0
            || adjacencies.at(node_b_id).size() == 0) {

            return -1;
        }
    } catch (std::out_of_range& e) {
        return -1;
    }

    // all marked nodes, keeps track of recursion to prohibit infinite loops
    std::map<int32_t, bool> marks {};

    for (auto node : node_list) {
        // initialize all nodes as unmarked
        marks[node.first] = false;
    }

    // initialize the recursive version, starting with the first node
    marks[node_a_id] = true;
    double ret = getLen(marks, node_a_id, node_b_id,
        std::numeric_limits<double>::infinity(), 0);
    marks[node_a_id] = false;

    if (ret == std::numeric_limits<double>::infinity()) {
        return -1;
    }

    return ret;
}
