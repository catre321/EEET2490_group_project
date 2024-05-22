// ----------------------------------- framebf.h -------------------------------------
#define SCREEN_PYS_WIDTH 1280
#define SCREEN_PYS_HEIGHT 720
#define SCREEN_VIR_WIDTH 1280
#define SCREEN_VIR_HEIGHT 720

void framebf_init(int phy_width, int phy_height, int vir_width, int vir_height);
void frambf_release();
void get_resolution_fb_VCmem();
void dma_init();
void set_virtual_offset(int x, int y); 
void clear_fb();
void clear_back_buffer();
void do_dma(void *dest,const void *src, unsigned int total);
void draw_pixel_ARGB32(int x, int y, unsigned int attr);
void draw_rect_ARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill);
void draw_icon(int x, int y, int w, int h, const unsigned int *image);
void draw_iconv2(int x, int y, int w, int h, const unsigned int *image);

void drawChar(unsigned char ch, int x, int y, unsigned int color, int zoom);
int drawString(int x, int y, char *str, unsigned int color, int zoom);

void draw_image(int startX, int startY, int endX, int endY, const unsigned int *imgArray);
void draw_imagev2(const unsigned int *imgArray);
void copy_back_buffer_to_fb();
void timer_init();
// void test_dma();
