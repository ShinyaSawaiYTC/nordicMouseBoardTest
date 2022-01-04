
#ifndef BTN_ENC_H
#define BTN_ENC_H

typedef enum {
    ENC_NO_CHANGE,
    ENC_CW,
    ENC_CCW,
}encoder_state_t;
typedef enum {
    BTN_NO_CHANGE,
    BTN_PUSH,
    BTN_RELEASE,
}button_state_t;


void gpio_init(void);
void timer_init();


#endif