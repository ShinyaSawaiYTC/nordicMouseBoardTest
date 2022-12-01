#ifndef __RADIO_H
#define __RADIO_H

#include <stdint.h>
#include <stdbool.h>

#define RADIO_MAX_PAYLOAD_LENGTH 31
#define RADIO_S0_LENGTH 1
#define RADIO_S1_LENGTH 1
#define RADIO_L_LENGTH 1

extern const uint8_t *m_radio_packet_ptr;//送受信するパケットポインタ 上位モジュールCCMで設定する

typedef struct{
    uint32_t add_base[2];
    uint8_t add_prefix[8];
    uint8_t tx_add;
    uint8_t rx_adds;
    uint8_t power;
    uint8_t frequency;
} rf_config_t;

typedef enum {
    RF_STATE_UNDEFINED = 0,
    RF_STATE_TX,
    RF_STATE_RX,
    RF_STATE_DISABLE,
} rf_state_t;

typedef struct{
    rf_state_t end_state;
    bool crc_success;
    uint32_t crc;//24bit
    uint8_t rssi;//-{data}dBm
    uint8_t rx_ch;//0~7ch
} rf_result_t;

#define RF_CONFIG_DEFAULT {\
    .add_base = {0x99999999,0x43434343},\
    .add_prefix = {0x99,0x43,0xC3,0x23,0xA3,0x63,0xE3,0x13},\
    .tx_add = 0,\
    .rx_adds = 1,\
    .power = RADIO_TXPOWER_TXPOWER_Neg40dBm,\
    .frequency = 2\
}

void RadioInit(void(*end_callback_function)(rf_result_t*));
int RadioStart(rf_state_t mode,rf_config_t *settings);//0なら問題なし
#endif