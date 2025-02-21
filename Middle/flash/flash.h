#ifndef __FLASH_H
#define __FLASH_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "FreeRTOS.h"
#include "task.h"

#define SECTOR_START	10
#define ADDR_START		0x080C0000
#define SECTOR_SIZE		128000


void Bootloader_JumpToApplication(void);
//void JumpToSector10(void);
HAL_StatusTypeDef Flash_erase(uint32_t sector);
HAL_StatusTypeDef Flash_Write(uint32_t start_address, uint8_t *data, uint32_t size);
void Flash_Read(uint32_t start_address, uint8_t *buffer, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
