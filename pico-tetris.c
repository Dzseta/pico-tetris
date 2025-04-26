#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include <hardware/clocks.h>
#include <hardware/gpio.h>
#include <sys/time.h>

// Display libraries
#include <hagl_hal.h>
#include <hagl.h>
#include <fps.h>
#include <font6x9.h>

// Buttons
#include "buttons.h"
// Colors
#include "colors.h"
// tetronimos
#include "tetronimo.c"

// ########################## VARIABLES ###############################

// fps counter
volatile bool fps_flag = false;
static fps_instance_t fps;
static const uint64_t US_PER_FRAME_60_FPS = 1000000 / 60;

// message
wchar_t message[32];
// text
wchar_t text[20];

// display
static hagl_backend_t *display;

// timer
uint16_t tetris_timer;
// board
int board[12][20];
// score
int score;
// active squares
int active_sq[8];
// is there active
bool is_active;
// buttons
bool key_a, key_b, joy_left, joy_right; 

// move the active tetronimo
void move_tetronimo() {
    for(int i=3; i>=0; i--) {
        if(active_sq[i*2+1] == 19) {
            is_active = false;
        } else if (board[active_sq[i*2]][active_sq[i*2+1]+1] > 0) {
            is_active = false;
        }
    }

    if(is_active) {
        for(int i=3; i>=0; i--) {
            board[active_sq[i*2]][active_sq[i*2+1]+1] = board[active_sq[i*2]][active_sq[i*2+1]];
            board[active_sq[i*2]][active_sq[i*2+1]] = 0;
            active_sq[i*2+1]++;
        }
    } else {
        for(int i=3; i>=0; i--) {
            board[active_sq[i*2]][active_sq[i*2+1]] = board[active_sq[i*2]][active_sq[i*2+1]] * (-1);
            active_sq[i*2+1]++;
        }
    }
}

// move the active tetronimo left
void move_tetronimo_left() {
    for(int i=0; i<4; i++) {
        if(active_sq[i*2] == 0) {
            return;
        } else if (board[active_sq[i*2]-1][active_sq[i*2+1]] > 0) {
            return;
        }
    }

    for(int i=0; i<4; i++) {
        board[active_sq[i*2]-1][active_sq[i*2+1]] = board[active_sq[i*2]][active_sq[i*2+1]];
        board[active_sq[i*2]][active_sq[i*2+1]] = 0;
        active_sq[i*2]--;
    }
}

// move the active tetronimo right
void move_tetronimo_right() {
    for(int i=3; i>=0; i--) {
        if(active_sq[i*2] == 11) {
            return;
        } else if (board[active_sq[i*2]+1][active_sq[i*2+1]] > 0) {
            return;
        }
    }

    for(int i=3; i>=0; i--) {
        board[active_sq[i*2]+1][active_sq[i*2+1]] = board[active_sq[i*2]][active_sq[i*2+1]];
        board[active_sq[i*2]][active_sq[i*2+1]] = 0;
        active_sq[i*2]++;
    }
}

// check rows
void check_rows() {
    uint8_t sq_counter = 0;
    for(int i=19; i>=0; i--) {
        for(int j=0; j<12; j++) {
            if(board[j][i] > 0) sq_counter++;
        }
        if (sq_counter == 12) {
            for(int k=0; k<12; k++) board[k][i] = 0;
            for(int k=i; k>0; k--) {
                for(int l=0; l<12; l++) {
                    board[l][k] = board[l][k-1];
                }
            }
            score++;
            i--;
        }
        sq_counter = 0;
    }
}

// draw the board
void draw_board() {
    for(int i=0; i<12; i++) {
        for(int j=0; j<20; j++) {
            if(board[i][j] == 0) hagl_fill_rectangle_xywh(display, i*12, j*12, 12, 12, color_black);
            else if(board[i][j] == 1 || board[i][j] == -1) hagl_fill_rectangle_xywh(display, i*12, j*12, 12, 12, color_lightblue);
            else if(board[i][j] == 2 || board[i][j] == -2) hagl_fill_rectangle_xywh(display, i*12, j*12, 12, 12, color_blue);
            else if(board[i][j] == 3 || board[i][j] == -3) hagl_fill_rectangle_xywh(display, i*12, j*12, 12, 12, color_orange);
            else if(board[i][j] == 4 || board[i][j] == -4) hagl_fill_rectangle_xywh(display, i*12, j*12, 12, 12, color_yellow);
            else if(board[i][j] == 5 || board[i][j] == -5) hagl_fill_rectangle_xywh(display, i*12, j*12, 12, 12, color_lightgreen);
            else if(board[i][j] == 6 || board[i][j] == -6) hagl_fill_rectangle_xywh(display, i*12, j*12, 12, 12, color_purple);
            else if(board[i][j] == 7 || board[i][j] == -7) hagl_fill_rectangle_xywh(display, i*12, j*12, 12, 12, color_red);
            hagl_draw_rectangle_xywh(display, i*12, j*12, 12, 12, color_black);
        }
    }
    hagl_fill_rectangle_xywh(display, 145, 0, 95, 240, color_darkgray);
}

