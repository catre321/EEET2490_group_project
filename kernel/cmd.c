#include "cmd.h"
#include "../uart/uart0.h"
#include "cursor.h"
#include "../lib/use_func.h"
#include "kernel.h"
#include "welcome.h"
#include "mbox.h"
#include "../uart/uart0.h"
#include "framebf.h"
#include "../images/big_image.h"

Command commands[COMMANDS_SIZE] = {
    {"help", "Prints help text", "- Show full information of the command\n"
                                 "- Example: MyOS> help hwinfo",
     help},
    {"clear", "Clears the screen", "- Clear screen (in our terminal it will scroll down to current position of the cursor)\n"
                                   "- Example: MyOS> clear",
     clear},
    {"setcolor", "Set text color, and/or background color", "- Set text color, and/or background color of the console to one of the following colors: BLACK, RED, GREEN, YELLOW, BLUE, PURPLE, CYAN, WHITE\n"
                                                            "- Examples:\n"
                                                            "CatfishOS> setcolor -t yellow\n"
                                                            "CatfishOS> setcolor -b yellow -t white",
     setcolor},
    {"showinfo", "Show board information", "- Show board revision and board MAC address in correct format/ meaningful information\n"
                                           "- Example: MyOS> showinfo",
     showinfo},
    {"fetch", "Show welcome message", "- Show welcome message\n"
                                      "- Example: MyOS> fetch",
     fetch},
    {"UART0config", "Configure UART0", "- Configure UART0 baudrate, data bits, stop bits, parity, handshaking control\n"
                                       "- Example: MyOS> UART0 config",
     UART0_config},
    {"bigimage", "Display a oversize image", "- Display a oversize image and use WASD to move around image\n"
                                             "- Example: CatfishOS> bigimage",
     bigimage}
    };


Color_code text_colors[TEXT_COLOR_SIZE] = {
    {"BLACK", "\033[30m"},
    {"RED", "\033[31m"},
    {"GREEN", "\033[32m"},
    {"YELLOW", "\033[33m"},
    {"BLUE", "\033[34m"},
    {"PURPLE", "\033[35m"},
    {"CYAN", "\033[36m"},
    {"WHITE", "\033[37m"}};

Color_code bg_colors[BG_COLOR_SIZE] = {
    {"BLACK", "\033[40m"},
    {"RED", "\033[41m"},
    {"GREEN", "\033[42m"},
    {"YELLOW", "\033[43m"},
    {"BLUE", "\033[44m"},
    {"PURPLE", "\033[45m"},
    {"CYAN", "\033[46m"},
    {"WHITE", "\033[47m"}};

void print_mac_address(unsigned int part1, unsigned int part2)
{
    unsigned int mac_address[6];

    // Extract bytes from the u32 values
    mac_address[0] = part1 & 0xFF;
    mac_address[1] = (part1 >> 8) & 0xFF;
    mac_address[2] = (part1 >> 16) & 0xFF;
    mac_address[3] = (part1 >> 24) & 0xFF;
    mac_address[4] = part2 & 0xFF;
    mac_address[5] = (part2 >> 8)& 0xFF;

    for (int i = 0; i < sizeof(mac_address) / sizeof(mac_address[0]); i++)
    {
        if (i != 0)
            uart0_puts(":");
        // uart0_hex(mac_address[i]);
        for (int pos = 4; pos >= 0; pos = pos - 4)
        {

            // Get highest 4-bit nibble
            char digit = (mac_address[i] >> pos) & 0xF;

            /* Convert to ASCII code */
            // 0-9 => '0'-'9', 10-15 => 'A'-'F'
            digit += (digit > 9) ? (-10 + 'A') : '0';
            uart0_sendc(digit);
        }
    }
}

char get_char(char *exit_code_ptr)
{
    char input = '0';
    unsigned int input_idx = 0;
    char c;
    while (1)
    {
        if(*exit_code_ptr){
            return '0';
        }
        if (is_uart0_byte_ready())
        {

            c = uart0_getc();
            if(c == 26){
                *exit_code_ptr = 1;
                return 0;
            }
            if (c == 8)
            {
                if (input_idx > 0)
                {
                    input_idx--;
                    input = '0';
                    uart0_sendc(8);
                    uart0_sendc(' ');
                    uart0_sendc(8);
                    continue;
                }
            }
            if (c == '\n')
            {
                return input;
            }
            // if they already enter a char
            if (input_idx >= 1)
            {
                continue;
            }
            if (c >= 32 && c <= 126)
            {
                input = c;
                input_idx++;
                uart0_sendc(c);
            }
        }
    }
}

