// ----------------------------------- framebf.c -------------------------------------
#include "mbox.h"
#include "../uart/uart0.h"
#include "framebf.h"
#include "font.h"
#include "../dma/timer.h"
#include "../dma/dma.h"
#include "../images/full_black.h"

// Use RGBA32 (32 bits for each pixel)
#define COLOR_DEPTH 32
// Pixel Order: BGR in memory order (little endian --> RGB in byte order)
#define PIXEL_ORDER 0
// Screen info
unsigned int width, height, pitch;
/* Frame buffer address
 * (declare as pointer of unsigned char to access each byte) */
unsigned char *fb;
unsigned int cur_val_1 = 0;
// Frame buffer size in Byte
unsigned int frame_buffer_size;
unsigned int curr_virtual_width;
unsigned int curr_virtual_height;

static dma_channel *dma;

void framebf_init(int phy_width, int phy_height, int vir_width, int vir_height)
{
    mBuf[0] = 35 * 4; // Length of message in bytes
    mBuf[1] = MBOX_REQUEST;
    mBuf[2] = MBOX_TAG_SETPHYWH;  // Set physical width-height
    mBuf[3] = 8;                  // Value size in bytes
    mBuf[4] = 0;                  // REQUEST CODE = 0
    mBuf[5] = phy_width;               // Value(width)
    mBuf[6] = phy_height;                // Value(height)
    mBuf[7] = MBOX_TAG_SETVIRTWH; // Set virtual width-height
    mBuf[8] = 8;
    mBuf[9] = 0;
    mBuf[10] = vir_width;
    mBuf[11] = vir_height;
    mBuf[12] = MBOX_TAG_SETVIRTOFF; // Set virtual offset
    mBuf[13] = 8;
    mBuf[14] = 0;
    mBuf[15] = 0;                 // x offset
    mBuf[16] = 0;                 // y offset
    mBuf[17] = MBOX_TAG_SETDEPTH; // Set color depth
    mBuf[18] = 4;
    mBuf[19] = 0;
    mBuf[20] = COLOR_DEPTH;         // Bits per pixel
    mBuf[21] = MBOX_TAG_SETPXLORDR; // Set pixel order
    mBuf[22] = 4;
    mBuf[23] = 0;
    mBuf[24] = PIXEL_ORDER;
    mBuf[25] = MBOX_TAG_GETFB; // Get frame buffer
    mBuf[26] = 8;
    mBuf[27] = 0;
    mBuf[28] = 16;                // alignment in 16 bytes
    mBuf[29] = 0;                 // will return Frame Buffer size in bytes
    mBuf[30] = MBOX_TAG_GETPITCH; // Get pitch
    mBuf[31] = 4;
    mBuf[32] = 0;
    mBuf[33] = 0; // Will get pitch value here
    mBuf[34] = MBOX_TAG_LAST;
    // Call Mailbox
    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP) // mailbox call is successful ?
        && mBuf[20] == COLOR_DEPTH          // got correct color depth ?
        && mBuf[24] == PIXEL_ORDER          // got correct pixel order ?
        && mBuf[28] != 0                    // got a valid address for frame buffer ?
    )
    {
        /* Convert GPU address to ARM address (clear higher address bits)
         * Frame Buffer is located in RAM memory, which VideoCore MMU
         * maps it to bus address space starting at 0xC0000000.
         * Software accessing RAM directly use physical addresses
         * (based at 0x00000000)
         */
        mBuf[28] &= 0x3FFFFFFF;
        // Access frame buffer as 1 byte per each address
        fb = (unsigned char *)((unsigned long)mBuf[28]);
        uart0_puts("Got allocated Frame Buffer at RAM physical address: ");
        uart0_hex(mBuf[28]);
        uart0_puts("\n");
        uart0_puts("Frame Buffer Size (bytes): ");
        uart0_dec(mBuf[29]);
        uart0_puts("\n");
        frame_buffer_size = mBuf[29];
        curr_virtual_width = width;
        curr_virtual_height = height;
        width = mBuf[5];
        // Actual physical width
        height = mBuf[6];
        // Actual physical height
        pitch = mBuf[33]; // Number of bytes per line
    }
    else
    {
        uart0_puts("Unable to get a frame buffer with provided setting\n");
    }
    // dma = dma_open_channel(5);
    // test_dma();
}

