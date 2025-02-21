#include "file_SQ.h"
#include "stdlib.h"
#include "string.h"
#include "flash.h"
#include <time.h>
void Respond(uint8_t *data, uint16_t len);

typedef void (*Service_Funct)(void);



SendPacket SendPacket_callback;

void Service_Handshake(void);
void Service_Listfile(void);
void Service_AccessFolder(void);
void Service_OpenFile(void);
void Service_WriteFirstPacket(void);
void Service_WriteContinue(void);
void Service_ReadFile(void);
void Service_ReadAllfile(void);
void Service_DeleteFile(void);
void Service_RealName(void);
void Service_WriteFlash(void);
void jumb(void);

Service_Funct Service[12] = {Service_Handshake, Service_Listfile, Service_AccessFolder,
							Service_OpenFile, Service_WriteFirstPacket,
							Service_WriteContinue, Service_ReadFile, Service_ReadAllfile,
							Service_DeleteFile, Service_RealName, Service_WriteFlash, jumb};

HandShake_p Handshake_infor;
FileCmd_t Filecmd;
UFS * Ufs;
ufs_Item_Type item;
ufs_ItemInfo_Type item_info[10];
uint32_t datafile[200] = {0};

void FileMng_init(void)
{
	Ufs = newUFS(&Ufs_Cfg);
//	ufs_FastFormat(Ufs);
	Handshake_infor.param.memSize = Ufs->conf->api->u32numberSectorOfDevice * Ufs->conf->api->u16numberByteOfSector;
	Handshake_infor.param.maxLen = Ufs->conf->api->u16numberByteOfSector ;
	Handshake_infor.param.tineWrite = 0;

	Filecmd.data = NULL;
	Filecmd.dataLen = 0;
}

void ServiceHandle(uint8_t *data, uint16_t length)
{
	Filecmd.Cmd_id = data[0];
	Filecmd.dataLen = length - 1;
	Filecmd.data = &data[1];

	Service[Filecmd.Cmd_id]();
//	Filecmd.data = NULL;
}

void Service_Handshake(void)
{

	Handshake_infor.param.memSize = Handshake_infor.param.memSize - ufs_GetUsedSize(Ufs);
	Handshake_infor.param.state = UFS_OK;
	Respond(Handshake_infor.raw, 8);
}

void Service_AccessFolder(void)
{
	uint8_t Ret = UFS_OK;
	uint8_t lenname = Filecmd.data[0];
	uint8_t namepath[16] = {0};
	memcpy(namepath, &Filecmd.data[1], lenname);
	if(ufs_Mount(Ufs, namepath) != UFS_OK)
	{
		Ret = UFS_NOT_OK;
	}

	Respond(&Ret, 1);
}

void Service_OpenFile(void)
{
	uint8_t Ret[5] = {UFS_NOT_OK};

	uint8_t lenname = Filecmd.data[0];
	uint8_t nameFile[16] = {0};
	memcpy(nameFile, &Filecmd.data[1], lenname);

	if(ufs_OpenItem(Ufs, nameFile , &item) == UFS_OK)
	{
		Ret[0] = UFS_OK;
	}
	Ret[1] = item.info.comp.size & 0xFF;
	Ret[2] = (item.info.comp.size >> 8) & 0xFF;
	Ret[3] = (item.info.comp.size >> 16) & 0xFF;
	Ret[4] = item.info.comp.size >> 24;

	Respond(Ret, 5);
}

double time_spent[2];
uint16_t stt = 0;

void Service_WriteFirstPacket(void)
{
	uint8_t Ret = UFS_OK;
	WriteFile_p infor_write;
	infor_write.head = (WriteFile_head*)Filecmd.data;
	infor_write.data = &Filecmd.data[7];
	stt = 0;

	if(ufs_WriteFile(&item, infor_write.data, infor_write.head->param.dataLen, CHECKSUM_ENABLE) != UFS_OK)
	{
		Ret = item.err;
	}

	Respond(&Ret, 1);
}

void Service_WriteContinue(void)
{
	uint8_t Ret = UFS_OK;
	WriteFileContinue_p infor_write;
	infor_write.head = (WriteFileContinue_head*)Filecmd.data;
	infor_write.data = &Filecmd.data[5];
	if( ++ stt == infor_write.head->param.stt)
	{
		if(ufs_WriteAppendFile(&item, infor_write.data, infor_write.head->param.dataLen, CHECKSUM_ENABLE) != UFS_OK)
		{
			Ret = item.err;
		}
	}
	Respond(&Ret, 1);
}

void Service_ReadFile(void)
{

	Readfile_t *InforRead = (Readfile_t *)Filecmd.data;
	uint32_t reallen_read = 0;
	uint8_t *data = (uint8_t *)malloc(InforRead->length + 5);
	data[0] = UFS_OK;


	reallen_read = ufs_ReadFile(&item, InforRead->offset, &data[5], InforRead->length);
	data[1] = reallen_read >> 24;
	data[2] = (reallen_read >> 16) & 0xFF;
	data[3] = (reallen_read >> 8) & 0xFF;
	data[4] = reallen_read & 0xFF;


	Respond(data, InforRead->length + 5);

	InforRead = NULL;
	free(data);
	data = NULL;
}

