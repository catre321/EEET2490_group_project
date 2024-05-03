// ----------------------------------- framebf.h -------------------------------------
#define SCREEN_PYS_WIDTH 1024
#define SCREEN_PYS_HEIGHT 768
#define SCREEN_VIR_WIDTH 1024
#define SCREEN_VIR_HEIGHT 768

void framebf_init();
void draw_pixel_ARGB32(int x, int y, unsigned int attr);
void draw_rect_ARGB32(int x1, int y1, int x2, int y2, unsigned int attr, int fill);
void draw_string(int x, int y, char *s, unsigned char attr);
int draw_pixel(int x, int y, unsigned char attr);
int draw_char(unsigned char ch, int x, int y, unsigned char color);