void help(char *args)
{
    if (args == 0)
    {
        uart0_puts("Available commands:\n");
        for (int i = 0; i < COMMANDS_SIZE; i++)
        {
            uart0_puts(commands[i].cmd_name);
            // new_line();
            uart0_puts("    ");
            uart0_puts(commands[i].help_text_short);
            uart0_puts("\n");
        }
    }
    else
    {
        for (int i = 0; i < COMMANDS_SIZE; i++)
        {
            if (strncmp(commands[i].cmd_name, args, 10) == 0)
            {
                uart0_puts(commands[i].cmd_name);
                uart0_puts("\n");
                uart0_puts(commands[i].help_text);
                uart0_puts("\n");
                return;
            }
        }
        uart0_puts("Command not found\n");
    }
}

void clear(char *args)
{
    // ANSI escape code to clear screen
    uart0_puts("\033[H\033[J");
}

void setcolor(char *args)
{
    char *save_ptr;
    char *text_color = 0;
    char *bg_color = 0;

    if(args == 0){
        uart0_puts("No color specified, type \"help setcolor\" for example\n");
        return;
    }

    char *token = strtok_r(args, " ", &save_ptr);

    while (token != 0)
    {
        if (strncmp(token, "-t", 2) == 0)
        {
            token = strtok_r(0, " ", &save_ptr);
            text_color = token;
        }
        else if (strncmp(token, "-b", 2) == 0)
        {
            token = strtok_r(0, " ", &save_ptr);
            bg_color = token;
        }
        token = strtok_r(0, " ", &save_ptr);
    }

    if (text_color != 0)
    {
        to_uppercase(text_color);
        for (int i = 0; i < TEXT_COLOR_SIZE; i++)
        {
            if (strncmp(text_color, text_colors[i].name, strlen(text_colors[i].name)) == 0)
            {
                uart0_puts(text_colors[i].code);
            }
        }
    }

    if (bg_color != 0)
    {
        to_uppercase(bg_color);
        for (int i = 0; i < BG_COLOR_SIZE; i++)
        {
            if (strncmp(bg_color, bg_colors[i].name, strlen(bg_colors[i].name)) == 0)
            {
                uart0_puts(bg_colors[i].code);
            }
        }
    }
}

void showinfo(char *args)
{
    // mailbox data buffer: Read ARM frequency
    mBuf[0] = 12 * 4;       // Message Buffer Size in bytes (8 elements * 4 bytes (32 bit) each)
    mBuf[1] = MBOX_REQUEST; // Message Request Code (this is a request message)

    mBuf[2] = 0x00010002; // TAG Identifier: board revision
    mBuf[3] = 4;          // Value buffer size in bytes (max of request and response lengths)
    mBuf[4] = 0;          // REQUEST CODE = 0
    mBuf[5] = 0;          // clear output buffer (response data are mBuf[5])

    mBuf[6] = 0x00010003; // TAG Identifier: Get board MAC address
    mBuf[7] = 6;          // Value buffer size in bytes (max of request and response lengths)
    mBuf[8] = 0;          // REQUEST CODE = 0
    mBuf[9] = 0;          // clear output buffer (response data are mBuf[9])
    mBuf[10] = 0;         // clear output buffer (response data are mBuf[10])

    // mBuf[11] = 0x00030002; // TAG Identifier: Get clock rate
    // mBuf[12] = 8;          // Value buffer size in bytes (max of request and response lengths)
    // mBuf[13] = 0;          // REQUEST CODE = 0
    // mBuf[14] = 2;          // clock id: UART clock
    // mBuf[15] = 0;          // clear output buffer (response data are mBuf[14] & mBuf[15])

    mBuf[11] = MBOX_TAG_LAST;

    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP))
    {
        // uart0_puts("\nResponse Code for whole message: ");
        // uart0_hex(mBuf[1]);

        uart0_puts("+ Board revision: ");
        uart0_hex(mBuf[5]);

        uart0_puts("\n+ Board MAC address: ");
        // uart0_hex(mBuf[9]);
        // uart0_puts(" ");
        // uart0_hex(mBuf[10]);
        print_mac_address(mBuf[9], mBuf[10]);

        // uart0_puts("\n+ UART clock rate: ");
        // uart0_dec(mBuf[15]);

        uart0_puts("\n");
    }
    else
    {
        uart0_puts("Unable to query!\n");
    }
}

void fetch(char *args)
{
    print_fetch_msg_display();
}

