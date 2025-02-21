#ifndef _UFS_TYPES_H_
#define _UFS_TYPES_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "stdlib.h"
#include "stdint.h"

// Constants and macros
#define MAX_NAME_LENGTH     16u   // Maximum length for file or item names

#define MAX_PATH_LENGTH     200u

// Return codes
#define UFS_OK            0x00   // Operation was successful
#define UFS_NOT_OK        0x01   // Operation failed

#define UFS_SUPPORT_FOLDER   UFS_OK

typedef uint8_t ufs_ReturnType;

/**
 * @brief Initializes the UFS device.
 *
 * @return ufs_ReturnType
 *         - UFS_OK if initialization was successful.
 *         - UFS_NOT_OK if initialization failed.
 */
typedef ufs_ReturnType (*ufs_Init(void));

/**
 * @brief Writes data to the specified sector in UFS.
 *
 * @param[in]  u16SectorNumb  The sector number to which data should be written.
 * @param[in]  pData          Pointer to the data buffer to write.
 * @param[in]  u32Size        The size of the data to be written (in bytes).
 *
 * @return ufs_ReturnType
 *         - UFS_OK if the write operation was successful.
 *         - UFS_NOT_OK if the write operation failed.
 */
typedef ufs_ReturnType (*ufs_WriteSector(uint16_t u16SectorNumb, uint8_t *pData, uint32_t u32Size));

/**
 * @brief Reads data from the specified sector in UFS.
 *
 * @param[in]  u16SectorNumb  The sector number to read from.
 * @param[out] pData          Pointer to the buffer where the read data will be stored.
 * @param[in]  u32Size        The size of the data to read (in bytes).
 *
 * @return ufs_ReturnType
 *         - UFS_OK if the read operation was successful.
 *         - UFS_NOT_OK if the read operation failed.
 */
typedef ufs_ReturnType (*ufs_ReadSector(uint16_t u16SectorNumb, uint8_t *pData, uint32_t u32Size));

/**
 * @brief Erases the specified sector in UFS.
 *
 * @param[in]  u16SectorNumb  The sector number to erase.
 *
 * @return ufs_ReturnType
 *         - UFS_OK if the erase operation was successful.
 *         - UFS_NOT_OK if the erase operation failed.
 */
typedef ufs_ReturnType (*ufs_EraseSector(uint16_t u16SectorNumb));

/**
 * @brief Erases the specified block in UFS.
 *
 * @param[in]  u16BlockNumb  The block number to erase.
 *
 * @return ufs_ReturnType
 *         - UFS_OK if the erase operation was successful.
 *         - UFS_NOT_OK if the erase operation failed.
 */
typedef ufs_ReturnType (*ufs_EraseBlock(uint16_t u16BlockNumb));

/**
 * @brief Erases the entire UFS chip.
 *
 * @return ufs_ReturnType
 *         - UFS_OK if the chip erase operation was successful.
 *         - UFS_NOT_OK if the chip erase operation failed.
 */
typedef ufs_ReturnType (*ufs_EraseChip(void));

/**
 * @brief Reads the unique ID of the UFS device.
 *
 * @param[out] Unique  Pointer to the buffer where the unique ID will be stored.
 * @param[in]  length  Length of the unique ID in bytes.
 *
 * @return ufs_ReturnType
 *         - UFS_OK if the unique ID is successfully read.
 *         - UFS_NOT_OK if the read operation failed.
 */
typedef ufs_ReturnType (*ufs_ReadUniqueID(uint8_t *Unique, uint8_t length));

/**
 * @brief Locks a mutex to synchronize access to shared resources.
 *
 * @param[in] mutex  Pointer to the mutex to be locked.
 */
typedef void (*ufs_LockMutex(void *mutex));

/**
 * @brief Unlocks a mutex to release shared resources.
 *
 * @param[in] mutex  Pointer to the mutex to be unlocked.
 */
typedef void (*ufs_UnlockMutex(void *mutex));

/**
 * @brief Status of checksum in UFS.
 */
typedef enum
{
    CHECKSUM_DISABLE = 0x00,  // Checksum disabled
    CHECKSUM_ENABLE  = 0x01,  // Checksum enabled
} ufs_CheckSumStatus;

