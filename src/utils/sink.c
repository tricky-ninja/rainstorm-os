#include "sink.h"
#include "stdbool.h"
#include "memory.h"

#define SINK_MAX 256

static sink_write_fn sinks[256];
static int curr_idx = -1;

int sink_register(sink_write_fn write_callback)
{
    if (curr_idx < 0) memset(&sinks[0], 0, SINK_MAX);
    if (((curr_idx+1) >= SINK_MAX) || (sinks[curr_idx+1] != NULL)) return -1;
    
    sinks[curr_idx+1] = write_callback;
    return ++curr_idx;
}

void sink_remove(int id)
{
    if (id >= SINK_MAX) return;
    sinks[id] = NULL;
}

void sink_write(int id, char ch)
{
    if (id >= SINK_MAX) return;
    if (sinks[id] == NULL) return;
    sinks[id](ch);
}