#ifndef LITELINK_H
#define LITELINK_H

#include <stdint.h>
#include "llnet_types.h"
#include "main.h"

#define LITELINK_DELAY(ms)   HAL_Delay(ms)

/* LiteLink core function declarations */

/**
 * @brief Initializes a new LiteLink instance.
 *
 * @param wlan_address WLAN address for the LiteLink instance.
 * @param local_address Local address for the LiteLink instance.
 * @param segment_size Maximum segment size (must exceed LITELINK_HEADER_SIZE).
 * @param name_device Name of the device to register.
 * @param outbound_event Callback function for outbound events.
 * @return Pointer to the initialized LiteLink instance or NULL if initialization fails.
 */
LiteLink* newLiteLink(uint16_t wlan_address, uint16_t local_address, uint16_t segment_size, const char* name_device, LiteLink_OutboundEvent outbound_event);

/**
 * @brief Starts listening on a specified port for a LiteLink service.
 *
 * @param llnet Pointer to the LiteLink instance.
 * @param name_service Name of the service to register.
 * @param port The port to listen on (must be less than 0x3F).
 * @return Pointer to the LiteLink service instance or NULL on failure.
 */
LiteLink_Service *LiteLink_uListen(LiteLink *llnet, const char *name_service, uint8_t port);

/**
 * @brief Registers a message event handler for a LiteLink service.
 *
 * @param service Pointer to the LiteLink service.
 * @param event The message event handler to register.
 */
void LiteLink_onMessage(LiteLink_Service* service, LiteLink_ReceivedEvent event);

/**
 * @brief Updates the outbound event callback function of a LiteLink instance.
 *
 * @param llnet Pointer to the LiteLink instance.
 * @param event New outbound event callback function.
 */
void LiteLink_OutboundEventUpdate(LiteLink* llnet, LiteLink_OutboundEvent event);

/**
 * @brief Closes a LiteLink service on the specified port.
 *
 * @param llnet Pointer to the LiteLink instance.
 * @param port The port of the service to close.
 */
void LiteLink_uClose(LiteLink *llnet, uint8_t port);

/**
 * @brief Processes an inbound message by parsing and updating headers.
 *
 * @param llnet Pointer to the LiteLink instance.
 * @param data Received data to process.
 * @param length Length of the received data.
 */
void LiteLink_InboundMessage(LiteLink *llnet, uint8_t *data, uint16_t length);

/**
 * @brief Sends a Ping packet to a specified address and waits for a response.
 *
 * This function sends a Ping packet from a LiteLink instance to a target address.
 * It locks the instance during the send operation, sets up the Ping packet with
 * the correct address and control flags, and enqueues it. After sending, it waits
 * for a PONG response up to a timeout limit. If no response is received within the
 * timeout, it returns a failure code.
 *
 * @param llnet Pointer to the LiteLink instance used to send the Ping.
 * @param address Target address to which the Ping packet is sent.
 * @return uint16_t Time waited for the response in ms, or 0xFFFF if the wait exceeds the timeout limit.
 */
uint16_t LiteLink_SendPing(LiteLink* llnet, uint16_t address);

/**
 * @brief Sends a packet by dividing data into segments and enqueuing them for transmission.
 *
 * @param service Pointer to the LiteLink service to send from.
 * @param address Destination address.
 * @param port Destination port.
 * @param data Pointer to the data to send.
 * @param length Length of the data to send.
 * @return LL_OK if sending was successful, otherwise LL_NOT_OK.
 */
LiteLink_ReturnType LiteLink_SendPacket(LiteLink_Service *service, uint16_t address, uint8_t port, uint8_t *data, uint16_t length);

/**
 * @brief Processes queued segments, validates headers, and triggers events for inbound data.
 *
 * @param llnet Pointer to the LiteLink instance to process segments.
 */
void LiteLink_Process(LiteLink *llnet);


/* Helper function declarations */

/**
 * @brief Calculates an 8-bit CRC for the given data.
 *
 * @param InitialValue Initial CRC value.
 * @param u8DataPtr Pointer to the data.
 * @param length Length of the data.
 * @return Calculated CRC8 value.
 */
uint8_t LiteLink_CalCrc8(uint8_t InitialValue, uint8_t* u8DataPtr, uint16_t length);

/**
 * @brief Creates a new object list for LiteLink.
 *
 * @param initialCapacity Initial capacity of the object list.
 * @return Pointer to the initialized LiteLink_ObjList or NULL on failure.
 */
