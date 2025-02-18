#ifndef __INA228_H
#define __INA228_H

typedef enum
{
	POWER = 0,
	CURRENT = 1,
	VOLTAGE = 2,
}ina228_regs_t;

void ina228_init(void);
double ina228_read(ina228_regs_t reg);

// int ina228_deinit(void);

#endif
