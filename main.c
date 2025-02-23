#include "core_cm3.h"

#include "usb_type.h"
#include "usb_core.h"
#include "usb_pwr.h"
#include "usb_init.h"
#include "usb_regs.h"
#include "usb_sil.h"

#include "hw_config.h"

#include "ina228.h"

#include "timer.h"

__IO uint8_t PrevXferComplete = 1;

void usb_init(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USB, ENABLE);
    // USB_Cable_Config(ENABLE);
    Set_USBClock();
    USB_Interrupts_Config();
    USB_Init();
}

// void usb_write(uint8_t *buf, uint32_t len)
// {
// 	PrevXferComplete = 0;
//
//     /* Copy mouse position info in ENDP1 Tx Packet Memory Area*/
//     USB_SIL_Write(EP1_IN, buf, len);
//     /* Enable endpoint for transmission */
//     SetEPTxValid(ENDP1);
// }

// void system_init(void)
// {
//   GPIO_InitTypeDef  GPIO_InitStructure;  
//   /*!< At this stage the microcontroller clock setting is already configured, 
//        this is done through SystemInit() function which is called from startup
//        file (startup_stm32xxx.s) before to branch to application main.
//        To reconfigure the default setting of SystemInit() function, refer to
//        system_stm32xxx.c file
//      */ 
//   
//   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
//   
//   /********************************************/
//   /*  Configure USB DM/DP pins                */
//   /********************************************/
//   
//   
//   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12;
//   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
//   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
//   GPIO_Init(GPIOA, &GPIO_InitStructure);
//   
//   /* Enable all GPIOs Clock*/
//   RCC_APB2PeriphClockCmd(RCC_APB2Periph_ALLGPIO, ENABLE);
//
//   
//   /********************************************/
//   /* Enable the USB PULL UP                   */
//   /********************************************/
// //   /* USB_DISCONNECT used as USB pull-up */
// //   GPIO_InitStructure.GPIO_Pin = USB_DISCONNECT_PIN;
// //   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
// //   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
// //   GPIO_Init(USB_DISCONNECT, &GPIO_InitStructure);
// //   
// //   /* Enable the USB disconnect GPIO clock */
// //   RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIO_DISCONNECT, ENABLE);
// }

int main(void)
{
	// uint8_t buf[5] = {0x01, 0x02, 0x03, 0x04, 0x05};
	//
	// Set_System();
	// USB_Interrupts_Config();
	// Set_USBClock();
	// USB_Init();
	//
	// while(1)
	// {
	// 	if(bDeviceState == CONFIGURED)
	// 	{
	// 		if(PrevXferComplete)
	// 		{
	// 			usb_write(buf, sizeof(buf)/sizeof(buf[0]));
	// 			// RHIDCheckState();
	// 		}
	// 	}
	// }
	
	double vbus    = 0.0;
	double current = 0.0;

	timer_init();

	usb_init();
	
	ina228_init();

	while(1)
	{
		if(timer_event)
		{
			timer_event = 0;
			vbus    = ina228_read(VBUS);
			current = ina228_read(CURRENT);
		}
	}

    return 0;
}
