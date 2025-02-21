#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "llnet.h"

#define CRC8_TABLELENGTH 256u

/*
 * Predefined CRC8 lookup table based on SAE J1850 standard
 * This table will be used to compute CRC8 values more efficiently.
 */
static const uint8_t CRC8_J1850_TABLE[CRC8_TABLELENGTH] =
{
    0x00 , 0x1D , 0x3A , 0x27 , 0x74 , 0x69 , 0x4E , 0x53 ,
    0xE8 , 0xF5 , 0xD2 , 0xCF , 0x9C , 0x81 , 0xA6 , 0xBB ,
    0xCD , 0xD0 , 0xF7 , 0xEA , 0xB9 , 0xA4 , 0x83 , 0x9E ,
    0x25 , 0x38 , 0x1F , 0x02 , 0x51 , 0x4C , 0x6B , 0x76 ,
    0x87 , 0x9A , 0xBD , 0xA0 , 0xF3 , 0xEE , 0xC9 , 0xD4 ,
    0x6F , 0x72 , 0x55 , 0x48 , 0x1B , 0x06 , 0x21 , 0x3C ,
    0x4A , 0x57 , 0x70 , 0x6D , 0x3E , 0x23 , 0x04 , 0x19 ,
    0xA2 , 0xBF , 0x98 , 0x85 , 0xD6 , 0xCB , 0xEC , 0xF1 ,
    0x13 , 0x0E , 0x29 , 0x34 , 0x67 , 0x7A , 0x5D , 0x40 ,
    0xFB , 0xE6 , 0xC1 , 0xDC , 0x8F , 0x92 , 0xB5 , 0xA8 ,
    0xDE , 0xC3 , 0xE4 , 0xF9 , 0xAA , 0xB7 , 0x90 , 0x8D ,
    0x36 , 0x2B , 0x0C , 0x11 , 0x42 , 0x5F , 0x78 , 0x65 ,
    0x94 , 0x89 , 0xAE , 0xB3 , 0xE0 , 0xFD , 0xDA , 0xC7 ,
    0x7C , 0x61 , 0x46 , 0x5B , 0x08 , 0x15 , 0x32 , 0x2F ,
    0x59 , 0x44 , 0x63 , 0x7E , 0x2D , 0x30 , 0x17 , 0x0A ,
    0xB1 , 0xAC , 0x8B , 0x96 , 0xC5 , 0xD8 , 0xFF , 0xE2 ,
    0x26 , 0x3B , 0x1C , 0x01 , 0x52 , 0x4F , 0x68 , 0x75 ,
    0xCE , 0xD3 , 0xF4 , 0xE9 , 0xBA , 0xA7 , 0x80 , 0x9D ,
    0xEB , 0xF6 , 0xD1 , 0xCC , 0x9F , 0x82 , 0xA5 , 0xB8 ,
    0x03 , 0x1E , 0x39 , 0x24 , 0x77 , 0x6A , 0x4D , 0x50 ,
    0xA1 , 0xBC , 0x9B , 0x86 , 0xD5 , 0xC8 , 0xEF , 0xF2 ,
    0x49 , 0x54 , 0x73 , 0x6E , 0x3D , 0x20 , 0x07 , 0x1A ,
    0x6C , 0x71 , 0x56 , 0x4B , 0x18 , 0x05 , 0x22 , 0x3F ,
    0x84 , 0x99 , 0xBE , 0xA3 , 0xF0 , 0xED , 0xCA , 0xD7 ,
    0x35 , 0x28 , 0x0F , 0x12 , 0x41 , 0x5C , 0x7B , 0x66 ,
    0xDD , 0xC0 , 0xE7 , 0xFA , 0xA9 , 0xB4 , 0x93 , 0x8E ,
    0xF8 , 0xE5 , 0xC2 , 0xDF , 0x8C , 0x91 , 0xB6 , 0xAB ,
    0x10 , 0x0D , 0x2A , 0x37 , 0x64 , 0x79 , 0x5E , 0x43 ,
    0xB2 , 0xAF , 0x88 , 0x95 , 0xC6 , 0xDB , 0xFC , 0xE1 ,
    0x5A , 0x47 , 0x60 , 0x7D , 0x2E , 0x33 , 0x14 , 0x09 ,
    0x7F , 0x62 , 0x45 , 0x58 , 0x0B , 0x16 , 0x31 , 0x2C ,
    0x97 , 0x8A , 0xAD , 0xB0 , 0xE3 , 0xFE , 0xD9 , 0xC4
};

