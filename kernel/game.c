#include "game.h"
#include "../images/games/space_shuttle.h"
#include "../images/games/chicken.h"
#include "../images/games/red_laser.h"
#include "../images/games/blue_laser.h"
#include "../uart/uart0.h"
#include "../images/games/background.h"
#include "../lib/use_func.h"

#include "framebf.h"

#define FPS 30
#define USPF (1000000 / FPS) // Microseconds per frame

#define MAX_BLUE_BULLET 8
#define MAX_RED_BULLET 6
#define NUM_CHICKEN_PER_LINE 6
#define ACTIVE 1
#define DEACTIVATE 0
#define PLAYER_MOVE_PIXEL 20
#define BULLET_MOVE_PIXEL 20
#define CHICKEN_SPACING (SCREEN_PYS_WIDTH / (NUM_CHICKEN_PER_LINE + 1))
#define CHICKEN_START_Y 0
#define COMPARE_VALUE 5
#define SPACE_SHUTTLE_LIVES 5
#define SPACE_SHUTTLE_START_X (SCREEN_PYS_HEIGHT - SPACE_SHUTTLE_HEIGHT)
#define SPACE_SHUTTLE_START_Y ((SCREEN_PYS_WIDTH / 2) - (SPACE_SHUTTLE_WIDTH / 2))

// #define PADDING 10

// #define LIVES_UI_X 10
// #define LIVES_UI_Y 20
// #define LIVES_PADDING 10

// #define SCORE_UI_X SCREEN_WIDTH - 250
// #define SCORE_UI_Y 20

unsigned int no_of_cmd_receive = 0;
int is_hard_mode = 0;
int is_exit_game = 0;
int chicken_lived = 0;
int is_change_direction = 0;
int space_shuttle_live = 0;
int compare_value = 0;

// Game objects
game_object space_shuttle_object;
game_object blue_laser_objects[MAX_BLUE_BULLET];

// The 2nd array of objects is for hard_mode
game_object chicken_objects_1[NUM_CHICKEN_PER_LINE];
game_object chicken_objects_2[NUM_CHICKEN_PER_LINE];
game_object red_laser_objects_1[MAX_RED_BULLET];
game_object red_laser_objects_2[MAX_RED_BULLET];

// check if user typed and get the input
char get_input()
{
    char c = 0;
    while (is_uart0_byte_ready())
    {
        c = uart0_getc(); // Read and discard the character
    }
    return c;
}

// get user input and handle what they press
void handle_input()
{
    // Handle key input
    int is_ack = 0;
    char c = get_input();
    switch (c)
    {
    case 'w':
    case 'W':
        is_ack = 1;
        space_shuttle_object.y_moved_pixel = -PLAYER_MOVE_PIXEL;
        break;
    case 'a':
    case 'A':
        is_ack = 1;
        space_shuttle_object.x_moved_pixel = -PLAYER_MOVE_PIXEL;
        break;
    case 's':
    case 'S':
        is_ack = 1;
        space_shuttle_object.y_moved_pixel = PLAYER_MOVE_PIXEL;
        break;
    case 'd':
    case 'D':
        is_ack = 1;
        space_shuttle_object.x_moved_pixel = PLAYER_MOVE_PIXEL;
        break;
    case 'j':
    case 'J':
        is_ack = 1;
        shoot_blue_bullet(&space_shuttle_object);
        break;
    // Ctrl+z exit code
    case 26:
        is_exit_game = 1;
        break;
    default:
        uart0_puts("NAK input\n");
        break;
    }
    
    if(is_ack){
        no_of_cmd_receive++;
        uart0_puts("ACK input, No of commands receive: ");
        uart0_dec(no_of_cmd_receive);
        uart0_puts("\n");
    }
}

void create_new_object(game_object *object, int x, int y, int width, int height, int x_moved_pixel, int y_moved_pixel, int is_active, int flash_counter, const unsigned int *img)
{
    object->image = img;
    object->x = x;
    object->y = y;
    object->width = width;
    object->height = height;
    object->x_moved_pixel = x_moved_pixel;
    object->y_moved_pixel = y_moved_pixel;
    object->is_active = is_active;
    object->flash_counter = flash_counter;
}

