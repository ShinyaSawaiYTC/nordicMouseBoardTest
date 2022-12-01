
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
#include "usb.h"
#include "radio_ccm.h"

void log_init(){
    APP_ERROR_CHECK(NRF_LOG_INIT(NULL));
    NRF_LOG_DEFAULT_BACKENDS_INIT();
    NRF_LOG_DEBUG("end init");
}
volatile int ms_cunt = 0;
void gen_timer_int_from_irq(){}
void gen_timer_int_from_loop(){
    led_in_timer_int();
    if(!nrf_gpio_pin_read(SNS_MOTION)){
        sens_burst_read_start();
    }
    if(ms_cunt<0x7fff){
        ms_cunt++;
    }
}
CCM_crypto_packet_t ccm_packet_rx;
CCM_crypto_packet_t ccm_packet_tx;
CCM_crypto_packet_t ccm_packet_buffer;
CCM_radio_result_t ccm_res_buffer;
rf_result_t  rf_res_buffer;
bool radio_end_flag = false;
CCM_config_t ccm_cnf = CCM_CONFIG_DEFAULT;
rf_config_t rf_config = RF_CONFIG_DEFAULT;

void RadioShowPacket(CCM_crypto_packet_t *packet) {//パケット内容を表示
    uint8_t len = packet->length;
    NRF_LOG_DEBUG("packet-------------");
    NRF_LOG_DEBUG("header = 0x%02x", packet->header);
    NRF_LOG_DEBUG("len = 0x%02x", len);
    NRF_LOG_DEBUG("RFU = 0x%02x", packet->RFU);
    NRF_LOG_HEXDUMP_DEBUG(packet->payload,len);
    NRF_LOG_DEBUG("end packet---------");
}

void RadioTxt2Packet(char *txtptr, CCM_crypto_packet_t *packet) {//文字列からパケットを作成
    int i;
    packet->header = 0x00;//送信されないがCRC計算に使用される
    packet->RFU = 0x00;//送信されるがCRC計算に使用されない
    for (i = 0; txtptr[i] != '\0'; i++){
        packet->payload[i] = txtptr[i];
    }
    packet->payload[i] = '\0';
    packet->length = (uint8_t)i + 1;
}

void RadioEndCallback(CCM_radio_result_t *res){
    ccm_res_buffer = *res;
    rf_res_buffer = *(res->rf_result);
    memcpy(&ccm_packet_buffer,&ccm_packet_rx,sizeof(CCM_crypto_packet_t));
    radio_end_flag = true;
    RADIO_CCMStart(RF_STATE_RX,&rf_config,&ccm_cnf,&ccm_packet_rx);
}

int main(void){
    log_init();
    sens_init();
    sens_cpi_set(CPI_800);
    gpio_init();
    timer_init();
    led_init();
    usb_init();
    gen_timer_init();
    gen_timer_set(1000);
    gen_timer_start();
    RADIO_CCMInit(RadioEndCallback);
    while (true)
    {
        usb_in_loop();
        gen_timer_in_loop();
        if(sens_burst_end_flag){
            sens_burst_end_flag = false;
            int16_t dx = sens_burst_result[3]<<8|sens_burst_result[2];
            int16_t dy = sens_burst_result[5]<<8|sens_burst_result[4];
            // NRF_LOG_DEBUG("%d,%d",dx,dy);
            app_usbd_hid_mouse_x_move(usb_mouse_handler, dx);
            app_usbd_hid_mouse_y_move(usb_mouse_handler, dy);
        }
        if(ms_cunt >= 1000){
            ms_cunt = 0;
            if(!nrf_gpio_pin_read(BUTTON_LED)){
                NRF_LOG_DEBUG("CPI = CPI_2800");
                sens_cpi_set(CPI_2800);
                
                RadioTxt2Packet("~~encrypt test text~~", &ccm_packet_tx);
                NRF_LOG_DEBUG("=== tx data ===");
                RadioShowPacket(&ccm_packet_tx);
                NRF_LOG_DEBUG("~~~ tx data end ~~~");
                RADIO_CCMStart(RF_STATE_TX,&rf_config,&ccm_cnf,&ccm_packet_tx);
            }
            // sens_in_loop();
        }
        if(radio_end_flag){
            radio_end_flag = false;
            if(rf_res_buffer.end_state == RF_STATE_TX){
                NRF_LOG_DEBUG("SEND END");
            }else if(rf_res_buffer.end_state == RF_STATE_RX){
                if(rf_res_buffer.crc_success){
                    NRF_LOG_DEBUG("=== rx data ===");
                    NRF_LOG_DEBUG("CRC %s",rf_res_buffer.crc_success?"OK":"NG");
                    NRF_LOG_DEBUG("CRC = 0x%06x",rf_res_buffer.crc);
                    NRF_LOG_DEBUG("CCM_MIC %s",ccm_res_buffer.ccm_mic_success?"OK":"NG");
                    NRF_LOG_DEBUG("RECIEVE END -%ddBm",rf_res_buffer.rssi);
                    NRF_LOG_DEBUG("RECIEVE ADDRESS %d ch",rf_res_buffer.rx_ch);
                    RadioShowPacket(&ccm_packet_buffer);
                    NRF_LOG_DEBUG("~~~ rx data end ~~~");
                }
            }
        }
        NRF_LOG_INTERNAL_PROCESS();
        // __WFI();//低電力モード(割り込みで解除)
    }
}