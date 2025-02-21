#include "llnet.h"
#include <stdlib.h>
#include <string.h>
#include <machine/endian.h>

#define LITELINK_HEADER_SIZE        12u          /* Header size in bytes */
#define LITELINK_BROADCAST_ADDR     0x3FFu       /* Broadcast address for WLAN */

#define LITELINK_FIN                0x01         /* FIN flag for LiteLink */
#define LITELINK_SYN                0x02         /* SYN flag for LiteLink */
#define LITELINK_PIN                0x04         /* PIN flag for LiteLink */
#define LITELINK_PON                0x08         /* PON flag for LiteLink */
#define LITELINK_PSH                0x10         /* PSH flag for LiteLink */
#define LITELINK_ACK                0x20         /* ACK flag for LiteLink */
#define LITELINK_SER                0x40         /* SER flag for LiteLink */
#define LITELINK_DEV                0x80         /* DEV flag for LiteLink */

/**
 * @brief Swaps the nibbles of an 8-bit integer.
 *
 * @param n The 8-bit integer to swap.
 * @return The 8-bit integer with swapped nibbles.
 */
static uint8_t swap8bit(uint8_t n)
{
    return (((n & 0x0F) << 4) | ((n & 0xF0) >> 4));
}

/**
 * @brief Swaps the byte order of a 16-bit integer based on endianness.
 *
 * @param n The 16-bit integer to swap.
 * @return The swapped 16-bit integer.
 */
static uint16_t swap16bit(uint16_t n)
{
#if _BYTE_ORDER == _BIG_ENDIAN
    return n;
#else
    return ((((n) & 0xff) << 8) | (((n) & 0xff00) >> 8));
#endif
}

/**
 * @brief Swaps the byte order of a 32-bit integer based on endianness.
 *
 * @param n The 32-bit integer to swap.
 * @return The swapped 32-bit integer.
 */
static uint32_t swap32bit(uint32_t n)
{
#if _BYTE_ORDER == _BIG_ENDIAN
    return n;
#else
    return (((n & 0x000000FF) << 24) |
            ((n & 0x0000FF00) << 8)  |
            ((n & 0x00FF0000) >> 8)  |
            ((n & 0xFF000000) >> 24));
#endif
}

/**
 * @brief Initializes a new LiteLink instance.
 *
 * @param wlan_address The WLAN address for the LiteLink instance.
 * @param local_address The local address for the LiteLink instance.
 * @param segment_size The maximum size of a data segment (must exceed LITELINK_HEADER_SIZE).
 * @param name_device Name of the device to register.
 * @param outbound_event Callback function for outbound events.
 * @return A pointer to the initialized LiteLink instance or NULL if initialization fails.
 */
LiteLink* newLiteLink(uint16_t wlan_address, uint16_t local_address, uint16_t segment_size, const char* name_device, LiteLink_OutboundEvent outbound_event)
{
    if (segment_size <= LITELINK_HEADER_SIZE)
    {
        return NULL;
    }

    LiteLink* llnet = (LiteLink*)malloc(sizeof(LiteLink));
    if (llnet == NULL)
    {
        return NULL;
    }

    llnet->segments = LiteLink_newFifo(0xFF);                  /* Initialize FIFO for segments */
    llnet->services = LiteLink_newObjList(0x3F);               /* Initialize object list for services */
    llnet->outbound_event = outbound_event;
    llnet->data_size = segment_size - LITELINK_HEADER_SIZE;
    llnet->local_address = local_address & LITELINK_BROADCAST_ADDR;
    llnet->wlan_address = wlan_address & LITELINK_BROADCAST_ADDR;
    llnet->lock = LL_UNLOCKED;
    llnet->name = (uint8_t *)name_device;

    LiteLink_uListen(llnet, "system_service", 0);

    return llnet;
}

/**
 * @brief Updates the outbound event callback function of a LiteLink instance.
 *
 * @param llnet A pointer to the LiteLink instance.
 * @param event The new outbound event callback function.
 */
