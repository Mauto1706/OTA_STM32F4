#ifndef __MAINAPP_H
#define __MAINAPP_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

#include "main.h"
#include "usb_device.h"
#include "MemFlash.h"
#include "llnet.h"
#include "buffer.h"
#include "file_SQ.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "queue.h"
#include "semphr.h"
#include "event_groups.h"

#define ADDR_PARNER	20
#define ADDR_WLAN	1
#define MAX_LENGTH	64
#define MAX_MESSAGE	100

typedef struct
{
	uint8_t length;
	uint8_t data[MAX_LENGTH];
}message;

typedef struct
{
	message _Mess[MAX_MESSAGE];
	int8_t head;
	int8_t tail;
	int8_t count;
}container;

extern container Usb_containData;

void Add_toContain(container *_contain, uint8_t *data, uint16_t length);
container newContainer(void);
uint8_t Get_toContain(container *_contain, uint8_t *data, uint16_t *length);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
