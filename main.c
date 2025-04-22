#include "core_cm3.h"

#include "usb_type.h"
#include "usb_core.h"
#include "usb_pwr.h"
#include "usb_init.h"
#include "usb_regs.h"
#include "usb_sil.h"
#include "usb_istr.h"
#include "hw_config.h"

#include "ina228.h"

#include "timer.h"

__IO uint8_t PrevXferComplete = 1;

void usb_init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
	GPIO_Init(GPIOA, &GPIO_InitStructure);

    Set_USBClock();
    USB_Init();
    USB_Interrupts_Config();
    USB_Cable_Config(ENABLE);
}

// void usb_write(uint8_t *report, uint8_t len)
// {
// 	PrevXferComplete = 0;
// 	USB_SIL_Write(EP1_IN, report, len);
// 	SetEPTxValid(EP1_IN);
// }
void usb_write(uint8_t *buf, uint32_t len)
{
	PrevXferComplete = 0;
    // 检查端点是否就绪
    if (GetEPTxStatus(ENDP1) != EP_TX_VALID) {
        // 复制数据到发送缓冲区
        UserToPMABufferCopy(buf, GetEPTxAddr(ENDP1), len);
        // 设置发送有效
        SetEPTxCount(ENDP1, len);
        SetEPTxValid(ENDP1);
    }
}


int main(void)
{
	uint8_t buf[9] = {0x01, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
	
	// double vbus    = 0.0;
	// double current = 0.0;
	uint32_t current = 0;

	timer_init();

	usb_init();
	
	ina228_init();

	while(1)
	{
		if(timer_event)
		{
			timer_event = 0;
			// vbus    = ina228_read(VBUS);
			current = ina228_read(CURRENT);
			buf[1] = current & 0xFF;
			buf[2] = (current >> 8) & 0xFF;
			buf[3] = (current >> 16) & 0xFF;
			buf[4] = (current >> 24) & 0xFF;
			if(bDeviceState == CONFIGURED)
			{
				if(PrevXferComplete)
				{
					usb_write(buf, sizeof(buf)/sizeof(buf[0]));
					// RHIDCheckState();
				}
			}
		}
	}

    return 0;
}

/**
  * @brief  USB低优先级/CAN接收中断服务函数
  * @note   此函数处理所有USB事件（如数据传输、复位、挂起等）
  */
void USB_LP_CAN1_RX0_IRQHandler(void) {
  // 调用USB中断处理函数
  USB_Istr();
}