// draw the object if is Active
void draw_game_object(game_object *object)
{
    // draw_iconv2(object->x, object->y, object->width, object->height, full_black);
    // case object deactivate or terminated
    if (!object->is_active)
    {
        return;
    }
    draw_iconv2(object->x, object->y, object->width, object->height, object->image);
}

// check collision between 2 object
int is_collision(game_object *obj1, game_object *obj2)
{
    // Check if obj1 is to the right of obj2
    if (obj1->x > obj2->x + obj2->width)
    {
        return 0;
    }
    // Check if obj1 is to the left of obj2
    if (obj1->x + obj1->width < obj2->x)
    {
        return 0;
    }
    // Check if obj1 is below obj2
    if (obj1->y > obj2->y + obj2->height)
    {
        return 0;
    }
    // Check if obj1 is above obj2
    if (obj1->y + obj1->height < obj2->y)
    {
        return 0;
    }
    // If none of the above are true, the objects must be intersecting
    return 1;
}

// check if the bullet hit the object or not
void check_bullet_hit(game_object *bullets, game_object *objects_being_checked, int max_bullet, int max_object)
{
    for (int i = 0; i < max_object; i++)
    {
        for (int j = 0; j < max_bullet; j++)
        {
            if (objects_being_checked[i].is_active == ACTIVE &&
                bullets[j].is_active == ACTIVE &&
                is_collision(&objects_being_checked[i], &bullets[j]))
            {
                uart0_puts("Bullet hitted\n");
                // Bullet has hit the chicken, mark chicken as dead
                objects_being_checked[i].is_active = DEACTIVATE;
                // Deactivate the bullet
                bullets[j].is_active = DEACTIVATE;
            }
        }
    }
}

// Check collision of blue_bullet to chicken(enemy)
void check_blue_bullet_hit(game_object *chicken_objects)
{
    for (int i = 0; i < NUM_CHICKEN_PER_LINE; i++)
    {
        for (int j = 0; j < MAX_BLUE_BULLET; j++)
        {
            if (chicken_objects[i].is_active == ACTIVE &&
                blue_laser_objects[j].is_active == ACTIVE &&
                is_collision(&chicken_objects[i], &blue_laser_objects[j]))
            {
                uart0_puts("Bullet hitted chicken\n");
                chicken_lived--;
                uart0_puts("No of chicken lived: ");
                uart0_dec(chicken_lived);
                uart0_puts("\n");

                // Bullet has hit the chicken, mark chicken as dead
                chicken_objects[i].is_active = DEACTIVATE;
                // Deactivate the bullet
                blue_laser_objects[j].is_active = DEACTIVATE;
            }
        }
    }
}

// Check collision of red_bullet to space_shuttle(player)
void check_red_bullet_hit(game_object *red_laser_objects)
{
    for (int i = 0; i < MAX_RED_BULLET; i++)
    {
        if (is_collision(&space_shuttle_object, &red_laser_objects[i]))
        {
            // Deactivate the bullet
            red_laser_objects[i].is_active = DEACTIVATE;

            // Immortal state after being hit
            if (space_shuttle_object.flash_counter > 0)
            {
                continue;
            }
            uart0_puts("Bullet hitted ship\n");

            // Bullet has hit the space_shuttle, reduce lives
            space_shuttle_live--;
            uart0_puts("Shuttle remaining lives: ");
            uart0_dec(space_shuttle_live);
            uart0_puts("\n");
            // init flashing, imortal state
            space_shuttle_object.flash_counter = 6;
        }
    }
}

// Update XY coordinate of space_shuttle
void update_space_shuttle(game_object *object)
{
    // Calculate the new x position
    int new_x = object->x + object->x_moved_pixel;

    // Check if the new x position is within the screen bounds and there is moved input
    if (new_x >= 0 && new_x != object->x && new_x + object->width <= SCREEN_PYS_WIDTH)
    {
        // If it is, update the x position
        object->x = new_x;
        object->x_moved_pixel = 0;
    }

    // Calculate the new y position
    int new_y = object->y + object->y_moved_pixel;

    // Check if the new y position is within the screen bounds and there is moved input
    if (new_y >= 0 && new_y != object->y && new_y + object->height <= SCREEN_PYS_HEIGHT)
    {
        // If it is, update the y position
        object->y = new_y;
        object->y_moved_pixel = 0;
    }

    // If flash_counter is greater than 0, decrement it and draw the space shuttle only if it's even
    if (object->flash_counter > 0)
    {
        object->flash_counter--;
        if (object->flash_counter % 2 == 0)
            object->is_active = ACTIVE;
        else
            object->is_active = DEACTIVATE;
    }
    else
        object->is_active = ACTIVE;

    draw_game_object(&space_shuttle_object);
}

