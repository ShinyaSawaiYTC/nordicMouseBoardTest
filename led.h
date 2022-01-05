#ifndef LED_H
#define LED_H

#include "stdint.h"

typedef struct{
    uint16_t r;
    uint16_t g;
    uint16_t b;
}rgb_pwm_t;
void led_pwm_start();
void led_pwm_stop();
void led_pwm_set(rgb_pwm_t *seq);
void led_init();
void led_in_timer_int();

#endif