#ifndef _FILE_SQ_H
#define _FILE_SQ_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f4xx_hal.h"
#include "ufs.h"



typedef struct
{
	uint8_t 	Cmd_id;
	uint16_t 	dataLen;
	uint8_t 	*data;
}FileCmd_t;

typedef union
{
	struct
	{
		uint32_t memSize;
		uint16_t tineWrite;
		uint16_t  maxLen;
		uint8_t  state;
	}param;
	uint8_t raw[8];
}HandShake_p;


typedef union
{
	struct
	{
		uint8_t name[16];
	}param;
	uint8_t raw[21];
}OpenFile_p;
typedef union
{
	struct
	{
		uint16_t NumbPack;
		uint16_t dataLen;
		uint16_t stt;
		ufs_CheckSumStatus CheckSum;
	}param;
	uint8_t raw[7];
}WriteFile_head;

typedef struct
{
	WriteFile_head 	*head;
	uint8_t 		*data;
}WriteFile_p;

typedef union
{
	struct
	{
		uint16_t dataLen;
		uint16_t stt;
		ufs_CheckSumStatus CheckSum;
	}param;
	uint8_t raw[5];
}WriteFileContinue_head;


typedef struct
{
	WriteFileContinue_head 	*head;
	uint8_t 				*data;
}WriteFileContinue_p;

typedef struct
{
	uint32_t offset;
	uint16_t length;
	uint8_t name[16];

}Readfile_t;

typedef void (*SendPacket)(uint8_t *data, uint16_t length);

void FileMng_init(void);
void ServiceHandle(uint8_t *data, uint16_t length);
void respond_addEvent(SendPacket callback);

#ifdef __cplusplus
}
#endif

#endif
