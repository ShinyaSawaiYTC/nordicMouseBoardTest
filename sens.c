#include "sens.h"

#include "nrf_delay.h"
#include "nrf_gpio.h"
#include "nrf_log.h"

#include "boards.h"
/*****************************************************************************************************************
 * センサ : PAW3311DB
 * SPIクロック : 4MHz(最大8MHz)クロックによってMOSI-MISO間の抵抗値調整
 * NCS Low→SCLKアクティブ : 120ns
 * SCLK 終了→NCS Hi : 読み込み時:120ns,書き込み時1us
 * 
 * TSWW : 5us       ...書き込みから書き込み時の1つ目のデータ終了のCLK立ち上がりから2つ目のデータ終了のCLK立ち上がりまで最小時間
 * TSWR : 5us       ...書き込みから読み出し時の1つ目のデータ終了のCLK立ち上がりから2つ目のアドレス終了のCLK立ち上がりまで最小時間
 * TSRW/TSRR : 2us  ...読み出しから書き込み/読み出し時の1つ目のデータ終了のCLK立ち上がりから2つ目のアドレス開始のCLK立ち下がりまでの最小時間
 * TSRAD : 2us      ...レジスタ読み込み時のアドレス終了のCLK立ち上がりからデータ開始のCLK立ち下がりまでの最小時間
 * 書き込み時にはアドレス終了→データ開始の待機は不要
 * 
 * 1バイト送信のために 1/4M * 8= 2us
 * レジスタ操作2連続の場合、
 * 1つ目書き終わり→2つ目書き始め = TSWW - (2us + TSRAD + 2us )  = -1us →待機不要
 * 1つ目書き終わり→2つ目読み始め = TSWR - (2us + TSRAD + 2us )  = -1us →待機不要
 * 1つ目読み終わり→2つ目書き始め = TSRW  = 2us
 * 1つ目読み終わり→2つ目読み始め = TSRR  = 2us
 *************************************************************************************************************/

#define SPI_SCK_PIN  NRF_GPIO_PIN_MAP(0,07)
#define SPI_MOSI_PIN NRF_GPIO_PIN_MAP(0,05)
#define SPI_MISO_PIN NRF_GPIO_PIN_MAP(0,06)
#define SPI_CSN_PIN  NRF_GPIO_PIN_MAP(0,26)

#define SPI_BUFFER_LEN 8
uint8_t spi_tx_buf[SPI_BUFFER_LEN];
uint8_t spi_rx_buf[SPI_BUFFER_LEN];
uint8_t spi_cursor;
volatile bool spi_int_flag;
typedef enum{
    SENS_55_RES_PROCESSING,
    SENS_55_RES_OK,
    SENS_55_RES_TIMED_OUT,
}sens_init_setting_55_result_t;
volatile sens_init_setting_55_result_t sens_init_setting_55_result;

void (*sens_timer_int_function_ptr)();