void LiteLink_OutboundEventUpdate(LiteLink* llnet, LiteLink_OutboundEvent event)
{
    llnet->outbound_event = event;
}

/**
 * @brief Disposes of a LiteLink instance, releasing any allocated memory.
 *
 * @param llnet A pointer to the LiteLink instance to dispose of.
 */
void LiteLink_Dispose(LiteLink* llnet)
{
    LiteLink_ObjListDispose(llnet->services);                 /* Dispose of the object list */
    llnet->outbound_event = NULL;
    free(llnet);                                              /* Free the LiteLink instance */
}

/**
 * @brief Registers a message event handler for a LiteLink service.
 *
 * @param service A pointer to the LiteLink service.
 * @param event The message event handler to register.
 */
void LiteLink_onMessage(LiteLink_Service* service, LiteLink_ReceivedEvent event)
{
    if (service != NULL)
    {
        service->onMessage = event;
    }
}

/**
 * @brief Starts listening on a specified port for a LiteLink service.
 *
 * @param llnet A pointer to the LiteLink instance.
 * @param name_service Name of the service to register.
 * @param port The port to listen on (must be less than 0x3F).
 * @return A pointer to the LiteLink service instance or NULL on failure.
 */
LiteLink_Service* LiteLink_uListen(LiteLink* llnet, const char* name_service, uint8_t port)
{
    LiteLink_Service service;
    if (port >= 0x3F)
    {
        return NULL;
    }

    /* Set up service properties */
    service.data_size = llnet->data_size;
    service.local.psr.addr = llnet->local_address;
    service.local.psr.port = port;
    service.wlan = llnet->wlan_address;
    service.name = (uint8_t*)name_service;
    service.partner.value = 0x00;
    service.seq.value = 0x00;
    service.llnet = llnet;
    service.state = DONT_WAIT_ANYTHING;
    service.buff = NULL;
    service.onMessage = NULL;

    if (LL_NOT_OK == LiteLink_AddObject(llnet->services, (void*)&service, sizeof(LiteLink_Service), (uint16_t)port))
    {
        return NULL;
    }

    return LiteLink_GetObjectPtr(llnet->services, port);
}

/**
 * @brief Closes a LiteLink service on the specified port.
 *
 * @param llnet A pointer to the LiteLink instance.
 * @param port The port of the service to close.
 */
void LiteLink_uClose(LiteLink* llnet, uint8_t port)
{
    if (port >= 0x3F)
    {
        return;
    }

    LiteLink_Service* service = (LiteLink_Service*)LiteLink_GetObjectPtr(llnet->services, port);
    if (service != NULL)
    {
        LiteLink_DeleteObj(llnet->services, port);
    }
}

/**
 * @brief Parses a LiteLink segment, performing byte swaps and checksum calculations.
 *
 * @param segment A pointer to the segment structure to parse.
 * @param buffer Output buffer to store parsed data.
 * @param length The length of the parsed segment.
 * @return The return status, LL_OK if successful.
 */
LiteLink_ReturnType LiteLink_ParserSegment(LiteLink_Segment* segment, uint8_t* buffer, uint16_t* length)
{
    LiteLink_Header* _header = (LiteLink_Header*)&segment->header;
    uint8_t len = _header->offset.psr.len;

    memcpy(buffer, (uint8_t*)&segment->header, sizeof(LiteLink_Header)); /* Copy header */
    memcpy((uint8_t*)&buffer[sizeof(LiteLink_Header)], (uint8_t*)segment->data, len); /* Copy data */

    /* Update header fields */
    _header = (LiteLink_Header*)buffer;
    _header->seq.value = rand();
    _header->des.value = swap16bit(_header->des.value);
    _header->src.value = swap16bit(_header->src.value);
    _header->ctl.value = swap32bit(_header->ctl.value);
    _header->offset.value = swap16bit(_header->offset.value);

    /* Calculate CRC */
    _header->crc = LiteLink_CalCrc8(0xFF, &buffer[1], len + sizeof(LiteLink_Header) - 1);
    *length = len + sizeof(LiteLink_Header);

    /* Perform XOR with the first byte */
//    for (uint8_t countByte = 1; countByte < *length; countByte++)
//    {
//        buffer[countByte] ^= buffer[0];
//    }

    return LL_OK;
}

