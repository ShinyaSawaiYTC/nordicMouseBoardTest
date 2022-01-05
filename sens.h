#ifndef SENS_H
#define SENS_H

#include "boards.h"

#define SENS_BURST_READ_BYTES 6
extern volatile uint8_t sens_burst_result[SENS_BURST_READ_BYTES];
extern volatile bool sens_burst_end_flag;

void sens_init();
void sens_in_loop();

void sens_burst_read_start();

#endif