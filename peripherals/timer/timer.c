#include "stdint.h"
#include "timer.h"

// 定义定时器参数（系统时钟以72MHz计算）
#define TIM2_PRESCALER     7199    // 预分频值
#define TIM2_PERIOD       4999    // 自动重装载值

volatile uint8_t timer_event = 0;

void timer_init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    
    // 使能TIM2时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    
    // 定时器基础设置
    TIM_TimeBaseStructure.TIM_Period = TIM2_PERIOD;          // 自动重装载值
    TIM_TimeBaseStructure.TIM_Prescaler = TIM2_PRESCALER;    // 预分频值
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;  // 时钟分频
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; // 向上计数模式
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    
    // 使能更新中断
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
    
    // 启动定时器
    TIM_Cmd(TIM2, ENABLE);

	NVIC_InitTypeDef NVIC_InitStructure;
    
    // 设置中断优先级
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

void TIM2_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);

		timer_event = 1;
	}
}

