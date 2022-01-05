#include "gen_timer.h"

static bool gen_timer_irq_flag;

void TIMER4_IRQHandler(){
    if(NRF_TIMER4->EVENTS_COMPARE[0] && NRF_TIMER4->INTENSET & TIMER_INTENSET_COMPARE0_Msk){
        NRF_TIMER4->EVENTS_COMPARE[0] = 0;
        gen_timer_irq_flag = true;
        gen_timer_int_from_irq();
    }
}
void gen_timer_init(){
    NRF_TIMER4->INTENSET = TIMER_INTENSET_COMPARE0_Enabled << TIMER_INTENSET_COMPARE0_Pos;
    NRF_TIMER4->SHORTS =  TIMER_SHORTS_COMPARE0_CLEAR_Enabled << TIMER_SHORTS_COMPARE0_CLEAR_Pos;
    NRF_TIMER4->MODE = TIMER_MODE_MODE_Timer << TIMER_MODE_MODE_Pos;
    NRF_TIMER4->BITMODE = TIMER_BITMODE_BITMODE_16Bit << TIMER_BITMODE_BITMODE_Pos;
    NRF_TIMER4->PRESCALER = 4 << TIMER_PRESCALER_PRESCALER_Pos;//timer step = 2^PRESCALER/16MHz
    NRF_TIMER4->TASKS_CLEAR = 1;
    NVIC_SetPriority(TIMER4_IRQn,7);
    gen_timer_irq_flag = false;
}
void gen_timer_set(uint16_t timer){
    NRF_TIMER4->CC[0] = timer;
}
void gen_timer_start(){
    NRF_TIMER4->TASKS_CLEAR = 1;
    NVIC_ClearPendingIRQ(TIMER4_IRQn); //割り込み要求クリア
    NVIC_EnableIRQ(TIMER4_IRQn);       //割り込み有効化
    NRF_TIMER4->TASKS_START = 1;
}
void gen_timer_stop(){
    NVIC_DisableIRQ(TIMER4_IRQn);       //割り込み無効化
    NRF_TIMER4->TASKS_STOP = 1;
}
void gen_timer_in_loop(){
    if(gen_timer_irq_flag){
        gen_timer_irq_flag = false;
        gen_timer_int_from_loop();
    }
}