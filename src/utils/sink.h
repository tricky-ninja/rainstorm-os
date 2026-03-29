#pragma once

typedef void (*sink_write_fn)(char ch);

int sink_register(sink_write_fn write_callback);
void sink_remove(int id);
void sink_write(int id, char ch);