/*
 * Function: LiteLink_CalCrc8
 * --------------------------
 * This function calculates the CRC8 checksum based on the SAE J1850 standard.
 * It uses a lookup table (CRC8_J1850_TABLE) to speed up the computation.
 *
 *  InitialValue: Initial CRC8 value
 *  u8DataPtr: Pointer to the data array for CRC calculation
 *  length: Length of the data array
 *
 *  returns: Calculated CRC8 value
 */
uint8_t LiteLink_CalCrc8( uint8_t InitialValue,  uint8_t* u8DataPtr, uint16_t length)
{
    uint8_t crc = InitialValue;

    /* Iterate through each byte in the data array */
    for (uint16_t idx = 0; idx < length; idx++)
    {
        /* Update the CRC using the lookup table */
        crc = CRC8_J1850_TABLE[crc ^ (u8DataPtr[idx & 0xFF])];
    }

    /* XOR the final CRC value with the initial value to get the result */
    crc ^= InitialValue;

    return crc;
}

/*
 * Function: LiteLink_newObjList
 * ------------------------------
 * This function creates a new object list with the specified initial capacity.
 * The list is dynamically allocated and initialized.
 *
 *  initialCapacity: The maximum number of objects the list can hold
 *
 *  returns: Pointer to the newly created object list
 */
LiteLink_ObjList* LiteLink_newObjList(uint16_t initialCapacity)
{
    LiteLink_ObjList* obj_list = (LiteLink_ObjList*)malloc(sizeof(LiteLink_ObjList));
    obj_list->objects = (LiteLink_Object*)malloc(initialCapacity * sizeof(LiteLink_Object));
    obj_list->count = 0;
    obj_list->capacity = initialCapacity;

    /* Initialize each object in the list */
    for (uint16_t i = 0; i < initialCapacity; ++i)
    {
        obj_list->objects[i].data = NULL;
        obj_list->objects[i].length = 0;
    }

    return obj_list;
}

/*
 * Function: LiteLink_AddObject
 * -----------------------------
 * This function adds a new object to the object list. If an empty slot is found,
 * the object is added and its index is returned. Otherwise, the function returns an error.
 *
 *  obj_list: Pointer to the object list
 *  data: Pointer to the data to be stored in the object
 *  length: Length of the data
 *  index: Output parameter to hold the index of the added object
 *
 *  returns: LL_OK if the object was added successfully, LL_NOT_OK otherwise
 */
LiteLink_ReturnType LiteLink_AddObject(LiteLink_ObjList* obj_list, void* data, uint16_t length, uint16_t index)
{
    /* Check each slot in the list for an empty position */

	if (obj_list->objects[index].data == NULL)
	{
		/* Allocate memory and copy the data into the object */
		obj_list->objects[index].data = malloc(length);
		if (obj_list->objects[index].data == NULL)
		{
			return LL_NOT_OK;
		}

		memcpy(obj_list->objects[index].data, data, length);
		obj_list->objects[index].length = length;
		obj_list->count++;

		return LL_OK;
    }

    return LL_NOT_OK;
}

/*
 * Function: LiteLink_GetObjectData
 * ---------------------------------
 * This function retrieves the data of the object at the specified index.
 * The data is copied into the provided memory buffer.
 *
 *  obj_list: Pointer to the object list
 *  index: Index of the object in the list
 *  data: Output buffer to store the object's data
 *  length: Length of the data to be copied
 *
 *  returns: LL_OK if the data was retrieved successfully, LL_NOT_OK otherwise
 */
LiteLink_ReturnType LiteLink_GetObjectData(LiteLink_ObjList* obj_list, uint16_t index, void* data, uint16_t length)
{
    if (index >= obj_list->capacity || obj_list->objects[index].data == NULL)
    {
        return LL_NOT_OK;  // No data at the requested index
    }

    /* Copy the object's data into the provided buffer */
    memcpy(data, obj_list->objects[index].data, length);

    return LL_OK;
}

/*
 * Function: LiteLink_GetObjectPtr
 * ---------------------------------
 * This function retrieves the data of the object at the specified index.
 * The data is copied into the provided memory buffer.
 *
 *  obj_list: Pointer to the object list
 *  index: Index of the object in the list
 *  data: Output buffer to store the object's data
 *
 *  returns: LL_OK if the data was retrieved successfully, LL_NOT_OK otherwise
 */
void *LiteLink_GetObjectPtr(LiteLink_ObjList* obj_list, uint16_t index)
{
    if (index >= obj_list->capacity || obj_list->objects[index].data == NULL)
    {
        return NULL;  // No data at the requested index
    }

    return obj_list->objects[index].data;
}

