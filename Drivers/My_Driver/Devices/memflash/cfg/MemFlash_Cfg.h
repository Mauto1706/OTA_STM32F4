#ifndef __MEMFLASH_CFG_H__
#define __MEMFLASH_CFG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "Std_Types.h"
#include "stm32f4xx_hal.h"


#define USING_W25QXX     STD_ON

#if(USING_W25QXX == STD_ON)
#include "../hw/w25qxx/W25qxx.h"
#define _W25QXX_USE_FREERTOS          0
#define _W25QXX_DEBUG                 0

extern SPI_HandleTypeDef hspi1;

#define W25QXX_READWRITE(DATA,RET,SIZE)           HAL_SPI_TransmitReceive(&hspi1,DATA,RET,SIZE,100)
#define W25QXX_READ(DATA, SIZE, TIMEOUT)          HAL_SPI_Receive(&hspi1, DATA, SIZE, TIMEOUT)
#define W25QXX_WRITE(DATA, SIZE, TIMEOUT)         HAL_SPI_Transmit(&hspi1, DATA, SIZE, TIMEOUT)
#define W25QXX_CS_OFF()                           HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_RESET)
#define W25QXX_CS_ON()                            HAL_GPIO_WritePin(GPIOB,GPIO_PIN_14,GPIO_PIN_SET)
#endif

#ifdef __cplusplus
}
#endif
#endif
