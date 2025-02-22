#include "stdint.h"
#include "timer.h"

volatile uint8_t timer_event = 0;

void timer_init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	uint16_t PrescalerValue = 0;
	// uint16_t CCR1_Val = 40961;
	/*!< At this stage the microcontroller clock setting is already configured, 
	   this is done through SystemInit() function which is called from startup
	   file (startup_stm32f10x_xx.s) before to branch to application main.
	   To reconfigure the default setting of SystemInit() function, refer to
	   system_stm32f10x.c file
	 */     
	/* System Clocks Configuration */
	/* PCLK1 = HCLK/4 */
	RCC_PCLK1Config(RCC_HCLK_Div4);

	/* TIM2 clock enable */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);   

	/* NVIC Configuration */
	NVIC_InitTypeDef NVIC_InitStructure;

	/* Enable the TIM2 global Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);

	/* ---------------------------------------------------------------
	TIM2 Configuration: Output Compare Timing Mode:
	TIM2 counter clock at 6 MHz
	CC1 update rate = TIM2 counter clock / CCR1_Val = 146.48 Hz
	CC2 update rate = TIM2 counter clock / CCR2_Val = 219.7 Hz
	CC3 update rate = TIM2 counter clock / CCR3_Val = 439.4 Hz
	CC4 update rate = TIM2 counter clock / CCR4_Val = 878.9 Hz
	--------------------------------------------------------------- */

	/* Compute the prescaler value */
	// PrescalerValue = (uint16_t) (SystemCoreClock / 12000000) - 1;

	/* Time base configuration */
	TIM_TimeBaseStructure.TIM_Period = 7199;
	TIM_TimeBaseStructure.TIM_Prescaler = 4999;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

	/* Prescaler configuration */
	TIM_PrescalerConfig(TIM2, PrescalerValue, TIM_PSCReloadMode_Immediate);

	/* Output Compare Timing Mode configuration: Channel1 */
	// TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
	// TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	// TIM_OCInitStructure.TIM_Pulse = ccr_val;
	// TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;

	TIM_OC1Init(TIM2, &TIM_OCInitStructure);

	TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Disable);

	TIM_ITConfig(TIM2, TIM_IT_CC1, ENABLE);

	/* TIM2 enable counter */
	TIM_Cmd(TIM2, ENABLE);
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_CC1) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_CC1);

		timer_event = 1;

		// capture = TIM_GetCapture1(TIM2);
		// TIM_SetCompare1(TIM2, capture + CCR1_Val);
	}
}

