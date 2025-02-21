#ifndef __MEMFLASH_H__
#define __MEMFLASH_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "./cfg/MemFlash_Cfg.h"
#define Std_ReturnType  uint8_t
#define E_OK 0x00


extern Std_ReturnType MemFlash_Init(uint8_t *Id);
extern Std_ReturnType MemFlash_ReadID(uint8_t *data, uint16_t length);
extern Std_ReturnType MemFlash_WriteSector(uint16_t SectorNumb, uint8_t *SectorData, uint16_t SectorSize);
extern Std_ReturnType MemFlash_ReadSector(uint16_t SectorNumb, uint8_t *SectorData, uint16_t SectorSize);
extern Std_ReturnType MemFlash_EraseSector(uint16_t SectorNumb);
extern Std_ReturnType MemFlash_EraseBlock(uint16_t BlockNumb);
extern Std_ReturnType MemFlash_EraseChip();

#ifdef __cplusplus
}
#endif
#endif
