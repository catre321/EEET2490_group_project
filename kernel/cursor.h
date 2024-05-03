#ifndef CURSOR_H
#define CURSOR_H

extern unsigned int cursor_x;
extern unsigned int cursor_y;
extern unsigned int cursor_x_from_initital;
extern unsigned char color;

void new_line();
void print_cursor();
void clear_cursor();
void clear_current_string();
void print_char(unsigned char ch);
void print_string(char *s);

#endif
