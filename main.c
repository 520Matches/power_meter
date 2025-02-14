#include "core_cm3.h"

#include "usb_type.h"
#include "usb_core.h"
#include "usb_pwr.h"
#include "usb_init.h"

#include "hw_config.h"

__IO uint8_t PrevXferComplete = 1;

int main(void)
{
	Set_System();
	USB_Interrupts_Config();
	Set_USBClock();
	USB_Init();

	while(1)
	{
		if(bDeviceState == CONFIGURED)
		{
			if(PrevXferComplete)
			{
				// RHIDCheckState();
			}
		}
	}
    return 0;
}