int main()
{
    // Init I/O
    stdio_init_all();
    // Init display
    display = hagl_init();
    // Clear the display
    hagl_clear(display);
    // Init buttons
    buttons_init();

    // ending
    bool end;

    while(true) {
        // ################################################# Game logic ##############################################################
        uint64_t start = time_us_64();
        hagl_flush(display);
        busy_wait_until(start + 1000000);
        
        // initialise variables
        srand(start);
        end = false;

        // timer
        tetris_timer = 0;
        // score
        score = 0;
        // active squares
        for(int i=0; i<8; i++) {
            active_sq[i] = 0;
        }
        is_active = false;
        // clear board
        for(int i=0; i<12; i++) {
            for(int j=0; j<20; j++) {
                board[i][j] == 0;
            }
        }
        // clear keys
        key_a = key_b = joy_left = joy_right = false;

        // Draw the board
        draw_board();

        while (true) {
            // Game logic
            // start frame timer
            uint64_t start = time_us_64();
            tetris_timer++;

            // read input
            if(end) {
                if(!gpio_get(KEY_Y)) {
                    if(end) break;
                }
            } else {
                if(!gpio_get(KEY_A)) {
                    key_b = joy_left = joy_right = false;
                    key_a = true;
                } else if(!gpio_get(KEY_B)) {
                    key_a = joy_left = joy_right = false;
                    key_b = true;
                } else if(!gpio_get(JOY_DOWN)) {
                    tetris_timer = 60;
                } else if(!gpio_get(JOY_LEFT)) {
                    key_a = key_b = joy_right = false;
                    joy_left =  true;
                } else if(!gpio_get(JOY_RIGHT)) {
                    key_a = key_b = joy_left = false;
                    joy_right =  true;
                }
            }

            if(tetris_timer%20 == 10) {
                if(joy_left) {
                    move_tetronimo_left();
                    joy_left = false;
                    draw_board();
                } else if(joy_right) {
                    move_tetronimo_right();
                    joy_right = false;
                    draw_board();
                } else if(key_a) {
                    
                } else if(key_b) {
                    
                }
            }

            // move
            if(tetris_timer >= 60 && !end) {
                if(!is_active) {
                    is_active = true;
                    // put in new tetronimo
                    u_int8_t r = rand() % 7;
                    if(board[4][0] != 0 && tetronimo[0+8*r] != 0) {
                        end = true;
                    } else board[4][0] = -tetronimo[0+8*r];
                    if(board[5][0] != 0 && tetronimo[1+8*r] != 0) {
                        end = true;
                    } else board[5][0] = -tetronimo[1+8*r];
                    if(board[6][0] != 0 && tetronimo[2+8*r] != 0) {
                        end = true;
                    } else board[6][0] = -tetronimo[2+8*r];
                    if(board[7][0] != 0 && tetronimo[3+8*r] != 0) {
                        end = true;
                    } else board[7][0] = -tetronimo[3+8*r];
                    if(board[4][1] != 0 && tetronimo[4+8*r] != 0) {
                        end = true;
                    } else board[4][1] = -tetronimo[4+8*r];
                    if(board[5][1] != 0 && tetronimo[5+8*r] != 0) {
                        end = true;
                    } else board[5][1] = -tetronimo[5+8*r];
                    if(board[6][1] != 0 && tetronimo[6+8*r] != 0) {
                        end = true;
                    } else board[6][1] = -tetronimo[6+8*r];
                    if(board[7][1] != 0 && tetronimo[7+8*r] != 0) {
                        end = true;
                    } else board[7][1] = -tetronimo[7+8*r];
                    uint8_t helper = 0;
                    for(int i=0; i<8; i++) {
                        if(tetronimo[i+8*r] >0) {
                            if(i<4) {
                                active_sq[helper] = 4+i;
                                active_sq[helper+1] = 0;
                            } else {
                                active_sq[helper] = i;
                                active_sq[helper+1] = 1;
                            }
                            helper+=2;
                        }
                    }
                } else {
                    move_tetronimo();
                }
                if(!is_active) check_rows();
                // draw board
                draw_board();
                tetris_timer = 0;
            }
        
            // Flush back buffer contents to display
            hagl_flush(display);

            // Update FPS
            fps_update(&fps);

            // Cap to 60 fps
            busy_wait_until(start + US_PER_FRAME_60_FPS);
        }
        start = time_us_64();
        busy_wait_until(start + 500000);
    }
    return 0;
}
