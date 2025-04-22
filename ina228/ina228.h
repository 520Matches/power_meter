#ifndef __INA228_H
#define __INA228_H

#include <stdint.h>

typedef enum
{
	VSHUNT = 0,
	VBUS = 1,
	POWER = 2,
	CURRENT = 3,
}ina228_regs_t;

void ina228_init(void);
uint32_t ina228_read(ina228_regs_t reg);

// int ina228_deinit(void);

#endif