/*
 * Function: LiteLink_GetObjectLength
 * -----------------------------------
 * This function retrieves the length of the object data at the specified index.
 *
 *  obj_list: Pointer to the object list
 *  index: Index of the object in the list
 *
 *  returns: Length of the object data, or 0 if no object is found
 */
uint16_t LiteLink_GetObjectLength(LiteLink_ObjList* obj_list, uint16_t index)
{
    if (index >= obj_list->capacity || obj_list->objects[index].data == NULL)
    {
        return 0;  // No object found at the specified index
    }
    return obj_list->objects[index].length;
}

/*
 * Function: LiteLink_MergeObjects
 * --------------------------------
 * This function merges all objects in the list into the first object.
 * All data from other objects is concatenated into the first object,
 * and the remaining objects are cleared.
 *
 *  obj_list: Pointer to the object list
 *
 *  returns: LL_OK if merge is successful, LL_NOT_OK otherwise
 */
LiteLink_ReturnType LiteLink_MergeObjects(LiteLink_ObjList* obj_list)
{
    if (obj_list->count == 0)
    {
        return LL_NOT_OK;  // No objects to merge
    }

    uint16_t total_length = 0;

    /* Calculate total length of all objects' data */
    for (uint16_t i = 0; i < obj_list->capacity; ++i)
    {
        if (obj_list->objects[i].data != NULL)
        {
            total_length += obj_list->objects[i].length;
        }
    }

    /* Allocate memory for the merged data */
    void* merged_data = malloc(total_length);
    if (merged_data == NULL)
    {
        return LL_NOT_OK;  // Memory allocation failure
    }

    uint16_t offset = 0;

    /* Copy data from all objects into the merged buffer */
    for (uint16_t i = 0; i < obj_list->capacity; ++i)
    {
        if (obj_list->objects[i].data != NULL)
        {
            memcpy((uint8_t*)merged_data + offset, obj_list->objects[i].data, obj_list->objects[i].length);
            offset += obj_list->objects[i].length;

            /* Free data for non-first objects */
            if (i != 0)
            {
                free(obj_list->objects[i].data);
                obj_list->objects[i].data = NULL;
                obj_list->objects[i].length = 0;
            }
        }
    }

    /* Set the merged data to the first object */
    free(obj_list->objects[0].data);
    obj_list->objects[0].data = merged_data;
    obj_list->objects[0].length = total_length;

    /* Update object count to 1 */
    obj_list->count = 1;

    return LL_OK;
}

/*
 * Function: LiteLink_DeleteObj
 * ----------------------------
 * This function deletes an object from the object list at the specified index.
 * It frees the memory used by the object's data and resets its length.
 *
 *  obj_list: Pointer to the object list
 *  index: Index of the object to delete
 *
 *  returns: LL_OK if the object was deleted successfully, LL_NOT_OK otherwise
 */
LiteLink_ReturnType LiteLink_DeleteObj(LiteLink_ObjList* obj_list, uint16_t index)
{
    if (index >= obj_list->capacity || obj_list->objects[index].data == NULL)
    {
        return LL_NOT_OK;  // Invalid index or no data to delete
    }

    /* Free the memory of the object's data */
    free(obj_list->objects[index].data);
    obj_list->objects[index].data = NULL;
    obj_list->objects[index].length = 0;
    obj_list->count--;

    return LL_OK;
}

/*
 * Function: LiteLink_Dispose
 * ----------------------------
 * This function disposes of an object list by freeing all allocated memory
 * for the objects and their data.
 *
 *  obj_list: Pointer to the object list to be disposed
 */
void LiteLink_ObjListDispose(LiteLink_ObjList* obj_list)
{
    /* Free each object's data */
    for (uint16_t i = 0; i < obj_list->capacity; ++i)
    {
        free(obj_list->objects[i].data);
    }

    /* Free the object list and its structure */
    free(obj_list->objects);
    free(obj_list);
}

/*
 * Function: LiteLink_newFifo
 * --------------------------
 * This function initializes a new FIFO with the specified capacity.
 * The FIFO is circular, meaning the head and tail wrap around.
 *
 *  capacity: The maximum number of objects the FIFO can hold
 *
 *  returns: Pointer to the newly created FIFO
 */