/**
 * @brief Status of encoding in UFS.
 */
typedef enum
{
    UFS_ENCODE_DISABLE = 0x00,  // Encoding disabled
    UFS_ENCODE_ENABLE  = 0x01,  // Encoding enabled
} ufs_EncodeStatus;

/**
 * @brief Status of items in UFS.
 */
typedef enum
{
    UFS_ITEM_FREE      = 0x00, /**< The item is free. */
    UFS_FILE_DELETE    = 0x01, /**< The file has been deleted. */
    UFS_FILE_EXIST     = 0x02, /**< The file exists. */
    UFS_FILE_LOCK      = 0x03, /**< The file is locked. */
    UFS_FOLDER_EXIST   = 0x04, /**< The folder exists. */
    UFS_FOLDER_DELETE  = 0x05, /**< The folder has been deleted. */
    UFS_ROOT           = 0x06, /**< The root directory. */
} ufs_ItemStatus;

/**
 * @brief Status of clusters in UFS.
 */
typedef enum
{
    UFS_CLUSTER_FREE = 0xFFFF, /**< The cluster is free. */
    UFS_CLUSTER_BAD  = 0xFFFE, /**< The cluster is marked as bad. */
    UFS_CLUSTER_END  = 0xFFFD, /**< End of the cluster chain. */
} ufs_LUTClusterStatus;

/**
 * @brief Error codes for UFS operations.
 */
typedef enum
{
    UFS_ERROR_NONE              = 0x00, /**< No error occurred. */
    UFS_ERROR_FULL_MEM          = 0x01, /**< Memory is full. */
    UFS_ERROR_READ_MEM          = 0x02, /**< Memory read error. */
    UFS_ERROR_WRITE_MEM         = 0x03, /**< Memory write error. */
    UFS_ERROR_EXISTED           = 0x04, /**< The item already exists. */
    UFS_ERROR_NOT_EXISTED       = 0x05, /**< The item does not exist. */
    UFS_ERROR_ALLOCATE_MEM      = 0x06, /**< Memory allocation error. */
    UFS_ERROR_API_NOT_FOUND     = 0x07, /**< API not found. */
    UFS_ERROR_MEM_SECTOR_BAD    = 0x08, /**< Memory sector is bad. */
    UFS_ERROR_FULL_FILE         = 0x09, /**< File system is full. */
    UFS_ERROR_FULL_CLUSTER      = 0x0A, /**< Cluster is full. */
    UFS_ERROR_INVALID_SECTOR    = 0x0B, /**< Invalid sector. */
    UFS_ERROR_SUM_SECTOR_FAIL   = 0x0C, /**< Sector checksum calculation failed. */
	UFS_ERROR_ITEM_NOT_FILE     = 0x0D, /**< Item is not File. */
	UFS_ERROR_ITEM_NOT_FOLDER   = 0x0D  /**< Item is not Folder. */
} ufs_ErrorCodes;

#if UFS_SUPPORT_FOLDER_MANAGER == UFS_OK
// Structure for a single node in the linked list
typedef struct ufs_PathNode
{
    uint8_t part[MAX_PATH_LENGTH];     // Part of the path
    struct ufs_PathNode *next;         // Pointer to the next node
} ufs_PathNode;
#endif

/**
 * @brief Structure for holding the cluster ID and its link to the next cluster.
 */
typedef struct
{
    uint16_t *value;  /**< Cluster ID. */
    uint16_t length;  /**< Link to the next cluster in the chain. */
} ufs_ListClusterID_Type;

/**
 * @brief Represents a file name and extension in UFS.
 */
typedef struct
{
    uint8_t    head[MAX_NAME_LENGTH];  /**< Main part of the file name. */
    uint8_t    extention[3];           /**< File extension. */
    uint8_t    length;                 /**< Length of the file name. */
} ufs_Name_Type;

/**
 * @brief Represents the location of an item in UFS.
 */
typedef struct
{
    uint16_t sector_id;  /**< Sector ID where the item is stored. */
    uint16_t position;   /**< Position of the item in the sector. */
} ufs_Location_Type;

/**
 * @brief Represents detailed item information in UFS.
 */