// Update X coordinate of the chicken (make it move left-right)
void update_chicken(game_object *object)
{
    // Calculate the new x position
    int new_x = object->x + object->x_moved_pixel;
    // If the chicken has reached the edge of the screen, change direction
    // If the chicken has reached the edge of the screen, change direction
    if (new_x < 0 || new_x + object->width > SCREEN_PYS_WIDTH)
    {
        is_change_direction = 1;
        // object->x_moved_pixel = -(object->x_moved_pixel);
    }
    else
    {
        // If it hasn't, update the x position
        object->x = new_x;
    }
    // // direction == 1 mean left -> right
    // if(direction){
    //     object->x_moved_pixel =
    // }
    draw_game_object(object);
}

void init_blue_lasers()
{
    for (int i = 0; i < MAX_BLUE_BULLET; i++)
    {
        blue_laser_objects[i].x = 0;
        blue_laser_objects[i].y = 0;
        blue_laser_objects[i].width = BLUE_LASER_WIDTH;
        blue_laser_objects[i].height = BLUE_LASER_WIDTH;
        blue_laser_objects[i].x_moved_pixel = 0;
        blue_laser_objects[i].y_moved_pixel = -BULLET_MOVE_PIXEL;
        blue_laser_objects[i].is_active = DEACTIVATE;
        blue_laser_objects[i].image = blue_laser;
    }
}

void init_red_lasers()
{
    for (int i = 0; i < MAX_RED_BULLET; i++)
    {
        red_laser_objects_1[i].x = 0;
        red_laser_objects_1[i].y = 0;
        red_laser_objects_1[i].width = RED_LASER_WIDTH;
        red_laser_objects_1[i].height = RED_LASER_WIDTH;
        red_laser_objects_1[i].x_moved_pixel = 0;
        red_laser_objects_1[i].y_moved_pixel = BULLET_MOVE_PIXEL;
        red_laser_objects_1[i].is_active = DEACTIVATE;
        red_laser_objects_1[i].image = red_laser;
    }

    if (is_hard_mode)
    {
        for (int i = 0; i < MAX_RED_BULLET; i++)
        {
            red_laser_objects_2[i].x = 0;
            red_laser_objects_2[i].y = 0;
            red_laser_objects_2[i].width = RED_LASER_WIDTH;
            red_laser_objects_2[i].height = RED_LASER_WIDTH;
            red_laser_objects_2[i].x_moved_pixel = 0;
            red_laser_objects_2[i].y_moved_pixel = BULLET_MOVE_PIXEL;
            red_laser_objects_2[i].is_active = DEACTIVATE;
            red_laser_objects_2[i].image = red_laser;
        }
    }
}

// Scan to get inactive bullet and set it to active to show
void shoot_blue_bullet(game_object *shoot_from)
{
    // Find an unused bullet
    for (int i = 0; i < MAX_BLUE_BULLET; i++)
    {
        if (!blue_laser_objects[i].is_active)
        {
            // blue_laser_objects[i].image = blue_laser;
            // Set the bullet's position to the middle of the space shuttle
            blue_laser_objects[i].x = (shoot_from->x + shoot_from->width / 2) - blue_laser_objects[i].width / 2;
            blue_laser_objects[i].y = shoot_from->y - blue_laser_objects[i].height;

            // Set the bullet to active
            blue_laser_objects[i].is_active = ACTIVE;

            // Only shoot one bullet at a time
            break;
        }
    }
}

