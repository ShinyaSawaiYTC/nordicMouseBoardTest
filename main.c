
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

void log_init(){
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_DEBUG("end init");
}

int main(void){
    log_init();
    sens_init();
    gpio_init();
    timer_init();
    nrf_gpio_pin_write(LED_POWER_OFF,false);
    while (true)
    {
        sens_in_loop();
        // __WFI();//低電力モード(割り込みで解除)
    }
}
