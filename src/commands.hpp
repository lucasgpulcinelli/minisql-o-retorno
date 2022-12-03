#ifndef __COMMANDS_H__
#define __COMMANDS_H__

#include "table.hpp"

#define COMMANDS_SIZE 4
#define COMMANDS_OFFSET 10

void commandPrint(const Table& t);

void commandCicles(const Table& t);

void commandSpeed(const Table& t);

void commandTravel(const Table& t);

#endif