void spi_init(){
    NRF_SPI0->INTENSET = (
        SPI_INTENSET_READY_Set << SPI_INTENSET_READY_Pos
    );
    NRF_SPI0->ENABLE = (
        SPI_ENABLE_ENABLE_Enabled << SPI_ENABLE_ENABLE_Pos
    );
    NRF_SPI0->FREQUENCY = SPI_FREQUENCY_FREQUENCY_M4;
    NRF_SPI0->CONFIG = (
        0
        | SPI_CONFIG_ORDER_MsbFirst << SPI_CONFIG_ORDER_Pos
        | SPI_CONFIG_CPHA_Trailing << SPI_CONFIG_CPHA_Pos//CLK立ち上がりでサンプル
        | SPI_CONFIG_CPOL_ActiveLow << SPI_CONFIG_CPOL_Pos//CLKは通常HI
    );
    NRF_SPI0->PSEL.SCK = SPI_SCK_PIN;
    NRF_SPI0->PSEL.MOSI = SPI_MOSI_PIN;
    NRF_SPI0->PSEL.MISO = SPI_MISO_PIN;
    
    NVIC_SetPriority(SPI0_TWI0_IRQn,SPI_DEFAULT_CONFIG_IRQ_PRIORITY);
    NVIC_ClearPendingIRQ(SPI0_TWI0_IRQn); //割り込み要求クリア
    NVIC_EnableIRQ(SPI0_TWI0_IRQn);       //割り込み有効化
}
void SPI0_TWI0_IRQHandler(){
    if(NRF_SPI0->EVENTS_READY  && NRF_SPI0->INTENSET & SPI_INTENSET_READY_Msk){//送信が1バイト完了
        NRF_SPI0->EVENTS_READY = 0;
        spi_int_flag = true;
    }
}
void sens_timer_init(){
    NRF_TIMER3->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;
    NRF_TIMER3->SHORTS =  TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;
    NRF_TIMER3->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
    NRF_TIMER3->BITMODE = TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos;
    NRF_TIMER3->PRESCALER = 4 << TIMER_PRESCALER_PRESCALER_Pos;//timer step = 2^PRESCALER/16MHz
    NRF_TIMER3->TASKS_CLEAR = 1;
    NVIC_SetPriority(TIMER3_IRQn,SPI_DEFAULT_CONFIG_IRQ_PRIORITY + 1);//timer割込中にSPI割込を許可する。
}
void sens_timer_set(uint16_t timer){
    NRF_TIMER3->CC[0] = timer;
}
void sens_timer_start(){
    NRF_TIMER3->TASKS_CLEAR = 1;
    NVIC_ClearPendingIRQ(TIMER3_IRQn); //割り込み要求クリア
    NVIC_EnableIRQ(TIMER3_IRQn);       //割り込み有効化
    NRF_TIMER3->TASKS_START = 1;
}
void sens_timer_stop(){
    NVIC_DisableIRQ(TIMER3_IRQn);       //割り込み無効化
    NRF_TIMER3->TASKS_STOP = 1;
}
void TIMER3_IRQHandler(){
    if(NRF_TIMER3->EVENTS_COMPARE[0] && NRF_TIMER3->INTENSET & TIMER_INTENSET_COMPARE0_Msk){
        NRF_TIMER3->EVENTS_COMPARE[0] = 0;
        (*sens_timer_int_function_ptr)();
    }
}
void sens_send(uint8_t data){
    spi_int_flag = false;
    NRF_SPI0->TXD = data;
    while(!spi_int_flag);
}
uint8_t sens_wr(uint8_t reg,uint8_t value,int delay){
    volatile uint8_t rep;
    nrf_gpio_pin_clear(SPI_CSN_PIN);
    nrf_delay_us(1);
    sens_send(reg);
    nrf_delay_us(delay);//TSRAD
    rep = NRF_SPI0->RXD;
    sens_send(value);
    rep = NRF_SPI0->RXD;
    nrf_delay_us(delay);//TSRR,TSRW
    nrf_delay_us(1);
    nrf_gpio_pin_set(SPI_CSN_PIN);
    return rep;
}
void sens_write(uint8_t reg,uint8_t value){
    sens_wr(reg|0x80,value,0);
}
uint8_t sens_read(uint8_t reg){
    return sens_wr(reg,0xff,2);
}
void sens_init_setting_55(){//初期手順のひとつ。タイマーで1ms毎に呼ばれる
    static uint8_t i = 0;
    uint8_t res;
    if(i == 0){//最初に呼ばれた場合初期化
        sens_init_setting_55_result = SENS_55_RES_PROCESSING;
        sens_timer_int_function_ptr = sens_init_setting_55;//割り込み関数に自身を登録
        sens_timer_set(1000);//1000us
        sens_timer_start();
    }
    res = sens_read(0x20);//55ms間1ms+-1%で読む
    if(res == 0x0F){
        sens_init_setting_55_result = SENS_55_RES_OK;
    }else if(i >= 55){
        sens_init_setting_55_result = SENS_55_RES_TIMED_OUT;
    }else{
        i++;
    }
    if(sens_init_setting_55_result != SENS_55_RES_PROCESSING){//終了した場合
        i = 0;
        sens_timer_stop();
        sens_timer_int_function_ptr = NULL;//割り込み関数を開放
    }
}
void sens_init_setting(){//初期化手順。データシート参考のこと
    uint8_t r1,r2;
    sens_write(0x40,0x80);
    sens_write(0x55,0x01);
    nrf_delay_ms(1);
    sens_write(0x7F,0x0E);
    sens_write(0x43,0x1D);
    r1 = sens_read(0x46);
    sens_write(0x43,0x1E);
    r2 = sens_read(0x46);
    sens_write(0x7F,0x14);
    sens_write(0x6A,r1);
    sens_write(0x6C,r2);
    sens_write(0x7F,0x00);
    sens_write(0x55,0x00);
    sens_write(0x4D,0xD0);
    sens_write(0x4E,0x23);
    sens_write(0x4F,0x46);
    sens_write(0x77,0x24);
    sens_write(0x7F,0x05);
    sens_write(0x44,0xA8);
    sens_write(0x4A,0x14);
    sens_write(0x53,0x0C);
    sens_write(0x5B,0xEA);
    sens_write(0x61,0x13);
    sens_write(0x62,0x07);
    sens_write(0x64,0xD8);
    sens_write(0x6D,0x86);
    sens_write(0x7D,0x84);
    sens_write(0x7E,0x00);
    sens_write(0x7F,0x06);
    sens_write(0x60,0x70);
    sens_write(0x61,0x00);
    sens_write(0x7E,0x40);
    sens_write(0x7F,0x07);
    sens_write(0x42,0x16);
    sens_write(0x7F,0x09);
    sens_write(0x40,0x03);
    sens_write(0x7F,0x0A);
    sens_write(0x49,0x00);
    sens_write(0x4A,0x23);
    sens_write(0x4C,0x28);
    sens_write(0x4F,0x02);
    sens_write(0x7F,0x0C);
    sens_write(0x40,0xA0);
    sens_write(0x41,0x70);
    sens_write(0x42,0x20);
    sens_write(0x43,0xC5);
    sens_write(0x44,0x44);
    sens_write(0x45,0x04);
    sens_write(0x4C,0x60);
    sens_write(0x54,0x00);
    sens_write(0x55,0x40);
    sens_write(0x59,0x93);
    sens_write(0x7F,0x0D);
    sens_write(0x4F,0x02);
    sens_write(0x50,0xE6);
    sens_write(0x4E,0x6B);
    sens_write(0x7F,0x14);
    sens_write(0x4A,0x67);
    sens_write(0x62,0x1C);
    sens_write(0x63,0x1C);
    sens_write(0x6D,0x82);
    sens_write(0x6F,0xD8);
    sens_write(0x73,0x83);
    sens_write(0x74,0x00);
    sens_write(0x7A,0x16);
    sens_write(0x7F,0x10);
    sens_write(0x48,0x0F);
    sens_write(0x49,0x88);
    sens_write(0x4C,0x1D);
    sens_write(0x4F,0x08);
    sens_write(0x51,0x6F);
    sens_write(0x52,0x90);
    sens_write(0x54,0x64);
    sens_write(0x55,0XF0);
    sens_write(0x5C,0x40);
    sens_write(0x61,0xEE);
    sens_write(0x62,0xE5);
    sens_write(0x7F,0x00);
    sens_write(0x5B,0x40);
    sens_write(0x61,0xAD);
    sens_write(0x51,0xEA);
    sens_write(0x19,0x9F);
    sens_init_setting_55();//sens_read(0x20)を55ms間1ms+-1%で実行
    while(sens_init_setting_55_result == SENS_55_RES_PROCESSING);//終了まで待つ
    if(sens_init_setting_55_result == SENS_55_RES_TIMED_OUT){
        NRF_LOG_DEBUG("Sens init failed (timed out)");
    }else{
        NRF_LOG_DEBUG("Sens init success");
    }
    sens_write(0x19,0x10);
    sens_write(0x40,0x00);
    sens_write(0x61,0xD5);
    sens_write(0x7F,0x00);
}
void sens_init(){
    /** タイマ設定 ***/
    sens_timer_init();
    /** SPI設定 ***/
    nrf_gpio_cfg_output(SPI_CSN_PIN);
    nrf_gpio_pin_set(SPI_CSN_PIN);
    spi_init();
    /** センサ初期化 ***/
    nrf_delay_ms(50);
    sens_write(0x3A,0x5A);
    nrf_delay_ms(5);
    sens_init_setting();
    sens_read(0x02);
    sens_read(0x03);
    sens_read(0x04);
    sens_read(0x05);
    sens_read(0x06);
}

void sens_in_loop(){
    uint8_t test;
    test = sens_read(0x15);
    NRF_LOG_DEBUG("CO = 0x%02x",test);
    sens_write(0x15,0x00);
    test = sens_read(0x02);
    sens_write(0x02,0x20);
    NRF_LOG_DEBUG("MOT = 0x%02x",test);
    nrf_delay_ms(500);
}