// Scan to get inactive bullet and set it to active to show
void shoot_red_bullet(game_object *red_laser_objects, game_object *shoot_from)
{
    // Find an unused bullet
    for (int i = 0; i < MAX_RED_BULLET; i++)
    {
        if (!red_laser_objects[i].is_active)
        {
            // Set the bullet's position to the middle of the space shuttle
            red_laser_objects[i].x = (shoot_from->x + shoot_from->width / 2) - red_laser_objects[i].width / 2;
            red_laser_objects[i].y = shoot_from->height;

            // Set the bullet to active
            red_laser_objects[i].is_active = ACTIVE;

            // Only shoot one bullet at a time
            break;
        }
    }
}

// Ultilize random number to get randomess if number > compare_value = shoot
void chicken_shoot_randomly(game_object *red_laser_objects, game_object *chicken_objects)
{
    int adjusted_compare_value = compare_value * NUM_CHICKEN_PER_LINE / chicken_lived;
    for (int i = 0; i < NUM_CHICKEN_PER_LINE; i++)
    {
        int random_number = randomRange(1, 60);

        if (random_number > adjusted_compare_value)
        {
            continue;
        }
        if (chicken_objects[i].is_active == ACTIVE)
        {
            shoot_red_bullet(red_laser_objects, &chicken_objects[i]);
        }
    }
}

// Update Y coordinate of the bullet
void update_bullets(game_object *bullets, int max_bullet)
{
    for (int i = 0; i < max_bullet; i++)
    {
        if (bullets[i].is_active)
        {
            // Update the bullet's position
            bullets[i].y += bullets[i].y_moved_pixel;

            // If the bullet is off the screen, set it to inactive
            if (bullets[i].y <= 0 || (bullets[i].y + bullets[i].height) >= SCREEN_PYS_HEIGHT)
            {
                bullets[i].is_active = DEACTIVATE;
            }
            draw_game_object(&bullets[i]);
        }
    }
}

void create_chickens()
{
    for (int i = 0; i < NUM_CHICKEN_PER_LINE; i++)
    {
        int chicken_x = ((i + 1) * CHICKEN_SPACING) - (CHICKEN_WIDTH / 2);
        create_new_object(&chicken_objects_1[i], chicken_x, CHICKEN_START_Y, CHICKEN_WIDTH, CHICKEN_HEIGHT, 5, 0, ACTIVE, 0, chicken);
    }

    if (is_hard_mode)
    {
        for (int i = 0; i < NUM_CHICKEN_PER_LINE; i++)
        {
            int chicken_x = ((i + 1) * CHICKEN_SPACING) - (CHICKEN_WIDTH / 2);
            create_new_object(&chicken_objects_2[i], chicken_x, (CHICKEN_START_Y + CHICKEN_HEIGHT + 10), CHICKEN_WIDTH, CHICKEN_HEIGHT, -5, 0, ACTIVE, 0, chicken);
        }
    }
}

void init_game()
{
    no_of_cmd_receive = 0;
    is_exit_game = 0;
    space_shuttle_live = SPACE_SHUTTLE_LIVES;
    compare_value = COMPARE_VALUE;
    if (is_hard_mode)
    {
        chicken_lived = NUM_CHICKEN_PER_LINE * 2;
        uart0_puts("!!! THIS IS HARD MODE !!!\n");
    }
    else
    {
        chicken_lived = NUM_CHICKEN_PER_LINE;
        uart0_puts("!!! THIS IS NORMAL MODE !!!\n");
    }

    uart0_puts("Shuttle remaining lives: ");
    uart0_dec(space_shuttle_live);
    uart0_puts("\n");
    uart0_puts("No of chicken lived: ");
    uart0_dec(chicken_lived);
    uart0_puts("\n");
}

