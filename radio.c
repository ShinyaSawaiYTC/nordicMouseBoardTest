/*******************************************
 * nRF52 RADIO ペリフェラルライブラリ
 * 
 * *****************************************/

#include "radio.h"

#include "nrf.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

rf_state_t m_rf_state;
void(*m_enc_callback_function)(rf_result_t*);
rf_result_t m_rf_res;

void clock_init() { //クロックが無いとRADIOモジュールが動かない(レジスタ的には動作しているように見えるが送信も受信もしていない)
    if(NRF_CLOCK->HFCLKSTAT&CLOCK_HFCLKSTAT_STATE_Msk == CLOCK_HFCLKSTAT_STATE_Running){
        return;
    }
    NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
    NRF_CLOCK->TASKS_HFCLKSTART = 1;
    while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
}
void RadioInit(void(*end_callback_function)(rf_result_t*)) {
    m_enc_callback_function = end_callback_function;
    clock_init();    
    //ショートカットするイベント→タスクを設定
    NRF_RADIO->SHORTS = (
        0UL
        | RADIO_SHORTS_READY_START_Msk          //準備完了したら送信・受信
        | RADIO_SHORTS_END_DISABLE_Msk          //終了したら即停止
        | RADIO_SHORTS_ADDRESS_RSSISTART_Msk    //ADDRESS送信|受信したらRSSI取得開始
        | RADIO_SHORTS_DISABLED_RSSISTOP_Msk    //停止したらRSSI取得停止
        // | RADIO_SHORTS_DISABLED_RXEN_Msk        //停止したら受信開始
    );
    //割り込みするイベント設定
    NRF_RADIO->INTENSET = (
        0UL
        // | RADIO_INTENSET_READY_Msk    //準備完了したら
        // | RADIO_INTENSET_ADDRESS_Msk  //アドレス完了
        // | RADIO_INTENSET_PAYLOAD_Msk  //ペイロード完了
        // | RADIO_INTENSET_END_Msk      //終了したら
        | RADIO_INTENSET_DISABLED_Msk //停止したら
    );
    NRF_RADIO->TXPOWER = RADIO_TXPOWER_TXPOWER_Neg40dBm << RADIO_TXPOWER_TXPOWER_Pos; //ESBは0dbm
    NRF_RADIO->MODE = RADIO_MODE_MODE_Nrf_2Mbit << RADIO_MODE_MODE_Pos; //ESB は2Mbpsがデフォ
    // CRC設定
    NRF_RADIO->CRCINIT = 0xFFFFUL;  // Initial value
    NRF_RADIO->CRCPOLY = 0x11021UL; // CRC poly: x^16+x^12^x^5+1
    NRF_RADIO->CRCCNF = (
        0UL
        | (RADIO_CRCCNF_LEN_Two<< RADIO_CRCCNF_LEN_Pos)
        | (RADIO_CRCCNF_SKIPADDR_Skip<< RADIO_CRCCNF_SKIPADDR_Pos) //CRCにアドレスを含める=Include、CRCはアドレスの後から=Skip、 Lengthの後から=Ieee802154
    );
    // パケット設定
    NRF_RADIO->PCNF0 = (
        0UL
        | (0 << RADIO_PCNF0_S0LEN_Pos)                            //S0フィールドの長さ(0~1byte)
        | (8 << RADIO_PCNF0_LFLEN_Pos)                            //LENGTHフィールドの長さ(0~Fbit)(RF上でもこの長さになる。8bitにパディングされない) ESBは6bit→len=32が最長
        | (8 << RADIO_PCNF0_S1LEN_Pos)                            //S1フィールドの長さ(0~Fbit)
        | (RADIO_PCNF0_CRCINC_Exclude << RADIO_PCNF0_CRCINC_Pos)  //lengthにCRCの長さを含めるか
        | (RADIO_PCNF0_PLEN_8bit << RADIO_PCNF0_PLEN_Pos)         //preamble長さ
    );
    NRF_RADIO->PCNF1 = (
        0UL
        | (RADIO_PCNF1_WHITEEN_Disabled << RADIO_PCNF1_WHITEEN_Pos)    //ホワイトニング
        | (RADIO_PCNF1_ENDIAN_Big << RADIO_PCNF1_ENDIAN_Pos)       //S0 LENGTH S1 PAYLOADのエンディアン
        | (3 << RADIO_PCNF1_BALEN_Pos)                             //ベースアドレスの長さ(2~4byte)はみ出たら下から切り捨て
        | (0 << RADIO_PCNF1_STATLEN_Pos)                           //受信時のペイロードのLENDTHに追加される長さ
        | (RADIO_MAX_PAYLOAD_LENGTH << RADIO_PCNF1_MAXLEN_Pos)                           //パケット PAYLOADの最大値(超えると切り捨てられる)LENGTHは含めない
    );
    NVIC_SetPriority(RADIO_IRQn, 1 & 0x07); //割込優先度の設定(よくわからない)

}
void rf_disable(){//強制的にRXモードを停止する
    uint32_t shorts_tmp;
    uint32_t inten_tmp;
    if(NRF_RADIO->STATE != RADIO_STATE_STATE_Disabled){
        NVIC_DisableIRQ(RADIO_IRQn);//INTENCLRしても割り込み発生するから
        shorts_tmp = NRF_RADIO->SHORTS;
        inten_tmp = NRF_RADIO->INTENSET;
        NRF_RADIO->SHORTS = 0;
        NRF_RADIO->INTENCLR = 0xFFFFFFFF;
        NRF_RADIO->EVENTS_DISABLED = 0;
        NRF_RADIO->TASKS_DISABLE = 1;
        while (NRF_RADIO->EVENTS_DISABLED == 0);
        m_rf_state = RF_STATE_DISABLE;
        NRF_LOG_DEBUG("RADIO DISABLED");
        NRF_RADIO->SHORTS = shorts_tmp;
        NRF_RADIO->INTENSET = inten_tmp;
    }
}
void rf_txrx_set(rf_config_t *config){
    NVIC_DisableIRQ(RADIO_IRQn);

    NRF_RADIO->BASE0 = config->add_base[0];
    NRF_RADIO->BASE1 = config->add_base[1];
    NRF_RADIO->PREFIX0 = (
        0UL
        |(uint32_t)(config->add_prefix[0])
        |(uint32_t)(config->add_prefix[1])<<8
        |(uint32_t)(config->add_prefix[2])<<16
        |(uint32_t)(config->add_prefix[3])<<24
    );
    NRF_RADIO->PREFIX1 = (
        0UL
        |(uint32_t)(config->add_prefix[4])
        |(uint32_t)(config->add_prefix[5])<<8
        |(uint32_t)(config->add_prefix[6])<<16
        |(uint32_t)(config->add_prefix[7])<<24
    );;
    NRF_RADIO->TXADDRESS = config->tx_add;      //送信アドレス選択0-7
    NRF_RADIO->RXADDRESSES = config->rx_adds; //受信するアドレス0-7をビットマスクで指定

    NRF_RADIO->FREQUENCY =(
        0UL
        |RADIO_FREQUENCY_MAP_Default << RADIO_FREQUENCY_MAP_Pos //周波数マップ
        |config->frequency << RADIO_FREQUENCY_FREQUENCY_Pos                    //基準から+1MHz単位で周波数設定
    );

    NRF_RADIO->PACKETPTR = (uint32_t)m_radio_packet_ptr; //DMA。ポインタはダブルバッファされている。STARTタスク前に毎回設定

    NRF_RADIO->EVENTS_ADDRESS = 0; //イベントクリア
    NRF_RADIO->EVENTS_PAYLOAD = 0;
    NRF_RADIO->EVENTS_DISABLED = 0;

    NVIC_ClearPendingIRQ(RADIO_IRQn); //割り込み要求クリア
    NVIC_EnableIRQ(RADIO_IRQn);       //割り込み有効化
}
int RadioStart(rf_state_t mode,rf_config_t *config){//0なら問題なし
    switch(mode){
        case RF_STATE_TX:
            if(m_rf_state == RF_STATE_TX){
                return 1;
            }
            rf_disable();
            m_rf_state = RF_STATE_TX;
            rf_txrx_set(config);
            NRF_RADIO->TASKS_TXEN = 1; //送信開始
            return 0;
        break;
        case RF_STATE_RX:
            m_rf_state = RF_STATE_RX;
            rf_txrx_set(config);
            NRF_RADIO->TASKS_RXEN = 1; //受信開始
            return 0;
        break;
        default:
            return 1;
    }
}
void RADIO_IRQHandler() {//RADIO割り込み
    // char *msgprefix;
    // switch(m_rf_state){
    //     case TX:
    //         msgprefix  = "TX";
    //     break;
    //     case RX:
    //         msgprefix  = "RX";
    //     break;
    //     default:
    //         msgprefix = "_";
    // }
    // if (NRF_RADIO->EVENTS_READY && (NRF_RADIO->INTENSET & RADIO_INTENSET_READY_Msk)){
    //     NRF_RADIO->EVENTS_READY = 0;
    //     NRF_LOG_DEBUG("%s : EVENTS_READY",msgprefix);
    //     NRF_RADIO->TASKS_START = 1;
    // }
    // if (NRF_RADIO->EVENTS_ADDRESS && (NRF_RADIO->INTENSET & RADIO_INTENSET_ADDRESS_Msk)){
    //     NRF_RADIO->EVENTS_ADDRESS = 0;
    //     NRF_LOG_DEBUG("%s : EVENTS_ADDRESS",msgprefix);
    // }
    // if (NRF_RADIO->EVENTS_PAYLOAD && (NRF_RADIO->INTENSET & RADIO_INTENSET_PAYLOAD_Msk)){
    //     NRF_RADIO->EVENTS_PAYLOAD = 0;
    //     NRF_LOG_DEBUG("%s : EVENTS_PAYLOAD",msgprefix);
    // }
    // if (NRF_RADIO->EVENTS_END && (NRF_RADIO->INTENSET & RADIO_INTENSET_END_Msk)){
    //     NRF_RADIO->EVENTS_END = 0;
    //     NRF_LOG_DEBUG("%s : EVENTS_END",msgprefix);
    //     NRF_RADIO->TASKS_DISABLE = 1;
    // }
    if (NRF_RADIO->EVENTS_DISABLED && (NRF_RADIO->INTENSET & RADIO_INTENSET_DISABLED_Msk)){
        NRF_RADIO->EVENTS_DISABLED = 0;
        //NRF_LOG_DEBUG("%s : EVENTS_DISABLED",msgprefix);
        switch(m_rf_state){
            case RF_STATE_TX:
                m_rf_res.end_state = m_rf_state;
                m_rf_res.rssi = 0;
                m_rf_res.rx_ch = NRF_RADIO->TXADDRESS;
                m_rf_res.crc = 0;
                m_rf_res.crc_success = true;
                (*m_enc_callback_function)(&m_rf_res);
            break;
            case RF_STATE_RX:
                m_rf_res.end_state = m_rf_state;
                m_rf_res.rssi = NRF_RADIO->RSSISAMPLE;
                m_rf_res.rx_ch = NRF_RADIO->RXMATCH;
                m_rf_res.crc = NRF_RADIO->RXCRC;
                m_rf_res.crc_success = NRF_RADIO->CRCSTATUS;//CRCが整合
                (*m_enc_callback_function)(&m_rf_res);
            break;
        }
    }
}