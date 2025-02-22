#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stdint.h"
#include "core_cm3.h"

void systick_init(void);

void delay_ms(uint32_t ms);

#endif
