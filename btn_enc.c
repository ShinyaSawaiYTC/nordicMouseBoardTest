#include "btn_enc.h"

#include "boards.h"
#include "nrf_gpio.h"
#include "nrf_drv_gpiote.h"
#include "nrf_drv_timer.h"

#include "nrf_log.h"

typedef enum {
    SW_BUTTON_R = 0,
    SW_BUTTON_L,
    SW_BUTTON_M,
    SW_BUTTON_FOW,
    SW_BUTTON_PREV,
    SW_BUTTON_AIM,
    SW_ENCODER_A,
    SW_NUMBER,
}sw_list_t;

static int cntChatt[SW_NUMBER];
static bool swChanged[SW_NUMBER];
static const uint8_t TimeChattButton = 1;
static const uint8_t TimeChattEnc = 3;
int pin_to_sw_list_enm(nrf_drv_gpiote_pin_t pin){
    switch(pin){
        case BUTTON_R: return SW_BUTTON_R;
        case BUTTON_L: return SW_BUTTON_L;
        case BUTTON_M: return SW_BUTTON_M;
        case BUTTON_FOW: return SW_BUTTON_FOW;
        case BUTTON_PREV: return SW_BUTTON_PREV;
        case BUTTON_AIM: return SW_BUTTON_AIM;
        case ENCODER_A: return SW_ENCODER_A;
        default: return -1;
    }
}
int sw_to_pin_list_enm(nrf_drv_gpiote_pin_t pin){
    switch(pin){
        case SW_BUTTON_R: return BUTTON_R;
        case SW_BUTTON_L: return BUTTON_L;
        case SW_BUTTON_M: return BUTTON_M;
        case SW_BUTTON_FOW: return BUTTON_FOW;
        case SW_BUTTON_PREV: return BUTTON_PREV;
        case SW_BUTTON_AIM: return BUTTON_AIM;
        case SW_ENCODER_A: return ENCODER_A;
        default: return -1;
    }
}
void sw_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){
    for(int i=0; i < SW_NUMBER; i++){
        if(!swChanged[i] && pin_to_sw_list_enm(pin) == i){
            cntChatt[i] = 0;
            swChanged[i] = true;
        }
    }
}
void sens_motion_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action){

}
bool is_sw_wait_end(sw_list_t sw,uint8_t chatt_time){
    bool ret = false;
    if(swChanged[sw] && cntChatt[sw] > TimeChattEnc){
        swChanged[sw] = false;
        ret = true;
    }
    return ret;
}
button_state_t is_sw_wait_btn_changed(sw_list_t sw,uint8_t chatt_time){
    button_state_t ret = BTN_NO_CHANGE;
    if(is_sw_wait_end(sw,chatt_time)){
        if(nrf_gpio_pin_read(sw_to_pin_list_enm(sw))){
            ret = BTN_RELEASE;
        }else{
            ret = BTN_PUSH;
        }
    }
    return ret;
}
encoder_state_t is_sw_wait_enc_changed(sw_list_t sw,uint8_t chatt_time){
    uint8_t a;
    uint8_t b;
    if(is_sw_wait_end(sw,chatt_time)){
        a = nrf_gpio_pin_read(ENCODER_A);
        b = nrf_gpio_pin_read(ENCODER_B);
        // 1クリックで1周期するタイプ
        // state = ((state << 2)|a<<1|b)&0x0f;
        // if(state == 0b0011){
        //     enc_change(true);
        // }else if(state == 0b0110){
        //     enc_change(false);
        // }
        // 1クリックで半周期するタイプ
        if(a == b){
            return ENC_CW;
        }else{
            return ENC_CCW;
        }
    }else{
        return ENC_NO_CHANGE;
    }
}
void gpio_init(void){
    for(int i =0; i< SW_NUMBER; i++){
        swChanged[i] = false;
        cntChatt[i] = 0;
    }
    // GPIOTE入出力
    // GPIOTEはそれぞれ8つまでのTasks/Eventsが設定できる。
    APP_ERROR_CHECK(nrf_drv_gpiote_init());

    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);
    APP_ERROR_CHECK(nrf_drv_gpiote_out_init(LED_R, &out_config));
    APP_ERROR_CHECK(nrf_drv_gpiote_out_init(LED_G, &out_config));
    APP_ERROR_CHECK(nrf_drv_gpiote_out_init(LED_B, &out_config));

    nrf_drv_gpiote_in_config_t button_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    nrf_drv_gpiote_in_config_t enc_config = GPIOTE_CONFIG_IN_SENSE_TOGGLE(true);
    nrf_drv_gpiote_in_config_t sns_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    button_config.pull = NRF_GPIO_PIN_PULLUP;
    enc_config.pull = NRF_GPIO_PIN_PULLUP;
    sns_config.pull = NRF_GPIO_PIN_NOPULL;

    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BUTTON_L, &button_config, sw_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BUTTON_R, &button_config, sw_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BUTTON_M, &button_config, sw_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BUTTON_AIM, &button_config, sw_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BUTTON_FOW, &button_config, sw_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(BUTTON_PREV, &button_config, sw_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(ENCODER_A, &enc_config, sw_handler));
    APP_ERROR_CHECK(nrf_drv_gpiote_in_init(SNS_MOTION, &enc_config, sens_motion_handler));
    nrf_drv_gpiote_in_event_enable(BUTTON_L, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_R, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_M, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_AIM, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_FOW, true);
    nrf_drv_gpiote_in_event_enable(BUTTON_PREV, true);
    nrf_drv_gpiote_in_event_enable(ENCODER_A, true);
    nrf_drv_gpiote_in_event_enable(SNS_MOTION, true);
    
    // 通常出力
    nrf_gpio_cfg_output(LED_POWER_OFF);
    // 通常入力
    nrf_gpio_cfg_input(BUTTON_DPI, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BUTTON_LED, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(ENCODER_B, NRF_GPIO_PIN_PULLUP);
}


