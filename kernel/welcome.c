#include "welcome.h"
#include "framebf.h"
#include "../uart/uart0.h"


char ascii_art[] = {
    "===================================================================================\n"
    "=        ==        ==        ==        =====   ==========  ======     =====      ==\n"
    "=  ========  ========  ===========  ======   =   =======   =====  ===   ==   ==   =\n"
    "=  ========  ========  ===========  =====   ===   =====    ====  =====  ==  ====  =\n"
    "=  ========  ========  ===========  ==========   =====  =  ====  =====  ==  ====  =\n"
    "=      ====      ====      =======  =========   =====  ==  =====  ===   ==  ====  =\n"
    "=  ========  ========  ===========  ========   =====  ===  =======   =  ==  ====  =\n"
    "=  ========  ========  ===========  =======   ======         =========  ==  ====  =\n"
    "=  ========  ========  ===========  ======   ============  ====  =====  ==   ==   =\n"
    "=        ==        ==        =====  =====        ========  =====       ====      ==\n"
    "===================================================================================\n"
    "\n"
    "\tDeveloped by Group 9\n"
    "\0"
};

char credit[] = {"\n\t\t\tCredits\n"};
char person1[] = {"\tNguyen Hoang - s3929351\n"};
char person2[] = {"\tVo Hong Trien - s3907397\n"};
char person3[] = {"\tNguyen Hoang Duy - s3978268\n"};
char person4[] = {"\tPhan Bao Kim Ngan - s3914582\n"};


void print_welcome_msg_uart(){
    uart0_puts(ascii_art);
}

void print_fetch_msg_display(){
    int zoom = 1;
    int y_offset = 0;
    // print_string(ascii_art);
    y_offset = drawString(0, y_offset, ascii_art, 0xFFFF55, zoom); // yellow
    y_offset = drawString(0, y_offset, credit, 0x0000AA, zoom); // blue
    y_offset = drawString(0, y_offset, person1, 0x00AA00, zoom); // green
    y_offset = drawString(0, y_offset, person2, 0x00AAAA, zoom); // cyan
    y_offset = drawString(0, y_offset, person3, 0xAA00AA, zoom); // magenta
    y_offset = drawString(0, y_offset, person4, 0xAA0000, zoom); // red
}

char OS_initial_text[] = {"\nGroup9OS>"};

void print_OS_initial_text()
{
    uart0_puts(OS_initial_text);
}
