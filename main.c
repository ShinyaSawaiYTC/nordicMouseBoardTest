
#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "nordic_common.h"
#include "boards.h"

#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "nrf_drv_timer.h"

void button_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
    switch (pin){
        case BUTTON_L:
            nrf_drv_gpiote_out_toggle(LED_R);
        break;
        case BUTTON_R:
            nrf_drv_gpiote_out_toggle(LED_G);
        break;
        case BUTTON_M:
            nrf_drv_gpiote_out_toggle(LED_B);
        break;
    }
    NRF_LOG_DEBUG("press = %d",pin);
}
void enc_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
    uint8_t a;
    uint8_t b;
    a = nrf_gpio_pin_read(ENCODER_A);
    b = nrf_gpio_pin_read(ENCODER_B);
    NRF_LOG_DEBUG("A B = %d %d",a,b);
}
static void gpio_init(void){
    APP_ERROR_CHECK(nrf_drv_gpiote_init());

    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    APP_ERROR_CHECK(nrf_drv_gpiote_out_init(LED_R, &out_config));
    APP_ERROR_CHECK(nrf_drv_gpiote_out_init(LED_G, &out_config));
    APP_ERROR_CHECK(nrf_drv_gpiote_out_init(LED_B, &out_config));
    APP_ERROR_CHECK(nrf_drv_gpiote_out_init(LED_POWER_OFF, &out_config));

    nrf_drv_gpiote_in_config_t button_config = GPIOTE_CONFIG_IN_SENSE_HITOLO(true);
    nrf_drv_gpiote_in_config_t enc_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    button_config.pull = NRF_GPIO_PIN_PULLUP;
    enc_config.pull = NRF_GPIO_PIN_PULLUP;
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BUTTON_L, &button_config, button_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BUTTON_R, &button_config, button_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BUTTON_M, &button_config, button_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BUTTON_AIM, &button_config, button_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BUTTON_FOW, &button_config, button_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BUTTON_PREV, &button_config, button_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(ENCODER_A, &enc_config, enc_handler));
    nrf_drv_gpiote_in_event_enable(BUTTON_L, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_R, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_M, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_AIM, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_FOW, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_PREV, true);
    nrf_drv_gpiote_in_event_enable(ENCODER_A, true);
    
    // 通常入力
    nrf_gpio_cfg_input(BUTTON_DPI, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BUTTON_LED, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(ENCODER_B, NRF_GPIO_PIN_PULLUP);
}

void log_init(){
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_DEBUG("end init");
}

const nrf_drv_timer_t TIMER0 = NRF_DRV_TIMER_INSTANCE(0);

void timer_event_handler(nrf_timer_event_t event_type, void* p_context){
    static int cnt_btn1 = 0;
    switch (event_type){
        case NRF_TIMER_EVENT_COMPARE0:
            if(cnt_btn1 <500){
                cnt_btn1++;
            }else{
                cnt_btn1 = 0;
                nrf_drv_gpiote_out_toggle(LED_R);
            }
            break;
    }
}
void timer_init(){
    uint32_t time_ms = 1; //Time(in miliseconds) between consecutive compare events.
    uint32_t time_ticks;
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.frequency = NRF_TIMER_FREQ_250kHz;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_8;
    APP_ERROR_CHECK(nrf_drv_timer_init(&TIMER0, &timer_cfg, timer_event_handler));
    //ticks = 時間[s] * (timer_cfg.frequency)はtimer_cfg.bit_width以下になる必要がある(ERROR_CHECKに引っかからない)
    time_ticks = nrf_drv_timer_ms_to_ticks(&TIMER0, time_ms);
    nrf_drv_timer_extended_compare(&TIMER0, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);
    nrf_drv_timer_enable(&TIMER0);
}

int main(void){
    gpio_init();
    log_init();
    timer_init();
    nrf_drv_gpiote_out_clear(LED_POWER_OFF);
    while (true)
    {
        __WFI();//低電力モード(割り込みで解除)
    }
}