const nrf_drv_timer_t TIMER0 = NRF_DRV_TIMER_INSTANCE(0);

void timer_event_handler(nrf_timer_event_t event_type, void* p_context){
    encoder_state_t enc;
    button_state_t btn;
    switch (event_type){
        case NRF_TIMER_EVENT_COMPARE0:
            for(int i =0; i< SW_NUMBER; i++){
                if(cntChatt[i] <500){
                    cntChatt[i]++;
                }
            }
            enc = is_sw_wait_enc_changed(SW_ENCODER_A,TimeChattEnc);
            if(enc != ENC_NO_CHANGE){
                NRF_LOG_DEBUG("enc = %s",enc == ENC_CW?"CW":"CCW");
            }
            btn = is_sw_wait_btn_changed(SW_BUTTON_L,TimeChattButton);
            if(btn != BTN_NO_CHANGE){
                NRF_LOG_DEBUG("BUTTON_L = %s",btn == BTN_PUSH?"PUSHED":"RELEASED");
            }
            btn = is_sw_wait_btn_changed(SW_BUTTON_R,TimeChattButton);
            if(btn != BTN_NO_CHANGE){
                NRF_LOG_DEBUG("BUTTON_R = %s",btn == BTN_PUSH?"PUSHED":"RELEASED");
            }
            btn = is_sw_wait_btn_changed(SW_BUTTON_M,TimeChattButton);
            if(btn != BTN_NO_CHANGE){
                NRF_LOG_DEBUG("BUTTON_M = %s",btn == BTN_PUSH?"PUSHED":"RELEASED");
            }
            btn = is_sw_wait_btn_changed(SW_BUTTON_FOW,TimeChattButton);
            if(btn != BTN_NO_CHANGE){
                NRF_LOG_DEBUG("BUTTON_FOW = %s",btn == BTN_PUSH?"PUSHED":"RELEASED");
            }
            btn = is_sw_wait_btn_changed(SW_BUTTON_PREV,TimeChattButton);
            if(btn != BTN_NO_CHANGE){
                NRF_LOG_DEBUG("BUTTON_PREV = %s",btn == BTN_PUSH?"PUSHED":"RELEASED");
            }
            btn = is_sw_wait_btn_changed(SW_BUTTON_AIM,TimeChattButton);
            if(btn != BTN_NO_CHANGE){
                NRF_LOG_DEBUG("BUTTON_AIM = %s",btn == BTN_PUSH?"PUSHED":"RELEASED");
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