typedef union
{
    struct
    {
        ufs_Name_Type       name;           /**< Name of the item. */
        ufs_Location_Type   first_cluster;  /**< Location of the first cluster of the item. */
        uint16_t            parent;         /**< Parent directory/item. */
        uint16_t            revert;         /**< Reserved field. */
        uint32_t            size;           /**< Size of the item in bytes. */
    } comp;  /**< Detailed item information. */
    uint8_t data[32];  /**< Raw data of the item. */
} ufs_ItemInfo_Type;

/**
 * @brief Structure representing the UFS API and its function pointers.
 */
typedef struct
{
    ufs_Init          *Init;               /**< Initialization function pointer. */
    ufs_WriteSector   *WriteSector;        /**< Write sector function pointer. */
    ufs_ReadSector    *ReadSector;         /**< Read sector function pointer. */
    ufs_EraseSector   *EraseSector;        /**< Erase sector function pointer. */
    ufs_EraseBlock    *EraseBlock;         /**< Erase sector function pointer. */
    ufs_EraseChip     *EraseChip;          /**< Erase chip function pointer. */
    ufs_ReadUniqueID  *ReadUniqueID;       /**< Read unique ID function pointer. */
    ufs_LockMutex     *LockMutex;          /**< Lock mutex function pointer. */
    ufs_UnlockMutex   *UnlockMutex;        /**< Unlock mutex function pointer. */
    void              *mutex;              /**< Mutex pointer for synchronization. */
    uint16_t          u16numberByteOfSector;   /**< Number of bytes per sector. */
    uint16_t          u16numberSectorOfBlock;  /**< Number of Sector per Block. */
    uint32_t          u32numberSectorOfDevice; /**< Total number of sectors in the device. */
} ufs_Api_Type;

/**
 * @brief Structure representing the list of file extensions in UFS.
 */
typedef struct
{
    uint8_t *pListExtensionName;  /**< Pointer to the list of file extension names. */
} ufs_ExtensionName_Type;

/**
 * @brief Structure representing UFS configuration information.
 */
typedef struct
{
    ufs_Api_Type           *api;                      /**< Pointer to the UFS API. */
    uint8_t                u8NumberEncodeFileExtension; /**< Number of encoded file extensions. */
    uint8_t                u8NumberFileMaxOfDevice;    /**< Maximum number of files in the device. */
    ufs_ExtensionName_Type *pExtensionEncodeFileList;  /**< Pointer to the list of encoded file extensions. */
} ufs_Cfg_Type;

/**
 * @brief Structure representing UFS Path information.
 */
typedef struct
{
    uint16_t     id;
    uint8_t      *name;
} ufs_Path_Type;

/**
 * @brief Structure representing the UFS system.
 */
typedef struct
{
    uint16_t  ItemZoneFirstSector;            /**< First sector of the item zone. */
    uint16_t  ClusterMappingZoneFirstSector;  /**< First sector of the cluster mapping zone. */
    uint16_t  ClusterDataZoneFirstSector;     /**< First sector of the cluster data zone. */
    uint16_t  NumberSectorOfCluster;          /**< Number of sectors per cluster. */
    uint8_t   DeviceId[8];                    /**< Unique ID of the UFS device. */
    int32_t   UsedSize;                       /**< Amount of used space in bytes. */
    ufs_Cfg_Type      *conf;                  /**< Pointer to the UFS configuration structure. */
    ufs_Location_Type latest_cluster;         /**< Location of the latest allocated cluster. */
    ufs_Path_Type     path;
} UFS;

/**
 * @brief Structure representing a file item in UFS.
 */
typedef struct
{
    ufs_Location_Type      location;           /**< Location of the file in UFS. */
    ufs_ListClusterID_Type clusters;           /**< List of cluster IDs for the file. */
    ufs_ItemInfo_Type      info;               /**< Information about the file. */
    ufs_ItemStatus         status;             /**< Status of the file (free, deleted, etc.). */
    ufs_ErrorCodes         err;                /**< Error codes for file operations. */
    UFS                    *ufs;               /**< Pointer to the UFS structure. */
    ufs_EncodeStatus       EncodeEnable;       /**< Encoding enabled flag (0 = disabled, 1 = enabled). */
} ufs_Item_Type;

#ifdef __cplusplus
}
#endif

#endif /* _UFS_TYPES_H_ */
