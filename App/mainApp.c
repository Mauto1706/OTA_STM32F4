#include "mainApp.h"
#include "MemFlash.h"
#include "ufs.h"
#include "usb_device.h"
#define PORT_FILE_PARTNER	54

extern USBD_HandleTypeDef hUsbDeviceFS;
extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);

uint8_t tx_buffer[64];			//Variable to store the output data
uint8_t report_buffer[2048];		//Variable to receive the report buffer
uint8_t flag_rx = 0;			//Variable to store the reception flag
uint16_t len;

TaskHandle_t MCP_handler;
TaskHandle_t USB_handler;
TaskHandle_t FileMng_handler;

void LLnet_Task (void *pvParameters);
void USB_Task(void *pvParameters);
void FileMng_Task (void *pvParameters);

void Event_UsbSend(uint8_t *data, uint8_t length);
void File_service_onMess(LiteLink_Address src_addr, uint8_t *data, uint16_t length);

void Send_respond(uint8_t *data, uint16_t length);

extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;

LiteLink* litelink;
Fifo* usb_send;

LiteLink_Service *File_service;
Fifo * Fifo_File;

container Usb_containData;

uint8_t data[32];
int32_t remain = 0;

void mainApp(void)
{
	litelink = newLiteLink(1, 10, 62, "Vcar",Event_UsbSend);

	usb_send = newFifo(0xFF);

	xTaskCreate(FileMng_Task, "file_manager", 2048, NULL, 7, &FileMng_handler);
	xTaskCreate(LLnet_Task, "ROUTER", 1024, NULL, 7, &MCP_handler);
	xTaskCreate(USB_Task, "USB", 1024, NULL, 8, &USB_handler);

//	HAL_TIM_Base_Start_IT(&htim6);



	while(1)
	{
	}
}

void Event_UsbSend(uint8_t *data, uint8_t length)
{
	CDC_Transmit_FS(data, length);
}



void LLnet_Task (void *pvParameters)
{
//	xTaskCreate(USB_Task, "USB", 10 * 1024, NULL, 8, &USB_handler);
	for( ;; )
	{

		LiteLink_Process(litelink);
		vTaskDelay(1);
	}
}

void USB_Task(void *pvParameters)
{
	uint8_t data[64];
	uint16_t length = 0;
	for(;;)
	{

		if(Get_toContain(&Usb_containData, data, &length) == LL_OK)
		{
			LiteLink_InboundMessage(litelink, data, length);
		}
		vTaskDelay(1);
	}
}


#define PORT_FILE_PARTNER	54

typedef void (*FileCommandFunc_t)(void);

void Mcp_PortFileEstablishedEvent(void *connt);
void File_service_onMess(LiteLink_Address src_addr, uint8_t *data, uint16_t length);

void Send_respond(uint8_t *data, uint16_t length);


Fifo *file_cmdMess;


uint16_t numberitem;

LiteLink_Service *File_service;
Fifo * Fifo_File;


void FileMng_Task (void *pvParameters)
{
	File_service = LiteLink_uListen(litelink, "File", 55);
	LiteLink_onMessage(File_service, File_service_onMess);
	Fifo_File = newFifo(50);

	FileMng_init();

	respond_addEvent(Send_respond);

	uint8_t *data;
	uint16_t length  = 0;
	for( ;; )
	{
		length = Fifo_GetSizeData(Fifo_File);
		if(length > 0)
		{
			data = (uint8_t *)malloc(length);
			if(Fifo_GetData(Fifo_File, data, &length)== BUFF_OK)
			{
				ServiceHandle(data, length);
			}
			free(data);
			data = NULL;
		}


		vTaskDelay(1);

	}
}


void File_service_onMess(LiteLink_Address src_addr, uint8_t *data, uint16_t length)
{
	Fifo_AddData(Fifo_File, data, length);
}


void Send_respond(uint8_t *data, uint16_t length)
{
	if(LiteLink_SendPacket(File_service, ADDR_PARNER, PORT_FILE_PARTNER, data, length) == LL_NOT_OK)
	{
		LiteLink_SendPacket(File_service, ADDR_PARNER, PORT_FILE_PARTNER, data, length);
	}
}



container newContainer(void)
{
	container newContain;
	newContain.head = 0;
	newContain.tail = 0;
	newContain.count = 0;
	for(uint8_t count = 0; count < MAX_MESSAGE; count ++)
	{
		newContain._Mess[count].length = 0;
	}
	return newContain;
}

void Add_toContain(container *_contain, uint8_t *data, uint16_t length)
{

	for(uint8_t count = 0; count < length; count ++)
	{
		_contain->_Mess[_contain->tail].data[count] = data[count];
	}
	_contain->_Mess[_contain->tail].length = length;
	_contain->tail = (_contain->tail + 1) % MAX_MESSAGE;
	if(++_contain->count >= MAX_MESSAGE)
	{
		_contain->count = MAX_MESSAGE - 1;
	}

	if (_contain->tail == _contain->head)
	{
		if (--_contain->tail < 0)
		{
			_contain->tail = MAX_MESSAGE - 1;
		}

	}

}

uint8_t Get_toContain(container *_contain, uint8_t *data, uint16_t *length)
{
	if(_contain->head == _contain->tail)
	{
		return 1;
	}
	*length = _contain->_Mess[_contain->head].length;
	for(uint8_t count = 0; count < *length; count ++)
	{
		data[count] = _contain->_Mess[_contain->head].data[count] ;
	}
	_contain->_Mess[_contain->head].length = 0;
	if(--_contain->count <= 0)
	{
		_contain->count = 0;
	}
	if( ++ _contain->head == MAX_MESSAGE)
	{
		_contain->head = 0;
	}
	return 0;
}
