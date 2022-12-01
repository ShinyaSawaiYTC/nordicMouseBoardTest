/*******************************************
 * nRF52 CCM ペリフェラルライブラリ
 * radioモジュールの上位モジュール
 * PPIを通してRADIOペリフェラルに直結される
 * *****************************************/
#include "radio_ccm.h"
#include "radio.h"

#include "nrf.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

uint8_t m_ccm_scratch[43] = {};
CCM_crypto_packet_t m_ccm_packet_buffer;//radioとのDMA通信用
const uint8_t *m_radio_packet_ptr= (uint8_t*)(&m_ccm_packet_buffer)+1;//アドレスを1ずらしてS0を飛ばしてLengthから送信受信開始

void(*m_ccm_radio_end_callback)(CCM_radio_result_t*);

bool m_ccm_end_f;
bool m_ccm_rf_end_f;
CCM_radio_result_t m_ccm_radio_result;
rf_state_t m_ccm_radio_mode;

void CCMInit(){
    //割り込み設定
    NRF_CCM->INTENSET = (
        0UL
        | (CCM_INTENSET_ENDCRYPT_Enabled<<CCM_INTENSET_ENDCRYPT_Pos)
    );
    NRF_CCM->MODE = (
        0UL
        | (CCM_MODE_DATARATE_2Mbit<<CCM_MODE_DATARATE_Pos)
        | (CCM_MODE_LENGTH_Default<<CCM_MODE_LENGTH_Pos)//キーストリームの長さ
    );
    NRF_CCM->ENABLE = CCM_ENABLE_ENABLE_Enabled<<CCM_ENABLE_ENABLE_Pos;
    NRF_CCM->SCRATCHPTR = (uint32_t)m_ccm_scratch;//CCM一時保管場所
}
void CCMStart(CCM_config_t *ccm_cnf,CCM_crypto_packet_t *packet){
    NVIC_DisableIRQ(CCM_AAR_IRQn);//割り込み無効化
    NRF_CCM->CNFPTR = (uint32_t)ccm_cnf;//CCM設定
    if(m_ccm_radio_mode == RF_STATE_TX){// 次のRadioイベントは送信→暗号化
        NRF_CCM->MODE &= ~CCM_MODE_MODE_Msk;//CCM_MODE_MODE_Encryption = 0
        //ショートカットするイベント→タスクを設定
        NRF_CCM->SHORTS |= CCM_SHORTS_ENDKSGEN_CRYPT_Msk; // ENDKSGEN → CRYPT
        //ペリフェラル間ショートカットするイベント→タスクを設定
        NRF_PPI->CHENSET = PPI_CHENSET_CH24_Msk;    //RADIO->EVENTS_READY→CCM->TASKS_KSGEN
        NRF_PPI->CHENCLR = PPI_CHENSET_CH25_Msk;    //RADIO->EVENTS_ADDRESS→CCM->TASKS_CRYPT    無効
        NRF_CCM->INPTR = (uint32_t)packet;
        NRF_CCM->OUTPTR = (uint32_t)&m_ccm_packet_buffer;
    }else{// 次のRadioイベントは受信→復号
        NRF_CCM->MODE |= CCM_MODE_MODE_Msk;//CCM_MODE_MODE_Decryption = 1
        //ショートカットするイベント→タスクを設定
        NRF_CCM->SHORTS &= ~CCM_SHORTS_ENDKSGEN_CRYPT_Msk; // ENDKSGEN → CRYPT  無効
        //ペリフェラル間ショートカットするイベント→タスクを設定
        NRF_PPI->CHENSET = PPI_CHENSET_CH24_Msk;    //RADIO->EVENTS_READY→CCM->TASKS_KSGEN
        NRF_PPI->CHENSET = PPI_CHENSET_CH25_Msk;    //RADIO->EVENTS_ADDRESS→CCM->TASKS_CRYPT
        NRF_CCM->INPTR = (uint32_t)&m_ccm_packet_buffer;
        NRF_CCM->OUTPTR = (uint32_t)packet;
    }
    NVIC_ClearPendingIRQ(CCM_AAR_IRQn); //割り込み要求クリア
    NVIC_EnableIRQ(CCM_AAR_IRQn);       //割り込み有効化
}
void CCM_AAR_IRQHandler(){//CCM割り込み
    if(NRF_CCM->EVENTS_ENDCRYPT&&(NRF_CCM->INTENSET & CCM_INTENSET_ENDCRYPT_Msk)){
        NRF_CCM->EVENTS_ENDCRYPT = 0;
        if(m_ccm_radio_mode == RF_STATE_RX){//TX終了はRADIOからのみ
            m_ccm_radio_result.ccm_mic_success = NRF_CCM->MICSTATUS;
            m_ccm_end_f = true;
            if(m_ccm_rf_end_f){//RF受信がすでに終了していたら
                (*m_ccm_radio_end_callback)(&m_ccm_radio_result);
            }
        }
    }
}
void RadioEndHandler(rf_result_t* rf_result){
    m_ccm_radio_result.rf_result = rf_result;
    switch(rf_result->end_state){
        case RF_STATE_TX:
            m_ccm_radio_result.ccm_mic_success = true;
            (*m_ccm_radio_end_callback)(&m_ccm_radio_result);
        break;
        case RF_STATE_RX:
            m_ccm_rf_end_f = true;
            if(m_ccm_end_f){//CCMデコードがすでに終了していたら
                (*m_ccm_radio_end_callback)(&m_ccm_radio_result);
            }
        break;
    }
}
////////////////////// RADIO_CCM //////////////////////////////
void RADIO_CCMInit(void(*end_callback)(CCM_radio_result_t*)){
    RadioInit(RadioEndHandler);
    CCMInit();
    m_ccm_radio_end_callback = end_callback;
}
int RADIO_CCMStart(rf_state_t mode,rf_config_t *rf_conf, CCM_config_t *ccm_cnf,CCM_crypto_packet_t *packet){
    if(mode != RF_STATE_RX && mode !=RF_STATE_TX){
        return 1;
    }
    m_ccm_radio_mode = mode;
    m_ccm_end_f = false;
    m_ccm_rf_end_f = false;
    CCMStart(ccm_cnf,packet);
    RadioStart(mode,rf_conf);
    return 0;
}