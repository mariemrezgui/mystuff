#ifndef _SHELLFUNCTIONS_H_
#define _SHELLFUNCTIONS_H_

void read_input(char* const command_line, int size, FILE* stream);

void collect_defunct_process();

void execute_command(char* command_line);
#endif
