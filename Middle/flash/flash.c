#include "flash.h"

// Địa chỉ bắt đầu của từng sector Flash trên STM32F4
const uint32_t FLASH_SECTOR_ADDR[] = {
    0x08000000, // Sector 0, 16 KB
    0x08004000, // Sector 1, 16 KB
    0x08008000, // Sector 2, 16 KB
    0x0800C000, // Sector 3, 16 KB
    0x08010000, // Sector 4, 64 KB
    0x08020000, // Sector 5, 128 KB
    0x08040000, // Sector 6, 128 KB
    0x08060000, // Sector 7, 128 KB
    0x08080000, // Sector 8, 128 KB
    0x080A0000, // Sector 9, 128 KB
    0x080C0000, // Sector 10, 128 KB
    0x080E0000  // Sector 11, 128 KB
};

// Kích thước của từng sector Flash tương ứng
const uint32_t FLASH_SECTOR_SIZE[] = {
    0x4000,  // Sector 0, 16 KB
    0x4000,  // Sector 1, 16 KB
    0x4000,  // Sector 2, 16 KB
    0x4000,  // Sector 3, 16 KB
    0x10000, // Sector 4, 64 KB
    0x20000, // Sector 5, 128 KB
    0x20000, // Sector 6, 128 KB
    0x20000, // Sector 7, 128 KB
    0x20000, // Sector 8, 128 KB
    0x20000, // Sector 9, 128 KB
    0x20000, // Sector 10, 128 KB
    0x20000  // Sector 11, 128 KB
};

typedef void (*pFunction)(void);
pFunction JumpToApplication;
uint32_t JumpAddress;

// Địa chỉ bắt đầu của ứng dụng trong Flash
#define APPLICATION_ADDRESS 0x080C0000


void Bootloader_JumpToApplication(void) {
    // Disable interrupts
	HAL_RCC_DeInit();

	HAL_DeInit();


	__disable_irq();
	SCB->SHCSR &= ~(SCB_SHCSR_USGFAULTENA_Msk |
	                SCB_SHCSR_BUSFAULTENA_Msk |
	                SCB_SHCSR_MEMFAULTENA_Msk);

    uint32_t jumpAddress = *(__IO uint32_t*)(0x080C0000 + 4);

    // Get the reset handler of the application
    JumpToApplication = (pFunction) jumpAddress;

    // Set the stack pointer
    __set_MSP(*(__IO uint32_t*)0x080C0000);

    SCB->VTOR = 0x080C0000;
      __enable_irq();
    __set_CONTROL(__get_CONTROL() & ~CONTROL_SPSEL_Msk);

    // Jump to application
    JumpToApplication();
}


HAL_StatusTypeDef Flash_erase(uint32_t sector)
{
	HAL_FLASH_Unlock();

	// Xóa sector dựa trên địa chỉ bắt đầu và kích thước dữ liệu
	FLASH_EraseInitTypeDef EraseInitStruct;
	uint32_t SectorError;
	EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;



	EraseInitStruct.Sector = sector;
	EraseInitStruct.NbSectors = 1;

	// Thực hiện xóa sector
	if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
		// Xử lý lỗi nếu không xóa được
		HAL_FLASH_Lock();
		return HAL_ERROR;
	}
	HAL_FLASH_Lock();
	return HAL_OK;
}

HAL_StatusTypeDef Flash_Write(uint32_t start_address, uint8_t *data, uint32_t size) {
    // Mở khóa Flash
	HAL_FLASH_Unlock();

    // Ghi dữ liệu theo từng khối 32-bit
    for (uint32_t i = 0; i < size; i += 4) {
        uint32_t data32 = *(uint32_t*)&data[i];
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, start_address + i, data32) != HAL_OK) {
            // Xử lý lỗi nếu ghi không thành công
            HAL_FLASH_Lock();
            return HAL_ERROR;
        }
    }

    // Khóa lại Flash
    HAL_FLASH_Lock();
    return HAL_OK;
}


void Flash_Read(uint32_t start_address, uint8_t *buffer, uint32_t size) {
    // Đọc dữ liệu byte-by-byte từ Flash
    for (uint32_t i = 0; i < size; i++) {
        buffer[i] = *(uint8_t*)(start_address + i);
    }
}