void game_logic(int _is_hard_mode)
{
    is_hard_mode = _is_hard_mode;

    // Game initilization value
    init_game();
    // Init game objects
    create_new_object(&space_shuttle_object, SPACE_SHUTTLE_START_X, SPACE_SHUTTLE_START_Y, SPACE_SHUTTLE_WIDTH, SPACE_SHUTTLE_HEIGHT, 0, 0, ACTIVE, 0, space_shuttle);
    init_blue_lasers();
    init_red_lasers();
    create_chickens();

    // Game big loop
    while (1)
    {
        // space_shuttle_live = 1;
        // if user hit Ctrl+z or run out of live terminate the game
        if (is_exit_game || space_shuttle_live == 0)
        {
            uart0_puts("\nGame Over!!!\n\n");
            clear_back_buffer();
            clear_fb();
            return;
        }
        if (chicken_lived == 0)
        {
            uart0_puts("\nYOU WON CONGRATULATION!!!\n\n");
            clear_back_buffer();
            clear_fb();
            return;
        }
        // create_new_object(&space_shuttle_object, 100, 100, SPACE_SHUTTLE_WIDTH, SPACE_SHUTTLE_HEIGHT, 30, 30, space_shuttle);
        // create_new_object(&space_shuttle_object, 100, 100, SPACE_SHUTTLE_WIDTH, SPACE_SHUTTLE_HEIGHT, 30, 30, space_shuttle);
        // create_new_object(&space_shuttle_object, 100, 100, SPACE_SHUTTLE_WIDTH, SPACE_SHUTTLE_HEIGHT, 30, 30, space_shuttle);
        // create_new_object(&space_shuttle_object, 100, 100, SPACE_SHUTTLE_WIDTH, SPACE_SHUTTLE_HEIGHT, 30, 30, space_shuttle);

        // draw_iconv2(50, 50, SPACE_SHUTTLE_WIDTH, SPACE_SHUTTLE_HEIGHT, space_shuttle);
        // draw_iconv2(100, 100, SPACE_SHUTTLE_WIDTH, SPACE_SHUTTLE_HEIGHT, space_shuttle);
        // draw_iconv2(300, 300, CHICKEN_WIDTH, CHICKEN_HEIGHT, chicken);
        // draw_iconv2(500, 500, RED_LASER_WIDTH, RED_LASER_HEIGHT, red_laser);
        // draw_iconv2(600, 600, BLUE_LASER_WIDTH, BLUE_LASER_HEIGHT, blue_laser);

        handle_input();

        // Make chicken shoot
        chicken_shoot_randomly(red_laser_objects_1, chicken_objects_1);
        // check if the space_shuttle buttlet hit the chickens vice versa
        check_blue_bullet_hit(chicken_objects_1);
        check_red_bullet_hit(red_laser_objects_1);

        // Clear and re-draw all objects of the game
        clear_back_buffer();

        draw_iconv2(0, 0, 1280, 720, background);
        update_bullets(blue_laser_objects, MAX_BLUE_BULLET);
        update_bullets(red_laser_objects_1, MAX_RED_BULLET);

        update_space_shuttle(&space_shuttle_object);
        for (int i = 0; i < NUM_CHICKEN_PER_LINE; i++)
        {
            update_chicken(&chicken_objects_1[i]);
        }

        // hard mode have 2 line of chicken
        if (is_hard_mode)
        {
            // Make chicken shoot
            chicken_shoot_randomly(red_laser_objects_2, chicken_objects_2);
            // check if the space_shuttle buttlet hit the chickens vice versa
            check_blue_bullet_hit(chicken_objects_2);
            check_red_bullet_hit(red_laser_objects_2);

            // Re-draw the chicken and bullet
            update_bullets(red_laser_objects_2, MAX_RED_BULLET);
            for (int i = 0; i < NUM_CHICKEN_PER_LINE; i++)
            {
                update_chicken(&chicken_objects_2[i]);
            }
        }
        copy_back_buffer_to_fb();

        // Handle change_direction
        if (is_change_direction)
        {
            for (int i = 0; i < NUM_CHICKEN_PER_LINE; i++)
            {
                chicken_objects_1[i].x_moved_pixel = -(chicken_objects_1[i].x_moved_pixel);
            }
            if (is_hard_mode)
            {
                for (int i = 0; i < NUM_CHICKEN_PER_LINE; i++)
                {
                    chicken_objects_2[i].x_moved_pixel = -(chicken_objects_2[i].x_moved_pixel);
                }
            }
            is_change_direction = 0;
        }

// Because of DMA in real pi4 board is ~2ms fast and emulator is ~30ms fast
#ifdef RPI4
        wait_msec(50); // approx 30fps
#endif
        // wait_msec(33); // approx 30fps
    }
}
