// !gpio_get(KEY_A)

#include <stdio.h>
#include <math.h>

#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define KEY_A 15
#define KEY_B 17
#define KEY_X 19
#define KEY_Y 21

#define JOY_UP 2
#define JOY_DOWN 18
#define JOY_LEFT 16
#define JOY_RIGHT 20
#define JOY_CENTRE 3

static inline void set_panel_gpio(uint gpio_pin_number) {
    gpio_init(gpio_pin_number);
    gpio_pull_up(gpio_pin_number);
    gpio_set_dir(gpio_pin_number, GPIO_IN);
}

static inline void buttons_init() {
    // Waveshare 1.3 inch LCD Display Module, Joypad and 4 buttons
    set_panel_gpio(KEY_A);
    set_panel_gpio(KEY_B);
    set_panel_gpio(KEY_X);
    set_panel_gpio(KEY_Y);

    set_panel_gpio(JOY_UP);
    set_panel_gpio(JOY_DOWN);
    set_panel_gpio(JOY_LEFT);
    set_panel_gpio(JOY_RIGHT);
    set_panel_gpio(JOY_CENTRE);
}

