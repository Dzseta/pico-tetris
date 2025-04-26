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

// board
int board[20][20];

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

    while(true) {
        // ################################################# Game logic ##############################################################
        uint64_t start = time_us_64();
        hagl_flush(display);
        busy_wait_until(start + 1000000);
        
        // initialise variables
        srand(start);

        while (true) {
            // Game logic
            // start frame timer
            uint64_t start = time_us_64();

            // read input
            if(!gpio_get(JOY_UP)) {
                
            } else if(!gpio_get(JOY_DOWN)) {
                
            } else if(!gpio_get(JOY_LEFT)) {
                
            } else if(!gpio_get(JOY_RIGHT)) {
                
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
