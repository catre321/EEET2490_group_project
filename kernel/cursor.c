#include "cursor.h"
#include "framebf.h"

unsigned int cursor_x = 0;
unsigned int cursor_y = 0;
unsigned int cursor_x_from_initital = 0;
unsigned char color = 0;

void new_line()
{
    cursor_x = 0;
    cursor_y += 8;
}

void print_cursor()
{
    draw_char(178, cursor_x, cursor_y, color); // draw box
}

void clear_cursor()
{
    draw_char(0, cursor_x, cursor_y, color);
}

void clear_current_string(){
    for (int i = cursor_x; i > cursor_x_from_initital; i--)
    {
        draw_char(0, i, cursor_y, color);
    }
    cursor_x = cursor_x_from_initital;
}

void print_char(unsigned char ch)
{
    draw_char(ch, cursor_x, cursor_y, color);
}

void print_string(char *s)
{
    for (int i = 0; s[i] != '\0'; i++)
    {
        if(s[i] == '\n'){
            cursor_x = 0;
            cursor_y += 8;
        } else {
            print_char(s[i]);
            cursor_x += 8;
        }
    }
}

// print_char_from(unsigned int x, unsigned int y, unsigned char ch)
// {
//     draw_char(ch, x, y, color);
// }

// print_string_from(unsigned int x, unsigned int y, char *s)
// {
//     for (int i = 0; s[i] != '\0'; i++)
//     {
//         print_char_from(x, y, s[i]);
//         x += 8;
//     }
// }
