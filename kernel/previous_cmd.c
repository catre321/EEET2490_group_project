#include "kernel.h"
#include "previous_cmd.h"
#include "../lib/use_func.h"

char command_history[MAX_COMMANDS][INPUT_BUFFER_SIZE];
unsigned int total_commands = 0;
unsigned int current_command = 0;

void add_command_to_history(char *command)
{
    strncpy(command_history[total_commands % MAX_COMMANDS], command, INPUT_BUFFER_SIZE);
    total_commands++;
    current_command = total_commands;
}

char *get_previous_command()
{
    if (current_command > 0)
    {
        current_command--;
        return command_history[current_command % MAX_COMMANDS];
    }
    return 0;
}

char *get_next_command()
{
    if (current_command < total_commands - 1)
    {
        current_command++;
        return command_history[current_command % MAX_COMMANDS];
    }
    return 0;
}