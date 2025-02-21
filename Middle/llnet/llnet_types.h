#ifndef LLNET_TYPES_H
#define LLNET_TYPES_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Return types for LiteLink functions */
typedef uint8_t LiteLink_ReturnType;          /**< Type for returning function status */
#define LL_OK           0x00                  /**< Status OK */
#define LL_NOT_OK       0x01                  /**< Status error */

#define LL_UNLOCKED              0x00         /**< Unlocked state */
#define LL_CORE_LOCKED           0x01         /**< Core locked state */
#define LL_INBOUND_LOCKED        0x02         /**< Inbound locked state */

/* Connection states in LiteLink */
#define CLOSED          0                     /**< Connection is closed */
#define OPENING         1                     /**< Connection is opening */
#define ESTABLISHED     2                     /**< Connection established */
#define CONNECTING      3                     /**< Connection is in progress */
#define FIN_WAIT        4                     /**< Waiting for connection termination */
#define CLOSING         5                     /**< Connection is closing */
#define TIME_WAIT       6                     /**< Waiting for final connection termination */
#define LAST_ACK        7                     /**< Last acknowledgment received */


/** @brief Structure for LiteLink object */
typedef struct
{
    void *data;          /**< Pointer to data */
    uint16_t length;     /**< Length of the data */
} LiteLink_Object;

/** @brief Structure for managing an object list in LiteLink */
typedef struct
{
    LiteLink_Object *objects;   /**< Array of LiteLink objects */
    uint16_t count;             /**< Current count of objects */
    uint16_t capacity;          /**< Maximum capacity of the list */
} LiteLink_ObjList;

/** @brief Structure for managing FIFO queue in LiteLink */
typedef struct
{
    LiteLink_Object *objects;   /**< Array of LiteLink objects in the queue */
    uint16_t count;             /**< Current count of objects in the queue */
    uint16_t capacity;          /**< Maximum capacity of the FIFO queue */
    uint16_t head;              /**< Index of the head of the queue */
    uint16_t tail;              /**< Index of the tail of the queue */
    uint8_t lock;               /**< Lock state of the queue */
} LiteLink_Fifo;

/** @brief Enum for LiteLink state */
typedef enum
{
    DONT_WAIT_ANYTHING   = 0x00,   /**< No waiting required */
    WAITING_FOR_SENDING  = 0x01,   /**< Waiting for sending process */
	PONG_RECEIVED        = 0x02,   /**< A pong message received */
} LiteLink_State;

/**
 * @brief Address structure for managing address and port information
 */
typedef union
{
    struct
    {
        uint16_t port : 6;     /**< Port number (6 bits) */
        uint16_t addr : 10;    /**< Address (10 bits) */
    } psr;                     /**< Address structure with bit fields */
    uint16_t value;            /**< Combined address and port value */
} LiteLink_Address;

/**
 * @brief Sequence structure for managing transmission and acknowledgment information
 */
typedef union
{
    struct
    {
        uint8_t ack : 4;       /**< Acknowledgment number (4 bits) */
        uint8_t tx  : 4;       /**< Transmission number (4 bits) */
    } psr;                     /**< Sequence structure with bit fields */
    uint8_t value;             /**< Combined ack and tx value */
} LiteLink_Sequence;

/**
 * @brief Control frame structure for managing length, flags, and type information
 */
typedef union
{
    struct
    {
        uint32_t len   : 13;   /**< Length (14 bits) */
        uint32_t flags : 8;    /**< Flags (7 bits) */
        uint32_t type  : 1;    /**< Type (1 bit) */
        uint32_t wlan  : 10;   /**< WLAN identifier (10 bits) */
    } psr;                     /**< Control frame structure with bit fields */
    uint32_t value;            /**< Combined control frame value */
} LiteLink_ctlFrame;

/**
 * @brief Offset structure for segment offset and ID management
 */
typedef union
{
    struct
    {
        uint16_t len : 8;      /**< Length (8 bits) */
        uint16_t id  : 8;      /**< Identifier (8 bits) */
    } psr;                     /**< Offset structure with bit fields */
    uint16_t value;            /**< Combined offset and ID value */
} LiteLink_Offset;

/**
 * @brief Header structure for LiteLink segments
 */
typedef struct
{
    uint8_t           crc;     /**< CRC checksum */
    LiteLink_Sequence seq;     /**< Sequence structure */
    LiteLink_Address  des;     /**< Destination address */
    LiteLink_Address  src;     /**< Source address */
    LiteLink_Offset   offset;  /**< Offset structure */
    LiteLink_ctlFrame ctl;     /**< Control frame structure */
} LiteLink_Header;

/**
 * @brief Segment structure for LiteLink data segments
 */
typedef struct
{
    LiteLink_Header header;    /**< Segment header */
    uint8_t data[0xFF];        /**< Data payload */
} LiteLink_Segment;

/* Callback function pointers for LiteLink events */
typedef void (*LiteLink_OutboundEvent)(uint8_t *data, uint8_t length);  /**< Callback for outbound events */
typedef void (*LiteLink_ReceivedEvent)(LiteLink_Address src_addr, uint8_t *data, uint16_t length); /**< Callback for received events */

/**
 * @brief Main structure of LiteLink to manage the protocol
 */
typedef struct
{
    LiteLink_Fifo           *segments;        /**< FIFO queue of data segments */
    LiteLink_ObjList        *services;        /**< List of services */
    LiteLink_OutboundEvent  outbound_event;   /**< Callback for outbound events */
    uint8_t data_size;                        /**< Maximum data size for segments */
    uint8_t lock;                             /**< Lock state of the protocol */
    uint16_t local_address;                   /**< Local address */
    uint16_t wlan_address;                    /**< WLAN address */
    uint8_t *name;                            /**< Name of the device */
} LiteLink;

/**
 * @brief Structure to manage a service connection in LiteLink
 */
typedef struct
{
    LiteLink_ObjList *buff;                   /**< Buffer for storing received data */
    LiteLink *llnet;                          /**< Pointer to the main LiteLink instance */
    LiteLink_Sequence seq;                    /**< Sequence information */
    LiteLink_Address  local;                  /**< Local address */
    LiteLink_Address  partner;                /**< Partner address */
    uint16_t          wlan;                   /**< WLAN identifier */
    LiteLink_ReceivedEvent onMessage;         /**< Callback for received message events */
    volatile uint8_t data_size;               /**< Data size for the service */
    uint8_t *name;                            /**< Name of the service */
    volatile LiteLink_State state;            /**< Current state of the service */
} LiteLink_Service;

#endif /* LLNET_TYPES_H */