void UART0_config(char *args)
{
    char exit_code = 0;
    char *exit_code_ptr = &exit_code;
    // default values
    unsigned int baud_rate = 115200;
    unsigned int data_bits = 8;
    unsigned int stop_bits = 1;
    unsigned int parity = 0;
    unsigned int handshaking = 0;
    uart0_puts("UART0 config\n");

    uart0_puts("Choose a baud rate from the following options:\n");
    uart0_puts("1-9600 2-19200 3-38400 4-57600 5-115200(default)\n");
    uart0_puts("Enter the number corresponding to the baud rate: ");
    while (1)
    {
        char input = get_char(exit_code_ptr);
        // uart0_puts(input);
        switch (input)
        {
        case '1':
            baud_rate = 9600;
            break;
        case '2':
            baud_rate = 19200;
            break;
        case '3':
            baud_rate = 38400;
            break;
        case '4':
            baud_rate = 57600;
            break;
        case '5':
            baud_rate = 115200;
            break;
        case '0': // User entered nothing
            baud_rate = 115200;
            break;
        default:
            uart0_puts("\nInvalid input. Please enter a number between 1 and 5, or leave it empty for default: ");
            continue;
        }
        break;
    }

    uart0_puts("\nEnter number of data bits (5, 6, 7, 8) (default 8): ");
    while(1){
        char input = get_char(exit_code_ptr);
        switch (input)
        {
        case '5':
            data_bits = 5;
            break;
        case '6':
            data_bits = 6;
            break;
        case '7':
            data_bits = 7;
            break;
        case '8':
            data_bits = 8;
            break;
        case '0': 
            data_bits = 8;
            break;
        default:
            uart0_puts("\nInvalid input. Please enter a number between 5 and 8, or leave it empty for default: ");
            continue;   
        }
        break;
    };

    uart0_puts("\nEnter number of stop bits (1, 2) (default 1): ");
    while (1)
    {
        char input = get_char(exit_code_ptr);
        switch (input)
        {
        case '1':
            stop_bits = 1;
            break;
        case '2':
            stop_bits = 2;
            break;
        case '0':
            stop_bits = 1;
            break;
        default:
            uart0_puts("\nInvalid input. Please enter 1 or 2, or leave it empty for default: ");
            continue;
        }
        break;
    };    

    uart0_puts("\nEnter parity (0 for none, 1 for odd, 2 for even) (default none): ");
    while (1)
    {
        char input = get_char(exit_code_ptr);
        switch (input)
        {
        case '1':
            parity = 1;
            break;
        case '2':
            parity = 2;
            break;
        case '0':
            parity = 0;
            break;
        default:
            uart0_puts("\nInvalid input. Please enter 0, 1, or 2: ");
            continue;
        }
        break;
    };

    uart0_puts("\nEnter handshaking control (0 for off, 1 for on) (default off): ");
    while (1)
    {
        char input = get_char(exit_code_ptr);
        switch (input)
        {
        case '1':
            handshaking = 1;
            break;
        case '0':
            handshaking = 0;
            break;
        default:
            uart0_puts("\nInvalid input. Please enter 0 or 1: ");
            continue;
        }
        break;
    };

    if(exit_code){
        uart0_puts("\nUART0 configuration exited and nothing change!!!\n");
        return;
    };
    uart0_puts("\nUART0 set value: ");
    uart0_dec(baud_rate);
    uart0_puts(" ");
    uart0_dec(data_bits);
    uart0_puts(" ");
    uart0_dec(stop_bits);
    uart0_puts(" ");
    uart0_dec(parity);
    uart0_puts(" ");
    uart0_dec(handshaking);
    uart0_puts("\n");
    uart0_puts("Configuring UART0...\n");
    uart0_init(baud_rate, data_bits, stop_bits, parity, handshaking);

    uart0_puts("\nUART0 configured\n");
}

void bigimage(){
    uart0_puts("Enter the bigimage display, press 'Ctrl+Z' to exit");
    draw_image(0, 0, SCREEN_VIR_WIDTH, SCREEN_VIR_HEIGHT, epd_bitmap_big_image);    
   
    int currX = 0;
    int currY = 0;

    while(1) {
        char c = 0;
        if(is_uart0_byte_ready()){
            c = uart0_getc();
        }
        
        switch(c) {
            case 'w':
            case 'W':
                currY -= 1;
                if(currY < 0){
                    currY = 0;
                }
                set_virtual_offset(currX, currY);
                break;
            case 'a':
            case 'A':
                currX -= 4;
                if(currX < 0){
                    currX = 0;
                }
                set_virtual_offset(currX, currY);
                break;
            case 's':
            case 'S':
                currY += 1;
                if(currY > BIG_IMAGE_HEIGHT - SCREEN_PYS_HEIGHT){
                    currY = BIG_IMAGE_HEIGHT - SCREEN_PYS_HEIGHT;
                }    
                set_virtual_offset(currX, currY);
                break;
            case 'd':
            case 'D':
                currX += 4;
                if(currX > BIG_IMAGE_WIDTH - SCREEN_PYS_WIDTH){
                   currX = BIG_IMAGE_WIDTH - SCREEN_PYS_WIDTH; 
                }
                set_virtual_offset(currX, currY);
                break;
            // Ctrl+z to exit
            case 26:
                set_virtual_offset(0, 0);
                return;
            default: 
                break;
        }
    }
}