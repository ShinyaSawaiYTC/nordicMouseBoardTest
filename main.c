
#include <stdbool.h>
#include <stdint.h>

#include "nrf.h"
#include "nordic_common.h"
#include "boards.h"

#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "btn_enc.h"
#include "sens.h"
#include "led.h"
#include "gen_timer.h"

void log_init(){
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_DEBUG("end init");
}
volatile int ms_cunt = 0;
void gen_timer_int_from_irq(){}
void gen_timer_int_from_loop(){
    led_in_timer_int();
    ms_cunt ++;
    if(ms_cunt>50){
        if(!nrf_gpio_pin_read(SNS_MOTION)){
            sens_burst_read_start();
        }
        ms_cunt = 0;
    }
}
int main(void){
    log_init();
    sens_init();
    gpio_init();
    timer_init();
    led_init();
    gen_timer_init();
    gen_timer_set(1000);
    gen_timer_start();
    while (true)
    {
        gen_timer_in_loop();
        if(sens_burst_end_flag){
            sens_burst_end_flag = false;
            int16_t dx = sens_burst_result[3]<<8|sens_burst_result[2];
            int16_t dy = sens_burst_result[5]<<8|sens_burst_result[4];
            NRF_LOG_DEBUG("%d,%d",dx,dy);
        }
        NRF_LOG_FLUSH();
        // sens_in_loop();
        // __WFI();//低電力モード(割り込みで解除)
    }
}