void dma_init(){
    // int is_DMA_work = 0;
    // while(!is_DMA_work){
        dma = dma_open_channel(5);
    // }
    uart0_puts("DMA CHANNEL: ");
    uart0_dec(dma->channel);
    uart0_puts("\n");
        // is_DMA_work = test_dma();
        test_dma();
}


void timer_init() {
    cur_val_1 = REGS_TIMER->counter_lo;
    cur_val_1 += 1000000;
    REGS_TIMER->compare[1] = cur_val_1;
}

unsigned long timer_get_ticks() {
    unsigned int hi = REGS_TIMER->counter_hi;
    unsigned int lo = REGS_TIMER->counter_lo;

    //double check hi value didn't change after setting it...
    if (hi != REGS_TIMER->counter_hi) {
        hi = REGS_TIMER->counter_hi;
        lo = REGS_TIMER->counter_lo;
    }

    return ((unsigned long)hi << 32) | lo;
}

void do_dma(void *dest,const void *src, unsigned int total) {
    unsigned int ms_start = timer_get_ticks() / 1000;

    unsigned int start = 0;

    while(total > 0) {
        int num_bytes = total;

        if (num_bytes > 0xFFFFFF) {
            num_bytes = 0xFFFFFF;
        }
        
        dma_setup_mem_copy(dma, dest + start, src + start, num_bytes, 2);
        
        dma_start(dma);

        dma_wait(dma);

        start += num_bytes;
        total -= num_bytes;
    }

    unsigned int ms_end = timer_get_ticks() / 1000;
    uart0_dec((ms_end - ms_start));
}

void set_virtual_offset(int x, int y) {
    // mbox to set virtual offset to (0, SCREENHEIGHT)
    mBuf[0] = 8 * 4;  // Length of message in bytes
    mBuf[1] = MBOX_REQUEST;

    mBuf[2] = MBOX_TAG_SETVIRTOFF;    // Set physical width-height
    mBuf[3] = 8;                      // Value size in bytes
    mBuf[4] = 0;                      // REQUEST CODE = 0
    mBuf[5] = x;                      // Value(width)
    mBuf[6] = y;  // Value(height)

    mBuf[7] = MBOX_TAG_LAST;

    // Call Mailbox
    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)) {
        uart0_puts("Set offset x-y: ");
        uart0_dec(x);
        uart0_puts(" ");
        uart0_dec(y);
        uart0_puts("\n");
    } else {
        uart0_puts("Unable to set virtual offset\n");
    }
}

void set_virtual_screen_size(unsigned int width, unsigned int height){
    mBuf[0] = 13 * 4;  // Length of message in bytes
    mBuf[1] = MBOX_REQUEST; 

    mBuf[2] = MBOX_TAG_SETVIRTWH; // Set virtual width-height
    mBuf[3] = 8;
    mBuf[4] = 0;
    mBuf[5] = width;
    mBuf[6] = height;

    mBuf[7] = MBOX_TAG_GETFB; // Get frame buffer
    mBuf[8] = 8;
    mBuf[9] = 0;
    mBuf[10] = 16;                // alignment in 16 bytes
    mBuf[11] = 0;                 // will return Frame Buffer size in bytes

    mBuf[12] = MBOX_TAG_LAST;


    // Call Mailbox
    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)
        && mBuf[15] != 0                    // got a valid address for frame buffer ?
    ) {
        uart0_puts("Successfully set virtual screen: ");
        uart0_dec(width);
        uart0_puts("x");
        uart0_dec(height);
        uart0_puts("\n");

        /* Convert GPU address to ARM address (clear higher address bits)
         * Frame Buffer is located in RAM memory, which VideoCore MMU
         * maps it to bus address space starting at 0xC0000000.
         * Software accessing RAM directly use physical addresses
         * (based at 0x00000000)
         */
        mBuf[15] &= 0x3FFFFFFF;
        // Access frame buffer as 1 byte per each address
        fb = (unsigned char *)((unsigned long)mBuf[15]);
        frame_buffer_size = mBuf[16];
        curr_virtual_height = height;
        curr_virtual_width = width;
    } else {
        uart0_puts("Unable to set virtual screen size\n");
    }
}