/**
 * @brief Parses received data into a LiteLink segment, validates CRC, and performs byte swaps.
 *
 * @param data The raw received data.
 * @param length Length of the received data.
 * @return Parsed LiteLink_Segment with header and data.
 */
LiteLink_Segment LiteLink_ParserData(uint8_t* data, uint8_t length)
{
    LiteLink_Segment _seg;
    LiteLink_Offset offset;

//    for (uint8_t countByte = 1; countByte < length; countByte++)
//    {
//        data[countByte] ^= data[0];
//    }

    memcpy(&_seg.header, data, sizeof(LiteLink_Header));
    memcpy(_seg.data, &data[sizeof(LiteLink_Header)], length - sizeof(LiteLink_Header));

    offset.value = (uint16_t)swap16bit(_seg.header.offset.value);

    /* Check CRC */
    if (_seg.header.crc != LiteLink_CalCrc8(0xFF, (uint8_t*)&data[1], (uint16_t)(offset.psr.len + sizeof(LiteLink_Header) - 1)))
    {
        memset(&_seg.header, 0x00, sizeof(LiteLink_Header));
        return _seg;
    }

    /* Update header fields */
    _seg.header.seq.value = swap8bit(_seg.header.seq.value);
    _seg.header.des.value = swap16bit(_seg.header.des.value);
    _seg.header.src.value = swap16bit(_seg.header.src.value);
    _seg.header.ctl.value = swap32bit(_seg.header.ctl.value);
    _seg.header.offset.value = swap16bit(_seg.header.offset.value);

    return _seg;
}

/**
 * @brief Processes received inbound message by parsing and updating headers.
 *
 * @param llnet A pointer to the LiteLink instance.
 * @param data The received data to be processed.
 * @param length The length of the received data.
 */
uint16_t counterror = 0;
void LiteLink_InboundMessage(LiteLink* llnet, uint8_t* data, uint16_t length)
{
    llnet->lock |= LL_INBOUND_LOCKED;
    LiteLink_Segment _seg = LiteLink_ParserData(data, length);

    if (_seg.header.des.psr.addr == LITELINK_BROADCAST_ADDR)
    {
        _seg.header.des.psr.addr = llnet->local_address;
    }

    if(_seg.header.ctl.psr.wlan == LITELINK_BROADCAST_ADDR)
    {
    	_seg.header.ctl.psr.wlan = llnet->wlan_address;
    }

    if(_seg.header.des.psr.addr != llnet->local_address || _seg.header.ctl.psr.wlan != llnet->wlan_address)
    {
    	counterror ++;
    	return;
    }

    LiteLink_FifoEnqueue(llnet->segments, (void*)&_seg, length);
    llnet->lock &= ~LL_INBOUND_LOCKED;
}

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
LiteLink_ReturnType LiteLink_SendPacket(LiteLink_Service* service, uint16_t address, uint8_t port, uint8_t* data, uint16_t length)
{
    if (service->llnet->lock != LL_UNLOCKED)
    {
        return LL_NOT_OK;
    }
    service->llnet->lock |= LL_CORE_LOCKED;

    LiteLink_Segment segment;
    uint8_t nbSeg = length / service->llnet->data_size;
    uint8_t lastSeg = length % service->llnet->data_size;
    uint16_t time_out = 0x00;

    /* Set up destination information */
    service->partner.psr.addr = address;
    service->partner.psr.port = port;

    /* Process each segment */
    for (uint8_t countSeg = 0; countSeg < nbSeg; countSeg++)
    {
        /* Initialize segment header fields */
        segment.header.crc = 0x00;
        segment.header.seq.value = 0x00;
        segment.header.src.value = service->local.value;
        segment.header.des.psr.addr = address;
        segment.header.des.psr.port = port;
        segment.header.offset.psr.id = countSeg;
        segment.header.offset.psr.len = service->llnet->data_size;
        segment.header.ctl.psr.flags = LITELINK_PSH;
        segment.header.ctl.psr.len = length & 0x3FFF;
        segment.header.ctl.psr.wlan = service->llnet->wlan_address;

        memcpy(segment.data, &data[countSeg * service->llnet->data_size], service->llnet->data_size);

        if (LL_NOT_OK == LiteLink_FifoEnqueue(service->llnet->segments, (void*)&segment, sizeof(LiteLink_Segment)))
        {
            service->llnet->lock &= ~LL_CORE_LOCKED;
            return LL_NOT_OK;
        }
    }

    /* Process last segment if exists */
    if (lastSeg != 0)
    {
        segment.header.crc = 0x00;
        segment.header.seq.value = 0x00;
        segment.header.src.value = service->local.value;
        segment.header.des.psr.addr = address;
        segment.header.des.psr.port = port;
        segment.header.offset.psr.id = nbSeg;
        segment.header.offset.psr.len = lastSeg;
        segment.header.ctl.psr.flags = LITELINK_PSH;
        segment.header.ctl.psr.len = length & 0x3FFF;
        segment.header.ctl.psr.wlan = service->llnet->wlan_address;

        memcpy(segment.data, &data[nbSeg * service->llnet->data_size], lastSeg);

        if (LL_NOT_OK == LiteLink_FifoEnqueue(service->llnet->segments, (void*)&segment, sizeof(LiteLink_Segment)))
        {
            service->llnet->lock &= ~LL_CORE_LOCKED;
            return LL_NOT_OK;
        }
    }

    service->llnet->lock &= ~LL_CORE_LOCKED;
    service->state = WAITING_FOR_SENDING;

    /* Wait for completion */
    while (service->state != DONT_WAIT_ANYTHING)
    {
    	if(++ time_out == 1000)
    	{
    		return LL_NOT_OK;
    	}
    	LITELINK_DELAY(1);
    };

    return LL_OK;
}

