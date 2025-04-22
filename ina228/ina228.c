#include "stm32f10x_i2c.h"

#include "ina228.h"

#define INA228_I2C_ADDR	(0x40 << 1)

#define INA228_SHUNT_CAL	(0x02)

#define INA228_TEMPCO_PPM	(25)


static float current_lsb = 0.0f;
static uint16_t shunt_cal = 0;

static uint32_t bswap32(uint32_t x)
{
    return ((x & 0x000000FFU) << 24) |
           ((x & 0x0000FF00U) << 8)  |
           ((x & 0x00FF0000U) >> 8)  |
           ((x & 0xFF000000U) >> 24);
}

// static int32_t get_current(int32_t num)
// {
// 	int32_t temp = 0;
// 	// 屏蔽掉高8位，只保留低24位
//     num &= 0xFFFFFF;
//  
//     // 检查符号位（第24位，从0开始计数）
//     if(num & 0x800000) { // 如果符号位是1，表示负数
//         // 对补码取反加一得到绝对值（仍然是32位整数，但结果只关注低24位）
//         temp = ~num;
//         temp += 1;
//  
//         // 注意：这里返回的是一个32位整数，但它的高8位是0，低24位表示原码的绝对值。
//         // 如果需要，可以在调用此函数后仅使用返回值的低24位。
//         // 另外，由于这是一个负数，通常我们不会在“原码”表示中直接存储它，
//         // 因为原码通常用于表示正数和负数的符号+绝对值形式，
//         // 但在这里为了符合题目要求，我们返回的是负数的绝对值部分（低24位）。
//         // 要完全恢复为负数的原码表示，你需要在某处手动添加符号位信息。
//  
//         // 由于我们是在模拟24位整数的行为，并且C语言中没有直接的24位整数类型，
//         // 因此这里返回的是一个32位整数，调用者需要知道只有低24位是有效的。
//         return temp; // 返回绝对值的低24位部分（作为32位整数）
//     } else {
//         // 如果符号位是0，表示正数，原码和补码相同
//         return num; // 直接返回补码（也是原码）
//     }
// }

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
	//bit15,bit14,bit6,bit5
	uint8_t config_data[3] = {0x00, 0xC0, 0x30};  // 配置INA228寄存器值
	//MODE:0x0B,VBUSCT:0x06,VSHCT:0x06,VTCT:0x06,AVG:0x02
	uint8_t config_adc[3] = {0x01, 0xBD, 0xB2};
	uint8_t config_shunt_cal[3] = {0x02, 0x00, 0x00};
	//25摄氏度
	uint8_t config_tempco[3] = {0x03, 0x00, 0x00};

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
    I2C_InitStruct.I2C_ClockSpeed = 400000;  // 400kHz
    I2C_Init(I2C2, &I2C_InitStruct);

    // 启用 I2C
    I2C_Cmd(I2C2, ENABLE);

	current_lsb = 0.04096 / 524288.0;

	//1欧分流电阻
	//shunt_cal = 131072 * current_lsb * 100000 * (RSHUNT / 1000.0) * 4;
	shunt_cal = 131072 * current_lsb * 100000 * 4;

	//配置INA228,分流电压为-40.96mv ~ 40.96mv
    ina228_i2c_write(I2C2, INA228_I2C_ADDR, config_data, 3);
	//config ADC
    ina228_i2c_write(I2C2, INA228_I2C_ADDR, config_adc, 3);
	//配置分流电阻 
	config_shunt_cal[1] = shunt_cal >> 8;
	config_shunt_cal[2] = shunt_cal;
    ina228_i2c_write(I2C2, INA228_I2C_ADDR, config_shunt_cal, 3);

	config_tempco[1] = INA228_TEMPCO_PPM >> 8;
	config_tempco[2] = INA228_TEMPCO_PPM;
    ina228_i2c_write(I2C2, INA228_I2C_ADDR, config_tempco, 3);
}

uint32_t ina228_read(ina228_regs_t reg)
{
	// double ret = 0.0;
    uint8_t reg_addr = 0x07;  // 电流寄存器地址
    uint8_t data[3] = {0};
	int32_t read_data = 0;
	uint32_t ret = 0;

	switch(reg)
	{
		case POWER:
			reg_addr = 0x08;
			ina228_i2c_read(I2C2, INA228_I2C_ADDR, reg_addr, data, 3);
			read_data = ((data[0] << 16) | (data[1] << 8) | data[2]) >> 4;
			if(read_data & 0x80000)
			{
				read_data |= 0xFFF00000;
			}
			ret = 0.0000003125 * read_data;
		break;
		case CURRENT:
			reg_addr = 0x07;
			ina228_i2c_read(I2C2, INA228_I2C_ADDR, reg_addr, data, 3);
			read_data = ((data[0] << 16) | (data[1] << 8) | data[2]) >> 4;
			if(read_data & 0x80000)
			{
				read_data |= 0xFFF00000;
			}
			ret = read_data * 409600 / 524288;//0.1ua
			// ret = read_data * current_lsb;
		break;
		case VBUS:
			reg_addr = 0x05;
			ina228_i2c_read(I2C2, INA228_I2C_ADDR, reg_addr, data, 3);
			read_data = (data[0] << 16) | (data[1] << 8) | data[2];
			ret = read_data * 0.001;  // 根据数据手册转换
		break;
		default:
		break;
	}

	return ret;
}
