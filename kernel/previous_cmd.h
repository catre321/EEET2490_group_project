#ifndef PREVIOUS_CMD_H
#define PREVIOUS_CMD_H

#define MAX_COMMANDS 10

void add_command_to_history(char* command);
char* get_previous_command();
char* get_next_command();


#endif