/**
 * @brief Sends a Ping packet to a specified address and waits for a response.
 *
 * @param llnet Pointer to the LiteLink instance.
 * @param address Target address to send the Ping packet to.
 * @return uint16_t Timeout duration for the response, or 0xFFFF if the wait exceeds the limit.
 */
uint16_t LiteLink_SendPing(LiteLink* llnet, uint16_t address)
{
    uint16_t time_out = 0x00;
    LiteLink_Segment segment;

    if (llnet->lock != LL_UNLOCKED)
    {
        return LL_NOT_OK;
    }

    // Lock the LiteLink instance during the send operation
    llnet->lock |= LL_CORE_LOCKED;

    // Configure the Ping packet
    segment.header.crc = 0x00;
    segment.header.seq.value    = 0x00;
    segment.header.src.psr.addr = llnet->local_address;
    segment.header.src.psr.port = 0x00;
    segment.header.des.psr.addr = address;
    segment.header.des.psr.port = 0x00;
    segment.header.offset.psr.id  = 0x00;
    segment.header.offset.psr.len = 0x00;
    segment.header.ctl.psr.flags  = LITELINK_PIN;
    segment.header.ctl.psr.len    = 0x00;
    segment.header.ctl.psr.wlan   = llnet->wlan_address;

    // Enqueue the packet into the FIFO
    if (LL_NOT_OK == LiteLink_FifoEnqueue(llnet->segments, (void*)&segment, sizeof(LiteLink_Segment)))
    {
        llnet->lock &= ~LL_CORE_LOCKED; // Unlock if enqueue operation fails
        return LL_NOT_OK;
    }

    LiteLink_Service* service = LiteLink_GetObjectPtr(llnet->services, 0x00);

    // Update state and unlock service
    service->llnet->lock &= ~LL_CORE_LOCKED;
    service->state = WAITING_FOR_SENDING;

    // Wait for a PONG response with a timeout limit
    while (service->state != PONG_RECEIVED)
    {
        if (++time_out == 1000)
        {
            return 0xFFFF;  // Return if timeout limit is reached
        }
        LITELINK_DELAY(1);  // Delay for 1 ms
    }

    // Unlock the LiteLink instance after completing the operation
    llnet->lock &= ~LL_CORE_LOCKED;

    return time_out; // Return the time waited for the response
}

