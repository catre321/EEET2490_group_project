// ----------------------------------- framebf.h -------------------------------------
#define SCREEN_PYS_WIDTH 1280
#define SCREEN_PYS_HEIGHT 720
#define SCREEN_VIR_WIDTH 1920
#define SCREEN_VIR_HEIGHT 1080

void framebf_init();
void set_virtual_offset(int x, int y); 
void draw_pixel_ARGB32(int x, int y, unsigned int attr);
void draw_rect_ARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill);
void draw_string(int x, int y, char *s, unsigned char attr);
int draw_pixel(int x, int y, unsigned char attr);
int draw_char(unsigned char ch, int x, int y, unsigned char color);
void draw_image(int startX, int startY, int endX, int endY, const unsigned int *imgArray);

