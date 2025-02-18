#include "stm32f10x_i2c.h"

#include "ina228.h"

#define INA228_I2C_ADDR	(0x40)

static void ina228_i2c_write(I2C_TypeDef *I2Cx, uint8_t dev_addr, uint8_t *data, uint8_t len)
{
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2Cx, ENABLE);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2Cx, dev_addr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    for (uint8_t i = 0; i < len; i++) {
        I2C_SendData(I2Cx, data[i]);
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    }
    I2C_GenerateSTOP(I2Cx, ENABLE);
}

static void ina228_i2c_read(I2C_TypeDef *I2Cx, uint8_t dev_addr, uint8_t reg_addr, uint8_t *data, uint8_t len)
{
    while (I2C_GetFlagStatus(I2Cx, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2Cx, ENABLE);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2Cx, dev_addr, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));
    I2C_SendData(I2Cx, reg_addr);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));
    I2C_GenerateSTART(I2Cx, ENABLE);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_MODE_SELECT));
    I2C_Send7bitAddress(I2Cx, dev_addr, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));
    for (uint8_t i = 0; i < len; i++) {
        if (i == len - 1) {
            I2C_AcknowledgeConfig(I2Cx, DISABLE);
        }
        while (!I2C_CheckEvent(I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));
        data[i] = I2C_ReceiveData(I2Cx);
    }
    I2C_GenerateSTOP(I2Cx, ENABLE);
    I2C_AcknowledgeConfig(I2Cx, ENABLE);
}

void ina228_init(void)
{
	uint8_t config_data[3] = {0x00, 0x80, 0x00};  // 配置INA228寄存器值
												  //
	// 启用 GPIOB 和 I2C1 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2, ENABLE);

    // 配置 PB6 (SCL) 和 PB7 (SDA) 为复用开漏输出
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStruct);

    // 配置 I2C
    I2C_InitTypeDef I2C_InitStruct;
    I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStruct.I2C_OwnAddress1 = 0x00;
    I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
    // I2C_InitStruct.I2C_ClockSpeed = 100000;  // 100kHz
    I2C_InitStruct.I2C_ClockSpeed = 400000;  // 100kHz
    I2C_Init(I2C2, &I2C_InitStruct);

    // 启用 I2C
    I2C_Cmd(I2C2, ENABLE);

	//配置INA228,分流电压为-163.84mv ~ 163.84mv
    ina228_i2c_write(I2C2, INA228_I2C_ADDR, config_data, 3);
}

double ina228_read(ina228_regs_t reg)
{
	double ret = 0.0;
    uint8_t reg_addr = 0x01;  // 电流寄存器地址
    uint8_t data[3];
	int32_t read_data;

	switch(reg)
	{
		case POWER:
			reg_addr = 0x03;
			ina228_i2c_read(I2C2, INA228_I2C_ADDR, reg_addr, data, 3);

			read_data = (data[0] << 16) | (data[1] << 8) | data[2];
			ret = read_data * 0.01;  // 根据数据手册转换
		break;
		case CURRENT:
			reg_addr = 0x01;
			ina228_i2c_read(I2C2, INA228_I2C_ADDR, reg_addr, data, 3);

			read_data = (data[0] << 16) | (data[1] << 8) | data[2];
			ret = read_data * 0.0001;  // 根据数据手册转换
		break;
		case VOLTAGE:
			reg_addr = 0x02;
			ina228_i2c_read(I2C2, INA228_I2C_ADDR, reg_addr, data, 3);

			read_data = (data[0] << 16) | (data[1] << 8) | data[2];
			ret = read_data * 0.001;  // 根据数据手册转换
		break;
		default:
		break;
	}

	return ret;
}
