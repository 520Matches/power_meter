#include "systick.h"

volatile uint32_t time_delay = 0;

void systick_init(void)
{
	if(SysTick_Config(SystemCoreClock / 1000))
	{
		while(1);
	}
}

void delay_ms(uint32_t ms)
{
	time_delay = ms;
	while(time_delay != 0);
}

void SysTick_Handler(void)
{
	if(time_delay != 0)
	{
		time_delay--;
	}
}