void Service_ReadAllfile(void)
{
	Readfile_t *InforRead = (Readfile_t *)Filecmd.data;

//	uint8_t *data = (uint8_t *)malloc(Handshake_infor.param.maxLen + 2);
	uint8_t data[4096];
	data[0] = UFS_OK;

	if(ufs_OpenItem(Ufs, InforRead->name , &item) != UFS_OK)
	{
		data[0] = UFS_NOT_OK;
		Respond(data, 1);
	}

	uint16_t numpack = item.info.comp.size / Handshake_infor.param.maxLen;
	uint16_t lastlen = item.info.comp.size - numpack * Handshake_infor.param.maxLen;

	if(lastlen > 0)
	{
		numpack ++;
	}
	for(uint16_t count = 0; count < numpack - 1; count ++)
	{
		data[1] = count;
		ufs_ReadFile(&item, count * Handshake_infor.param.maxLen, &data[2], count * Handshake_infor.param.maxLen + Handshake_infor.param.maxLen);
		Respond(data, Handshake_infor.param.maxLen + 2);
	}
	data[0] = UFS_NOT_OK;
	data[1] = numpack - 1;
	ufs_ReadFile(&item, (numpack - 1) * Handshake_infor.param.maxLen, &data[2], (numpack - 1) * Handshake_infor.param.maxLen + lastlen);
	Respond(data, lastlen + 2);

//	free(data);
//	data = NULL;
}


void Service_Listfile(void)
{
	uint8_t numberItem[2] = {0};
	numberItem[1] = ufs_GetListItem(Ufs, item_info, 0xFFFF);
	Respond(numberItem, 2);
	uint8_t namefile[20]= {1};
	namefile[1] = item_info[0].comp.name.head[0];
	if(item_info[0].comp.name.length == 0)
	{
		Respond( namefile, 2);
	}
	else
	{
		memcpy(&namefile[2], item_info[0].comp.name.head, item_info[0].comp.name.length);
		Respond( namefile, item_info[0].comp.name.length + 2);
	}
	for(uint16_t count = 1; count < numberItem[1]; count ++)
	{
		memcpy(&namefile[1], item_info[count].comp.name.head, item_info[count].comp.name.length);
		if(item_info[count].comp.name.extention[0] != 0)
		{
			memcpy(&namefile[item_info[count].comp.name.length + 1], (uint8_t *)".", 1);
			memcpy(&namefile[item_info[count].comp.name.length + 2], item_info[count].comp.name.extention, 3);
			Respond( namefile, item_info[count].comp.name.length + 5);
		}
		else
		{
			Respond( namefile, item_info[count].comp.name.length + 1);
		}




	}
}

void Service_DeleteFile(void)
{
	uint8_t Ret[5] = {UFS_NOT_OK};
	uint8_t lenname = Filecmd.data[0];
	uint8_t nameFile[16] = {0};
	memcpy(nameFile, &Filecmd.data[1], lenname);
	if(ufs_OpenItem(Ufs, nameFile , &item) == UFS_OK)
	{
		if(ufs_DeleteItem(&item) == UFS_OK)
		{
			Ret[0] = UFS_OK;
		}
	}

	Respond(Ret, 1);
}
void Service_RealName(void)
{
	uint8_t Ret[5] = {UFS_NOT_OK};
	uint8_t nameFile[16] = {0};
	uint8_t newName[16] = {0};
	uint8_t len_nameFile = Filecmd.data[0];
	uint8_t len_newName = Filecmd.data[len_nameFile + 1];

	memcpy(nameFile, &Filecmd.data[1], len_nameFile);
	memcpy(newName, &Filecmd.data[len_nameFile + 2], len_newName);
	if(ufs_OpenItem(Ufs, nameFile , &item) == UFS_OK)
	{
		if(ufs_RenameItem(&item, newName) == UFS_OK)
		{
			Ret[0] = UFS_OK;
		}
	}
	Respond(Ret, 1);
}


void Service_WriteFlash(void)
{
	uint8_t data[2048];
	uint8_t percent = 0;
	uint32_t total_len = item.info.comp.size;
	uint32_t lenRead = 2048;
	uint32_t lenWrite = 0;
	uint32_t offset = 0;
	uint32_t addr_write = ADDR_START;
	Flash_erase(SECTOR_START);
	do{
		lenWrite = ufs_ReadFile(&item, offset, data, lenRead);
		Flash_Write(addr_write, data, lenWrite);
		addr_write += lenWrite;
		offset += lenWrite;
		if(total_len - offset < lenRead)
		{
			lenRead = total_len - offset;
		}
		percent = (offset * 100 )/ total_len;
		Respond(&percent, 1);
	}while(offset != total_len);
}

void jumb(void)
{
	Bootloader_JumpToApplication();
}

void Respond(uint8_t *data, uint16_t len)
{
	if(SendPacket_callback != NULL)
	{
		SendPacket_callback(data, len);
	}
}

void respond_addEvent(SendPacket callback)
{
	SendPacket_callback = callback;
}
