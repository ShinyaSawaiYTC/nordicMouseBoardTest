#ifndef __RADIO_CCM_H
#define __RADIO_CCM_H

#include <stdint.h>
#include <stdbool.h>
#include "radio.h"

typedef struct{//CCM設定構造体・DMAで読み込まれるのでメンバポインタ変更不可
    uint8_t key[16];
    uint8_t pktctr[5];//packet counter. [0]is LSByte, [4]bit6 is MSBit [4]bit7 is Ignored,
    uint8_t pktctr_ignored[3];
    uint8_t direction;
    uint8_t iv[8];// Initialization Vector. [0] is LSByte
}CCM_config_t;

typedef struct{//CCM入出力構造体・DMAで読み込まれるのでメンバポインタ変更不可
    uint8_t header; //headerに0xE3をマスクしたものがMIC生成に使用される(AAD)
    uint8_t length; //payloadの長さ
    uint8_t RFU;    //暗号化にもMIC生成にも使用されない。入力がそのまま出てくる
    uint8_t payload[RADIO_MAX_PAYLOAD_LENGTH];
} CCM_crypto_packet_t;

typedef struct{
    rf_result_t *rf_result;
    bool ccm_mic_success;
} CCM_radio_result_t;


#define CCM_CONFIG_DEFAULT {\
    .key = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F},\
    /*.iv = {0x07,0x06,0x05,0x04,0x03,0x02,0x01,0x00},*/\
    .iv = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00},\
    .pktctr = {0,0,0,0,0},\
    .direction = 0,\
};

void RADIO_CCMInit(void(*end_callback)(CCM_radio_result_t*));
int RADIO_CCMStart(rf_state_t mode,rf_config_t *rf_conf, CCM_config_t *ccm_cnf,CCM_crypto_packet_t *packet);


#endif