/**
 * @brief Processes queued segments, validates headers, and triggers events for inbound data.
 *
 * @param llnet Pointer to the LiteLink instance to process segments.
 */
void LiteLink_Process(LiteLink* llnet)
{
    uint8_t data_segment[0xFF];
    uint16_t length = 0;
    LiteLink_Segment segment;

    /* Check if locked or null */
    if (llnet == NULL || (llnet->lock & LL_CORE_LOCKED) == LL_CORE_LOCKED)
    {
        return;
    }

    /* Dequeue and process segments */
    if (LL_OK == LiteLink_FifoDequeue(llnet->segments, (void*)&segment, &length))
    {
        /* Handle incoming data based on address */
        if (segment.header.des.psr.addr == llnet->local_address)
        {
            LiteLink_Service* service = LiteLink_GetObjectPtr(llnet->services, segment.header.des.psr.port);
            service->state = DONT_WAIT_ANYTHING;

            if ((segment.header.ctl.psr.flags & LITELINK_PSH) == LITELINK_PSH)
            {
                /* Handle segmented data with merging */
                if (segment.header.ctl.psr.len > segment.header.offset.psr.len)
                {
                    if (segment.header.offset.psr.id == 0)
                    {
                        if (service->buff != NULL)
                        {
                            LiteLink_ObjListDispose(service->buff);
                        }
                        service->buff = LiteLink_newObjList(0xFF);
                    }

                    LiteLink_AddObject(service->buff, segment.data, segment.header.offset.psr.len, segment.header.offset.psr.id);
                    LiteLink_MergeObjects(service->buff);
                    length = LiteLink_GetObjectLength(service->buff, 0);

                    if (length >= segment.header.ctl.psr.len)
                    {
                        uint8_t* data = LiteLink_GetObjectPtr(service->buff, 0);

                        if (service != NULL && service->onMessage != NULL)
                        {
                            service->onMessage(segment.header.src, data, length);
                        }
                    }

                }
                else
                {
                    /* Trigger onMessage event directly */
                    if (service != NULL && service->onMessage != NULL)
                    {
                        service->onMessage(segment.header.src, segment.data, segment.header.ctl.psr.len);
                    }
                }
            }

            if ((segment.header.ctl.psr.flags & LITELINK_PIN) == LITELINK_PIN)
            {
            	LiteLink_Segment _seg;

            	_seg.header.crc = 0x00;
            	_seg.header.seq.value    = 0x00;
            	_seg.header.src.psr.addr = llnet->local_address;
            	_seg.header.src.psr.port = 0x00;
            	_seg.header.des.psr.addr = segment.header.src.psr.addr;
            	_seg.header.des.psr.port = 0x00;
            	_seg.header.offset.psr.id  = 0x00;
            	_seg.header.offset.psr.len = 0x00;
            	_seg.header.ctl.psr.flags = LITELINK_PON;
            	_seg.header.ctl.psr.len   = 0x00;
            	_seg.header.ctl.psr.wlan  = llnet->wlan_address;

            	if(_seg.header.des.psr.addr != llnet->local_address)
            	{
					LiteLink_ParserSegment(&_seg, data_segment, &length);
					if (llnet->outbound_event != NULL)
					{
						llnet->outbound_event(data_segment, length);
					}
            	}
            	else
            	{
            		service->state = PONG_RECEIVED;
            	}
            }

            if ((segment.header.ctl.psr.flags & LITELINK_PON) == LITELINK_PON)
            {
            	service->state = PONG_RECEIVED;
            }

        }
        else
        {
            LiteLink_Service* service = LiteLink_GetObjectPtr(llnet->services, segment.header.src.psr.port);
            service->state = DONT_WAIT_ANYTHING;

            LiteLink_ParserSegment(&segment, data_segment, &length);
            if (llnet->outbound_event != NULL)
            {
                llnet->outbound_event(data_segment, length);
            }
        }
    }
}
