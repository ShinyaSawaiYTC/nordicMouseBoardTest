#include "led.h"

#include "nrf_gpio.h"

#include "boards.h"
rgb_pwm_t pwm = {0,0,0};
typedef enum{
    LED_SC_STATE_RY,
    LED_SC_STATE_YG,
    LED_SC_STATE_GC,
    LED_SC_STATE_CB,
    LED_SC_STATE_BP,
    LED_SC_STATE_PR,
}led_scroll_state_t;
led_scroll_state_t led_scroll_state;
const uint16_t pwm_max = 500;

volatile uint16_t pwm_seq[4];
void init_pwm(){
    NRF_PWM0->PSEL.OUT[0] = LED_R;
    NRF_PWM0->PSEL.OUT[1] = LED_G;
    NRF_PWM0->PSEL.OUT[2] = LED_B;
    NRF_PWM0->MODE = PWM_MODE_UPDOWN_Up << PWM_MODE_UPDOWN_Pos;
    NRF_PWM0->PRESCALER = PWM_PRESCALER_PRESCALER_DIV_16 << PWM_PRESCALER_PRESCALER_Pos;
    NRF_PWM0->COUNTERTOP = 1000 << PWM_COUNTERTOP_COUNTERTOP_Pos ;
    NRF_PWM0->LOOP = 0;
    NRF_PWM0->DECODER = (
        0
        | PWM_DECODER_LOAD_Individual << PWM_DECODER_LOAD_Pos
        | PWM_DECODER_MODE_RefreshCount << PWM_DECODER_MODE_Pos
    );
    NRF_PWM0->SEQ[0].PTR = (uint32_t)&pwm_seq;
    NRF_PWM0->SEQ[0].CNT = 4;//4にしないと正しく反映されない?
    NRF_PWM0->SEQ[0].REFRESH  = 0;
    NRF_PWM0->SEQ[0].ENDDELAY = 0;
}
void led_pwm_start(){
    nrf_gpio_pin_clear(LED_POWER_OFF);
    NRF_PWM0->ENABLE = 1;
}
void led_pwm_stop(){
    nrf_gpio_pin_set(LED_POWER_OFF);
    NRF_PWM0->ENABLE = 0;
}
void led_pwm_set(rgb_pwm_t *pwm){
    pwm_seq[0] = pwm->r | 0x8000;
    pwm_seq[1] = pwm->g | 0x8000;
    pwm_seq[2] = pwm->b | 0x8000;
    NRF_PWM0->TASKS_SEQSTART[0] = 1;
}
void led_init(){
    nrf_gpio_cfg_output(LED_POWER_OFF);
    init_pwm();
    led_pwm_start();
    led_scroll_state = LED_SC_STATE_RY;
}

void led_in_timer_int(){
    switch(led_scroll_state){
        case LED_SC_STATE_RY:
            pwm.r = pwm_max;
            pwm.b = 0;
            pwm.g++;
            if(pwm.g>=pwm_max){
                led_scroll_state = LED_SC_STATE_YG;
            }
            led_pwm_set(&pwm);
        break;
        case LED_SC_STATE_YG:
            pwm.g = pwm_max;
            pwm.b = 0;
            pwm.r--;
            if(pwm.r==0){
                led_scroll_state = LED_SC_STATE_GC;
            }
            led_pwm_set(&pwm);
        break;
        case LED_SC_STATE_GC:
            pwm.r = 0;
            pwm.g = pwm_max;
            pwm.b++;
            if(pwm.b>=pwm_max){
                led_scroll_state = LED_SC_STATE_CB;
            }
            led_pwm_set(&pwm);
        break;
        case LED_SC_STATE_CB:
            pwm.r = 0;
            pwm.b = pwm_max;
            pwm.g--;
            if(pwm.g==0){
                led_scroll_state = LED_SC_STATE_BP;
            }
            led_pwm_set(&pwm);
        break;
        case LED_SC_STATE_BP:
            pwm.g = 0;
            pwm.b = pwm_max;
            pwm.r++;
            if(pwm.r>=pwm_max){
                led_scroll_state = LED_SC_STATE_PR;
            }
            led_pwm_set(&pwm);
        break;
        case LED_SC_STATE_PR:
            pwm.r = pwm_max;
            pwm.g = 0;
            pwm.b--;
            if(pwm.b==0){
                led_scroll_state = LED_SC_STATE_RY;
            }
            led_pwm_set(&pwm);
        break;
    }
}