void clear_buffer(){
    uart0_puts("clear_buffer called!");
    // for(int i = 0; i < frame_buffer_size; i++){
    //     // i[fb] = 0; // curse pattern 
    //     fb[i] = 0;
    // }
    do_dma(fb, full_black, frame_buffer_size);
}

void draw_pixel_ARGB32(int x, int y, unsigned int attr)
{
    int offs = (y * pitch) + (COLOR_DEPTH / 8 * x);
    /*
    //Access and assign each byte
    *(fb + offs ) = (attr >> 0 ) & 0xFF; //BLUE
    *(fb + offs + 1) = (attr >> 8 ) & 0xFF; //GREEN
    *(fb + offs + 2) = (attr >> 16) & 0xFF; //RED
    *(fb + offs + 3) = (attr >> 24) & 0xFF; //ALPHA
    */
    // Access 32-bit together
    *((unsigned int *)(fb + offs)) = attr;
}

int draw_pixel(int x, int y, unsigned char attr)
{
    int offs = (y * pitch) + (x * 4);
    *((unsigned int *)(fb + offs)) = vgapal[attr & 0x0f];
    // uart0_hex(&fb);
    // *((unsigned int *)(fb + offs)) = 0x00FFFF55;
    return 0;
}

int draw_char(unsigned char ch, int x, int y, unsigned char attr)
{
    unsigned char *glyph = (unsigned char *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;
    // unsigned char *glyph = (unsigned char *)&Font10x20 + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

    for (int i = 0; i < FONT_HEIGHT; i++)
    {
        for (int j = 0; j < FONT_WIDTH; j++)
        {
            // @ ARGB
            unsigned char mask = 1 << j;
            // right set color false black (print text)
            unsigned char col = (*glyph & mask) ? attr & 0x0F : (attr & 0xF0) >> 4;

            draw_pixel(x + j, y + i, col);
        }
        glyph += FONT_BPL;
    }
    return 0;
}

void draw_string(int x, int y, char *s, unsigned char attr)
{
    while (*s)
    {
        if (*s == '\r')
        {
            x = 0;
        }
        else if (*s == '\n')
        {
            x = 0;
            y += FONT_HEIGHT;
        }
        else
        {
            draw_char(*s, x, y, attr);
            x += FONT_WIDTH;
        }
        s++;
    }
}

void draw_rect_ARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill)
{
    for (int y = y1; y <= y2; y++)
        for (int x = x1; x <= x2; x++)
        {
            if ((x == x1 || x == x2) || (y == y1 || y == y2))
                draw_pixel_ARGB32(x, y, attr);
            else if (fill)
                draw_pixel_ARGB32(x, y, attr);
        }
}

// draw image v1 using CPU
void draw_image(int startX, int startY, int endX, int endY, const unsigned int *imgArray){
    uart0_puts("draw_image CALLED");
    unsigned int pixel = 0;
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            draw_pixel_ARGB32(x, y, imgArray[pixel]);
            pixel++;
        }
    }
}

// draw image v2 using DMA
void draw_imagev2(const unsigned int *imgArray){
    // uart0_puts("draw_image v2 CALLED");
    do_dma(fb, imgArray, frame_buffer_size);
}

// Test DMA functionality
int compare_memory(const unsigned int *a, const unsigned int *b, unsigned int size) {
    for (unsigned int i = 0; i < size; i++) {
        uart0_dec(a[i]);
        uart0_puts("-");
        uart0_dec(b[i]);
        uart0_puts(" ");
        if (a[i] != b[i]) {
            
            return 0; // Memory differs
        }
    }
    return 1; // Memory is the same
}

// Test DMA functionality
int test_dma() {
    uart0_puts("Testing DMA...\n");

    // Test data
    const unsigned int src_data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    unsigned int dest_data[10]; // Initialize destination array with zeros

    // Perform DMA operation (assuming size is in bytes)
    do_dma(dest_data, src_data, sizeof(src_data));
    uart0_puts("AFter do_dma");
    // Verify the copy
    if (compare_memory(src_data, dest_data, sizeof(src_data)/sizeof(unsigned int))) {
        uart0_puts("\nDMA Test PassedVVVV.\n");
        return 1;

    } else {
        uart0_puts("\nDMA Test Failed.\n");
        return 0;
    }
}