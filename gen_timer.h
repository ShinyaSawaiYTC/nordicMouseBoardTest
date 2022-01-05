#ifndef GEN_TIMER_H
#define GEN_TIMER_H

#include "boards.h"

extern void gen_timer_int_from_irq();//IRQから直接呼ばれる。重い処理は入れない方がよい
extern void gen_timer_int_from_loop();//メインループから呼ばれる。タイミングにゆるい処理を入れる

void gen_timer_init();
void gen_timer_set(uint16_t timer);
void gen_timer_start();
void gen_timer_stop();
void gen_timer_in_loop();//メインループに配置

#endif