LiteLink_ObjList* LiteLink_newObjList(uint16_t initialCapacity);

/**
 * @brief Adds an object to a LiteLink object list.
 *
 * @param obj_list Pointer to the LiteLink object list.
 * @param data Pointer to the data to add.
 * @param length Length of the data.
 * @param index Index to insert the object.
 * @return LL_OK if successful, otherwise LL_NOT_OK.
 */
LiteLink_ReturnType LiteLink_AddObject(LiteLink_ObjList* obj_list, void* data, uint16_t length, uint16_t index);

/**
 * @brief Retrieves data of an object at a specified index.
 *
 * @param obj_list Pointer to the LiteLink object list.
 * @param index Index of the object.
 * @param data Output buffer for the object data.
 * @param length Length of the output buffer.
 * @return LL_OK if successful, otherwise LL_NOT_OK.
 */
LiteLink_ReturnType LiteLink_GetObjectData(LiteLink_ObjList* obj_list, uint16_t index, void* data, uint16_t length);

/**
 * @brief Retrieves a pointer to an object at a specified index.
 *
 * @param obj_list Pointer to the LiteLink object list.
 * @param index Index of the object.
 * @return Pointer to the object or NULL if not found.
 */
void *LiteLink_GetObjectPtr(LiteLink_ObjList* obj_list, uint16_t index);

/**
 * @brief Retrieves the length of an object at a specified index.
 *
 * @param obj_list Pointer to the LiteLink object list.
 * @param index Index of the object.
 * @return Length of the object data.
 */
uint16_t LiteLink_GetObjectLength(LiteLink_ObjList* obj_list, uint16_t index);

/**
 * @brief Merges objects in the LiteLink object list into a single object.
 *
 * @param obj_list Pointer to the LiteLink object list.
 * @return LL_OK if successful, otherwise LL_NOT_OK.
 */
LiteLink_ReturnType LiteLink_MergeObjects(LiteLink_ObjList* obj_list);

/**
 * @brief Deletes an object at a specified index from the LiteLink object list.
 *
 * @param obj_list Pointer to the LiteLink object list.
 * @param index Index of the object to delete.
 * @return LL_OK if successful, otherwise LL_NOT_OK.
 */
LiteLink_ReturnType LiteLink_DeleteObj(LiteLink_ObjList* obj_list, uint16_t index);

/**
 * @brief Disposes of a LiteLink object list, freeing its resources.
 *
 * @param obj_list Pointer to the LiteLink object list to dispose of.
 */
void LiteLink_ObjListDispose(LiteLink_ObjList* obj_list);

/**
 * @brief Creates a new FIFO queue for LiteLink.
 *
 * @param capacity Capacity of the FIFO queue.
 * @return Pointer to the initialized LiteLink_Fifo or NULL on failure.
 */
LiteLink_Fifo* LiteLink_newFifo(uint16_t capacity);

/**
 * @brief Enqueues data into a LiteLink FIFO queue.
 *
 * @param fifo Pointer to the LiteLink FIFO.
 * @param data Pointer to the data to enqueue.
 * @param length Length of the data.
 * @return LL_OK if successful, otherwise LL_NOT_OK.
 */
LiteLink_ReturnType LiteLink_FifoEnqueue(LiteLink_Fifo* fifo, void* data, uint16_t length);

/**
 * @brief Dequeues data from a LiteLink FIFO queue.
 *
 * @param fifo Pointer to the LiteLink FIFO.
 * @param data Output buffer for the dequeued data.
 * @param length Output variable for the length of the dequeued data.
 * @return LL_OK if successful, otherwise LL_NOT_OK.
 */
LiteLink_ReturnType LiteLink_FifoDequeue(LiteLink_Fifo* fifo, void* data, uint16_t* length);

/**
 * @brief Checks if a LiteLink FIFO queue is empty.
 *
 * @param fifo Pointer to the LiteLink FIFO.
 * @return 1 if empty, 0 if not empty.
 */
uint8_t LiteLink_FifoIsEmpty(LiteLink_Fifo* fifo);

/**
 * @brief Disposes of a LiteLink FIFO queue, freeing its resources.
 *
 * @param fifo Pointer to the LiteLink FIFO to dispose of.
 */
void LiteLink_DisposeFifo(LiteLink_Fifo* fifo);

#endif /* LITELINK_H */
