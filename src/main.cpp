#include <cerrno>
#include <iostream>

#include "NetworkGraph.hpp"
#include "commands.hpp"
#include "table.hpp"

extern "C" {
#include "utils.h"
}

#define FIRST_COMMAND_NUM 11
enum commands {
    command_print = FIRST_COMMAND_NUM,
    command_num_cicles,
    command_max_speed,
    command_length
};

int main() {
    int32_t command;
    char* table_name;
    READ_INPUT("%d %ms", &command, &table_name);

    Table* topology = new Table(table_name, "rb");
    NetworkGraph* net_topology = new NetworkGraph(*topology);

    switch (command) {
    case command_print:
        std::cout << *net_topology;
        break;
    case command_num_cicles:
        std::cout << "Quantidade de ciclos: " << net_topology->getNumCicles();
        std::cout << std::endl;
        break;
    case command_max_speed:
        commandMaxSpeed(*net_topology);
        break;
    case command_length:
        commandLength(*net_topology);
        break;
    default:
        errno = EINVAL;
        ABORT_PROGRAM("command number");
    }

    delete net_topology;
    delete topology;
    free(table_name);
}
