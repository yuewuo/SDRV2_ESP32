#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"

#define u8 uint8_t
#define u32 uint32_t

//

//#define AnBT_MPU6050_INT			GPIO_Pin_3		//PB3
//#define AnBT_MPU6050_INT_PORT	GPIOB
//
//#define ANBT_MPU6050_INT_STATE   GPIO_ReadInputDataBit(AnBT_MPU6050_INT_PORT, AnBT_MPU6050_INT)
//
#define ANBT_I2C_SDA 	19
#define ANBT_I2C_SCL 	18

//#define ANBT_I2C_PORT   GPIOB
//
#define ANBT_I2C_SCL_0 	    gpio_set_level(ANBT_I2C_SCL, 0)	//GPIO_ResetBits(ANBT_I2C_PORT, ANBT_I2C_SCL)
#define ANBT_I2C_SCL_1 		gpio_set_level(ANBT_I2C_SCL, 1)//GPIO_SetBits(ANBT_I2C_PORT, ANBT_I2C_SCL)
#define ANBT_I2C_SDA_0 		gpio_set_level(ANBT_I2C_SDA, 0)//GPIO_ResetBits(ANBT_I2C_PORT, ANBT_I2C_SDA)
#define ANBT_I2C_SDA_1   	gpio_set_level(ANBT_I2C_SDA, 1)//GPIO_SetBits(ANBT_I2C_PORT, ANBT_I2C_SDA)
//
#define ANBT_I2C_SDA_STATE  gpio_get_level(ANBT_I2C_SDA)	//GPIO_ReadInputDataBit(ANBT_I2C_PORT, ANBT_I2C_SDA)
#define ANBT_I2C_DELAY 		ANBT_I2C_Delay(100000)
#define ANBT_I2C_NOP		ANBT_I2C_Delay(10) 
//
#define ANBT_I2C_READY		0x00
#define ANBT_I2C_BUS_BUSY	0x01	
#define ANBT_I2C_BUS_ERROR	0x02
//
#define ANBT_I2C_NACK	  0x00 
#define ANBT_I2C_ACK		0x01
//
void ANBT_I2C_Configuration(void);
void ANBT_I2C_Delay(u32 dly);
u8 ANBT_I2C_START(void);
void ANBT_I2C_STOP(void);
void ANBT_I2C_SendACK(void);
void ANBT_I2C_SendNACK(void);
u8 ANBT_I2C_SendByte(u8 anbt_i2c_data);
u8 ANBT_I2C_ReceiveByte_WithACK(void);
u8 ANBT_I2C_ReceiveByte(void);
void ANBT_I2C_Receive12Bytes(u8 *anbt_i2c_data_buffer);
void ANBT_I2C_Receive6Bytes(u8 *anbt_i2c_data_buffer);
void ANBT_I2C_Receive14Bytes(u8 *anbt_i2c_data_buffer);
u8 AnBT_DMP_I2C_Write(u8 anbt_dev_addr, u8 anbt_reg_addr, u8 anbt_i2c_len, u8 *anbt_i2c_data_buf);
u8 AnBT_DMP_I2C_Read(u8 anbt_dev_addr, u8 anbt_reg_addr, u8 anbt_i2c_len, u8 *anbt_i2c_data_buf);
void AnBT_DMP_Delay_us(u32 dly);
void AnBT_DMP_Delay_ms(u32 dly);

#define  I2C_Direction_Transmitter      ((uint8_t)0x00)
#define  I2C_Direction_Receiver         ((uint8_t)0x01)
