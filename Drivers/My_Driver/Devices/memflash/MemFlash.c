#include "MemFlash.h"

Std_ReturnType MemFlash_Init(uint8_t *Id)
{
	uint32_t ID = 0;
	W25qxx_Init(&ID);
	*Id = ID & 0xFF;
	return E_OK;
}

Std_ReturnType MemFlash_WriteSector(uint16_t SectorNumb, uint8_t *SectorData, uint16_t SectorSize)
{
	W25qxx_WriteSector(SectorData, SectorNumb, 0, SectorSize);
	return E_OK;
}

Std_ReturnType MemFlash_ReadSector(uint16_t SectorNumb, uint8_t *SectorData, uint16_t SectorSize)
{
	W25qxx_ReadSector(SectorData, SectorNumb, 0, SectorSize);
	return E_OK;
}

Std_ReturnType MemFlash_EraseSector(uint16_t SectorNumb)
{
	W25qxx_EraseSector(SectorNumb);
	return E_OK;
}

Std_ReturnType MemFlash_EraseChip()
{
	W25qxx_EraseChip();
	return E_OK;
}

Std_ReturnType MemFlash_ReadID(uint8_t *data, uint16_t length)
{
	return E_OK;
}

Std_ReturnType MemFlash_EraseBlock(uint16_t BlockNumb)
{
	W25qxx_EraseBlock(BlockNumb);
	return E_OK;
}
