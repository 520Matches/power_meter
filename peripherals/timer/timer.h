#ifndef __TIMER_H
#define __TIMER_H

#include "stm32f10x.h"
#include "stm32f10x_tim.h"

extern volatile uint8_t timer_event;

void timer_init(void);

#endif
