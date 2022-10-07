#ifndef __FILE_CONTROL_H__
#define __FILE_CONTROL_H__

#include <inttypes.h>
#include <stdbool.h>


typedef struct{
    bool status;
    uint32_t stack;
    uint32_t nextRRN;
    uint32_t entries_removed;
    uint32_t pages;
    uint32_t times_compacted;
} header;

#endif
