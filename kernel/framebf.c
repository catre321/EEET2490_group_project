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
volatile unsigned char *fb;
// Back_buffer to prevent screen flickering
volatile unsigned char back_buffer[1920*1080*4];

unsigned int cur_val_1 = 0;
// Frame buffer size in Byte
unsigned int frame_buffer_size;
// Current screen virtual width
unsigned int curr_virtual_width;
// Current screen virtual height
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

        /*
            Not using the size given by the mailbox bcs in 1920x1080 its give size bigger than theory calculation
            1920x1080x4 = 8294400. Real board is 8355840 bytes
            Might memory violation when using DMA. Therefor using calculation base on real size.
            Real board is Pi4, Pi3 not tested.  
        */ 

        // frame_buffer_size = mBuf[29];
        frame_buffer_size = vir_height * vir_width * COLOR_DEPTH / 8;

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

// Release the current ultilize framebuffer
void frambf_release(){
    mBuf[0] = 6 * 4; // Length of message in bytes
    mBuf[1] = MBOX_REQUEST;
    
    mBuf[2] = MBOX_TAG_RELEASEBUFFER;  // Release the framebuffer
    mBuf[3] = 0;                  
    mBuf[4] = 0;                  // REQUEST CODE = 0

    mBuf[5] = MBOX_TAG_LAST;
    // Call Mailbox
    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)) {
        uart0_puts("Release the framebuffer successfully\n");
    } else {
        uart0_puts("Unable to release the frambuffer\n");
    }
}

// Get current screen physical and virtual resolution
void get_resolution_fb_VCmem(){
    mBuf[0] = 18 * 4; // Length of message in bytes
    mBuf[1] = MBOX_REQUEST;
    
    mBuf[2] = MBOX_TAG_GETPHYDISPLAYWH;  // Get physical (display) width/height
    mBuf[3] = 8;                  
    mBuf[4] = 0;                  // REQUEST CODE = 0
    mBuf[5] = 0;                    // Width
    mBuf[6] = 0;                    // Height

    mBuf[7] = MBOX_TAG_GETVIRBUFWH;  // Get virtual (buffer) width/height
    mBuf[8] = 8;                  
    mBuf[9] = 0;                  // REQUEST CODE = 0
    mBuf[10] = 0;                    // Width
    mBuf[11] = 0;                    // Height

    mBuf[12] = MBOX_TAG_GETVCMEMORY;  // Get virtual (buffer) width/height
    mBuf[13] = 8;                  
    mBuf[14] = 0;                  // REQUEST CODE = 0
    mBuf[15] = 0;                    // Base address in bytes
    mBuf[16] = 0;                    // Size in bytes   

    mBuf[17] = MBOX_TAG_LAST;

    // Call Mailbox
    if (mbox_call(ADDR(mBuf), MBOX_CH_PROP)) {
        uart0_puts("Receive resolution successfully!\n");
        uart0_puts("Physical (display) width/height: ");
        uart0_dec(mBuf[5]); uart0_puts(" "); uart0_dec(mBuf[6]); uart0_puts("\n");
        uart0_puts("Virtual (buffer) width/height");
        uart0_dec(mBuf[10]); uart0_puts(" "); uart0_dec(mBuf[11]); uart0_puts("\n");
        uart0_puts("Frame buffer address and size:");
        uart0_hex(mBuf[15]); uart0_puts(" "); uart0_dec(mBuf[16]); uart0_puts("\n");
    } else {
        uart0_puts("Unable to release the frambuffer\n");
    }
}

void dma_init(){
    dma = dma_open_channel(5);

    uart0_puts("DMA CHANNEL: ");
    uart0_dec(dma->channel);
    uart0_puts("\n");

    test_dma();
}

// Timer and counter to get current ticks
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

// Call DMA to start memory transfer with default brust_length = 14 words
void do_dma(void *dest,const void *src, unsigned int total) {
    // unsigned int ms_start = timer_get_ticks() / 1000;

        dma_setup_mem_copy(dma, dest, src, total, 7);
        
        dma_start(dma);

        dma_wait(dma);

    // unsigned int ms_end = timer_get_ticks() / 1000;
    // uart0_dec((ms_end - ms_start));
}