LiteLink_Fifo* LiteLink_newFifo(uint16_t capacity)
{
    LiteLink_Fifo* fifo = (LiteLink_Fifo*)malloc(sizeof(LiteLink_Fifo));
    fifo->objects = (LiteLink_Object*)malloc(capacity * sizeof(LiteLink_Object));
    fifo->count = 0;
    fifo->capacity = capacity;
    fifo->head = 0;
    fifo->tail = 0;
    fifo->lock = LL_UNLOCKED;
    /* Initialize each object in the FIFO */
    for (uint16_t i = 0; i < capacity; ++i)
    {
        fifo->objects[i].data = NULL;
        fifo->objects[i].length = 0;
    }

    return fifo;
}

/*
 * Function: LiteLink_FifoEnqueue
 * ------------------------------
 * This function adds a new object to the tail of the FIFO.
 * If the FIFO is full, it returns an error.
 *
 *  fifo: Pointer to the FIFO
 *  data: Pointer to the data to be stored in the object
 *  length: Length of the data
 *
 *  returns: LL_OK if the object was added successfully, LL_NOT_OK if the FIFO is full
 */
LiteLink_ReturnType LiteLink_FifoEnqueue(LiteLink_Fifo* fifo, void* data, uint16_t length)
{
    if (fifo->count == fifo->capacity || (fifo->lock & LL_CORE_LOCKED) == LL_CORE_LOCKED)
    {
        return LL_NOT_OK;  // FIFO is full, cannot add object
    }

    fifo->lock |= LL_CORE_LOCKED;

    /* Allocate memory and copy the data into the object at the tail */
    fifo->objects[fifo->tail].data = malloc(length);
    if (fifo->objects[fifo->tail].data == NULL)
    {
    	fifo->lock &= ~LL_CORE_LOCKED;
        return LL_NOT_OK;  // Memory allocation failure
    }

    memcpy(fifo->objects[fifo->tail].data, data, length);
    fifo->objects[fifo->tail].length = length;

    /* Move the tail to the next position in the circular buffer */
    fifo->tail = (fifo->tail + 1) % fifo->capacity;
    fifo->count++;

    fifo->lock &= ~LL_CORE_LOCKED;
    return LL_OK;
}

/*
 * Function: LiteLink_FifoDequeue
 * ------------------------------
 * This function removes an object from the head of the FIFO.
 * The object's data is copied into the provided buffer.
 * If the FIFO is empty, it returns an error.
 *
 *  fifo: Pointer to the FIFO
 *  data: Output buffer to store the dequeued object's data
 *  length: Pointer to the length of the dequeued object's data
 *
 *  returns: LL_OK if the object was dequeued successfully, LL_NOT_OK if the FIFO is empty
 */
LiteLink_ReturnType LiteLink_FifoDequeue(LiteLink_Fifo* fifo, void* data, uint16_t* length)
{
    if (fifo->count == 0 || (fifo->lock & LL_CORE_LOCKED) == LL_CORE_LOCKED)
    {
        return LL_NOT_OK;  // FIFO is empty, cannot dequeue object
    }

    fifo->lock |= LL_CORE_LOCKED;
    /* Copy the object's data from the head */
    *length = fifo->objects[fifo->head].length;
    memcpy(data, fifo->objects[fifo->head].data, *length);

    /* Free the memory of the dequeued object */
    free(fifo->objects[fifo->head].data);
    fifo->objects[fifo->head].data = NULL;
    fifo->objects[fifo->head].length = 0;

    /* Move the head to the next position in the circular buffer */
    fifo->head = (fifo->head + 1) % fifo->capacity;
    fifo->count--;
    fifo->lock &= ~LL_CORE_LOCKED;
    return LL_OK;
}

/*
 * Function: LiteLink_FifoIsEmpty
 * ------------------------------
 * This function checks if the FIFO is empty.
 *
 *  fifo: Pointer to the FIFO
 *
 *  returns: 1 if the FIFO is empty, 0 otherwise
 */
uint8_t LiteLink_FifoIsEmpty(LiteLink_Fifo* fifo)
{
    return fifo->count == 0;
}

/*
 * Function: LiteLink_DisposeFifo
 * ------------------------------
 * This function frees the memory associated with the FIFO and its objects.
 *
 *  fifo: Pointer to the FIFO to be disposed
 */
void LiteLink_DisposeFifo(LiteLink_Fifo* fifo)
{
    /* Free each object's data */
    for (uint16_t i = 0; i < fifo->capacity; ++i)
    {
        free(fifo->objects[i].data);
    }

    /* Free the object list and the FIFO structure */
    free(fifo->objects);
    free(fifo);
}
