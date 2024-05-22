#include "kernel.h"
#include "../uart/uart0.h"
#include "../uart/uart0.h"
#include "mbox.h"
#include "framebf.h"
#include "welcome.h"
#include "previous_cmd.h"
#include "cmd.h"
#include "../lib/use_func.h"
#include "../dma/dma.h"

char input_bufer[INPUT_BUFFER_SIZE];
unsigned int input_bufer_idx = 0;
unsigned int is_cmd_right = 0;

void clear_input_buffer()
{
    for (int i = 0; i < INPUT_BUFFER_SIZE; i++)
    {
        input_bufer[i] = '\0';
    }
}

void clear_current_string_uart()
{
    for (int i = input_bufer_idx; i > 0; i--)
    {
        uart0_sendc(8);
        uart0_sendc(' ');
        uart0_sendc(8);
    }
}

int main()
{
    // set up serial console
    uart0_init(115200, 8, 2, 0, 0);
    // Initialize DMA
    dma_init();
    // say hello
    // uart0_puts("lllllllllllllllllllllllllll");
    // Initialize frame buffer
    framebf_init(SCREEN_PYS_WIDTH, SCREEN_PYS_HEIGHT, SCREEN_PYS_WIDTH, SCREEN_PYS_HEIGHT);
    // clear_fb();
    timer_init();
    // Display on terminal
    print_welcome_msg_uart();
    print_OS_initial_text();

    while (1)
    {
        if (is_uart0_byte_ready())
        {
            // read each char
            char c = uart0_getc();

            // backspace
            if (c == 8 || c == 127)
            {
                if (input_bufer_idx > 0)
                {
                    input_bufer_idx--;
                    input_bufer[input_bufer_idx] = '\0';
                    uart0_sendc(8);
                    uart0_sendc(' ');
                    uart0_sendc(8);
                }
            }

            if (input_bufer_idx + 1 >= INPUT_BUFFER_SIZE)
            {
                continue;
            }

            // press tab for auto_completion
            if (c == 9)
            {
                static unsigned int tab_press_count = 0;
                tab_press_count++;
                trim(input_bufer);
                if(tab_press_count > 2){
                    tab_press_count = 1;
                }

                char *matches[sizeof(commands) / sizeof(Command)];
                unsigned int num_matches = 0;
                for (int i = 0; i < sizeof(commands) / sizeof(Command); i++)
                {
                    if (strncmp(input_bufer, commands[i].cmd_name, (input_bufer_idx)) == 0)
                    {
                        matches[num_matches] = commands[i].cmd_name;
                        num_matches++;
                        // print_string("match found\n");
                    }
                }

                if (num_matches == 1 && tab_press_count == 1)
                {
                    clear_current_string_uart();
                    strncpy(input_bufer, matches[0], INPUT_BUFFER_SIZE);
                    uart0_puts(input_bufer);
                    input_bufer_idx = strlen(input_bufer);
                    tab_press_count = 0;
                }
                else if (num_matches > 1 && tab_press_count >= 2)
                {
                    uart0_puts("\n");
                    for (int i = 0; i < num_matches; i++)
                    {
                        uart0_puts(matches[i]);
                        uart0_puts("\t");
                    }
                    tab_press_count = 0;
                    uart0_puts("\n");
                    print_OS_initial_text();
                    uart0_puts(input_bufer);
                }
            }

            if (c == '_')
            {
                // up arrow
                char *previous_command = get_previous_command();
                if (previous_command == 0) 
                {
                    continue;
                }
                clear_current_string_uart();
                strncpy(input_bufer, previous_command, INPUT_BUFFER_SIZE);
                input_bufer[INPUT_BUFFER_SIZE - 1] = '\0'; // ensure '\0' termination
                input_bufer_idx = strlen(input_bufer);
                uart0_puts(input_bufer);
                continue;
            }
            if (c == '+')
            {
                // down arrow
                char *next_command = get_next_command();
                if (next_command == 0) 
                {
                    continue;
                }
                clear_current_string_uart();
                strncpy(input_bufer, next_command, INPUT_BUFFER_SIZE);
                input_bufer[INPUT_BUFFER_SIZE - 1] = '\0'; // ensure '\0' termination
                input_bufer_idx = strlen(input_bufer);
                uart0_puts(input_bufer);
                continue;
            }

            // valid character
            if (c >= 32 && c <= 126)
            {
                input_bufer[input_bufer_idx] = c;
                input_bufer_idx++;
                // draw the character to the screen at the current cursor position
                uart0_sendc(c);
            }

            // enter key
            if (c == '\n')
            {
                input_bufer[input_bufer_idx] = '\0';
                add_command_to_history(input_bufer);
                trim(input_bufer);
                // Split the input into command and arguments
                char *saveptr;
                char *cmd = strtok_r(input_bufer, " ", &saveptr);
                char *arg = strtok_r(0, "\0", &saveptr);

                // case no arg enter
                if (cmd == 0)
                {
                    cmd = input_bufer;
                }

                is_cmd_right = 0;
                for (int i = 0; i < sizeof(commands) / sizeof(Command); i++)
                {
                    if (strncmp(commands[i].cmd_name, cmd, 10) == 0)
                    {
                        is_cmd_right = 1;
                        uart0_sendc('\r');
                        uart0_sendc('\n');
                        commands[i].function(arg);
                        break;
                    }
                }

                if (!is_cmd_right)
                {
                    uart0_puts("\n");
                    uart0_puts(input_bufer);
                    uart0_puts(": is not a valid command\n");
                }

                input_bufer_idx = 0;
                clear_input_buffer();
                print_OS_initial_text();
            }
        }
        // send back
        // uart0_sendc(cursor_x);
        // uart0_hex(cursor_x);
    }
    return 0;
}
