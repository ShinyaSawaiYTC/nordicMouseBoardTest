
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
    ret_code_t err_code;

    err_code = nrf_drv_gpiote_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_out_config_t out_config = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(false);//GPIOTE_CONFIG_OUT_SIMPLE

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

int main(void){
    ret_code_t err_code;
    gpio_init();
    nrf_drv_gpiote_out_clear(LED_POWER_OFF);
    log_init();
    while (true)
    {
    }
}
