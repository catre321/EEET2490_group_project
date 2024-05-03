// ----------------------------------- cmd.h -------------------------------------
#ifndef CMD_H
#define CMD_H

#define COMMANDS_SIZE 6
#define TEXT_COLOR_SIZE 8
#define BG_COLOR_SIZE 8

typedef struct {
    char *cmd_name;
    char *help_text_short;
    char *help_text;
    void (*function)(char *args);
} Command;

typedef struct
{
    char *name;
    char *code;
} Color_code;

extern Color_code text_colors[TEXT_COLOR_SIZE];
extern Color_code bg_colors[BG_COLOR_SIZE];
extern Command commands[COMMANDS_SIZE];

// #define INPUT_BUFFER_SIZE 64
void help(char *args);
void clear(char *args);
void setcolor(char *args);
void showinfo(char *args);
void fetch(char *args);
void UART0_config(char *args);

#endif