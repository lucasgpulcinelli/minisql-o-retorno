#ifndef __COMMANDS_HPP__
#define __COMMANDS_HPP__

#include "NetworkGraph.hpp"

/*
 * commandMaxSpeed reads an integer n passed by the user, and
 * reads n times a pair of integers origin_pop and destiny_pop. As the
 * name suggests, this is the pair of addresses of an origin POP and
 * the address of a destination POP. For each pair, it prints the
 * maximum possible speed of connection between origin POP and
 * destination POP.
 */
void commandMaxSpeed(NetworkGraph& net_topology);

/*
 * commandLength reads an integer n passed by the user, and reads
 * n times a triple (origin_pop, destination_pop, stop). For each triple,
 * it prints the minimum length between origin_pop and destination_pop
 * if one is required to pass at 'stop'.
 */
void commandLength(NetworkGraph& net_topology);

#endif