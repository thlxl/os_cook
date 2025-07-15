#ifndef MEM_H
#define MEM_H

#include "type.h"
#include "config.h"
#include "task.h"

void *os_malloc(size_t xWantedSize);
void os_free(void *pv);

#endif /* MEM_H */