// Set offset of the framebuffer
void set_virtual_offset(int x, int y) {
    // mbox to set virtual offset to (0, SCREENHEIGHT)
    mBuf[0] = 8 * 4;  // Length of message in bytes
    mBuf[1] = MBOX_REQUEST;

    mBuf[2] = MBOX_TAG_SETVIRTOFF;    // Set virtual offset
    mBuf[3] = 8;                      
    mBuf[4] = 0;                      // REQUEST CODE = 0
    mBuf[5] = x;                      // x offset
    mBuf[6] = y;                      // y offset

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

// Set all value in framebuffer to 0 using DMA
void clear_fb(){
    uart0_puts("Sizeof full_black: ");
    uart0_dec(sizeof(full_black));
    uart0_puts("\n");

    do_dma((void*)fb, full_black, frame_buffer_size);
}

// Set all value in back_buffer to 0 using DMA
void clear_back_buffer(){
    do_dma((void*)back_buffer, full_black, frame_buffer_size);
}

void draw_pixel_ARGB32(int x, int y, unsigned int attr)
{
    int offs = (y * pitch) + (COLOR_DEPTH / 8 * x);
    /* //Access and assign each byte
     *(fb + offs ) = (attr >> 0 ) & 0xFF; //BLUE
     *(fb + offs + 1) = (attr >> 8 ) & 0xFF; //GREEN
     *(fb + offs + 2) = (attr >> 16) & 0xFF; //RED
     *(fb + offs + 3) = (attr >> 24) & 0xFF; //ALPHA
     */
    // Access 32-bit together
    *((unsigned int *)(fb + offs)) = attr;
}

/* Functions to display text on the screen */
// NOTE: zoom = 0 will not display the character
void drawChar(unsigned char ch, int x, int y, unsigned int color, int zoom)
{
    unsigned char *glyph = (unsigned char *)&font + (ch < FONT_NUMGLYPHS ? ch : 0) * FONT_BPG;

    for (int i = 1; i <= (FONT_HEIGHT*zoom); i++) {
		for (int j = 0; j< (FONT_WIDTH*zoom); j++) {
			unsigned char mask = 1 << (j/zoom);
            if (*glyph & mask) { //only draw pixels belong to the character glyph
			    draw_pixel_ARGB32(x + j, y + i, color);
            }
		}
		glyph += (i % zoom) ? 0 : FONT_BPL;
    }
}

/* Functions to display string on the screen and return current y coordinate */
// NOTE: zoom = 0 will not display the character
int drawString(int x, int y, char *str, unsigned int color, int zoom)
{
    while (*str) {
        if (*str == '\r') {
            x = 0;
        } else if (*str == '\n') {
            x = 0; 
			y += (FONT_HEIGHT*zoom);
        } else {
            drawChar(*str, x, y, color, zoom);
            x += (FONT_WIDTH*zoom);
        }
        str++;
    }
    return y;
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

// Draw a small icon using CPU v1
void draw_icon(int x, int y, int w, int h, const unsigned int *image)
{
    for (int i = x, cnt_w = 0; cnt_w < w; i++, cnt_w++)
    {
        for (int j = y, cnt_h = 0; cnt_h < h; j++, cnt_h++)
        {
            // if(image[cnt_w + cnt_h * w] != 0x00ffffff){
                draw_pixel_ARGB32(i, j, image[cnt_w + cnt_h * w]);
            // }
        }
    }
}

// ultilizing DMA to print line of Pixel vertically and using back_buffer
void draw_iconv2(int x, int y, int w, int h, const unsigned int *image)
{
    for (int i = 0; i < h; i++) {
        // Calculate the source and destination addresses for this row
        const unsigned int *src = image + i * w;
        volatile unsigned char* dest = back_buffer + (y + i) * pitch + x * (COLOR_DEPTH / 8);

        // Set up a DMA transfer from src to dest
        do_dma((void*)dest, src, w * (COLOR_DEPTH / 8));

        // Wait for the DMA transfer to complete
        // dma_wait_for_completion();
    }
}

// draw image v1 using CPU
void draw_image(int startX, int startY, int endX, int endY, const unsigned int *imgArray){
    // uart0_puts("draw_image CALLED");
    unsigned int pixel = 0;
    for (int y = startY; y < endY; y++) {
        for (int x = startX; x < endX; x++) {
            draw_pixel_ARGB32(x, y, imgArray[pixel]);
            pixel++;
        }
    }
}

// draw image v2 using DMA and back_buffer to avoid screen flickering
void draw_imagev2(const unsigned int *imgArray){
    // uart0_puts("draw_image v2 CALLED");
    // do_dma(back_buffer, full_black, frame_buffer_size);
    clear_back_buffer();
    do_dma((void*)back_buffer, imgArray, frame_buffer_size);
    do_dma((void*)fb, (void*)back_buffer, frame_buffer_size);
}

// Using DMA to copy back_buffer to framebuffer
void copy_back_buffer_to_fb(){
    do_dma((void*)fb, (void*)back_buffer, frame_buffer_size);
}

// // Test DMA functionality
// int compare_memory(const unsigned int *a, const unsigned int *b, unsigned int size) {
//     for (unsigned int i = 0; i < size; i++) {
//         uart0_dec(a[i]);
//         uart0_puts("-");
//         uart0_dec(b[i]);
//         uart0_puts(" ");
//         if (a[i] != b[i]) {
            
//             return 0; // Memory differs
//         }
//     }
//     return 1; // Memory is the same
// }

// // Test DMA functionality
// void test_dma() {
//     uart0_puts("Testing DMA...\n");

//     // Test data
//     const unsigned int src_data[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
//     unsigned int dest_data[10]; // Initialize destination array with zeros

//     // Perform DMA operation (assuming size is in bytes)
//     do_dma(dest_data[0], src_data[0], sizeof(src_data));
//     uart0_puts("AFter do_dma");
//     // Verify the copy
//     if (compare_memory(src_data, dest_data, sizeof(src_data)/sizeof(unsigned int))) {
//         uart0_puts("\nDMA Test PassedVVVV.\n");

//     } else {
//         uart0_puts("\nDMA Test Failed.\n");
//     }
// }