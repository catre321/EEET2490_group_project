#ifndef GAME_H
#define GAME_H

// #include "assets.h"

#define FPS 30
#define USPF (1000000 / FPS)  // Microseconds per frame
#define MAXBULLET 200
#define MAXPOWERUP 1
#define MAXENEMY 20

#define PADDING 10

#define LIVES_UI_X 10
#define LIVES_UI_Y 20
#define LIVES_PADDING 10

#define SCORE_UI_X SCREEN_WIDTH - 250
#define SCORE_UI_Y 20


typedef struct {
    const unsigned int* image;
    int x;
    int y;
    int width;
    int height;
    int x_moved_pixel;
    int y_moved_pixel;
    int is_active;
    int flash_counter;
} game_object;



// Use what u like, or create new functions if u dont like the current one
void game_logic(int is_hard_mode);
void shoot_blue_bullet(game_object* shoot_from);

#endif