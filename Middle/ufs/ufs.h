#ifndef _UFS_H_
#define _UFS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ufs_conf.h"
#include "ufs_types.h"

/**
 * @brief Initializes a new UFS instance.
 *
 * This function initializes a new instance of the UFS (Universal File System)
 * with the given configuration. It validates the provided function pointers
 * and reads the initial boot sector for setup.
 *
 * @param[in]  pUfsCfg  Pointer to the UFS configuration structure.
 *
 * @return UFS*         Pointer to the initialized UFS instance, or NULL if initialization failed.
 */
UFS *newUFS(ufs_Cfg_Type *pUfsCfg);

/**
 * @brief Performs a fast format of the UFS device.
 *
 * This function erases the critical sectors of the UFS device, including the boot sector,
 * item zone, and cluster mapping zone. It then initializes the device with the proper
 * structure, allowing for future file operations.
 *
 * @param[in]  ufs   Pointer to the UFS structure.
 *
 * @return ufs_ReturnType   UFS_OK on success, UFS_NOT_OK on failure.
 */
ufs_ReturnType ufs_FastFormat(UFS *ufs);

/**
 * @brief Opens a file in the UFS system.
 *
 * This function opens a file in the UFS by searching for the file name in the item zone.
 * If the file exists, it retrieves the necessary metadata and cluster information.
 * If the file doesn't exist, it creates a new entry if there's available space.
 *
 * @param[in]  ufs        Pointer to the UFS structure.
 * @param[in]  name_file  Pointer to the file name string.
 * @param[in]  item       Pointer to the UFS item structure that will hold the file metadata.
 *
 * @return ufs_ReturnType UFS_OK on success, UFS_NOT_OK if the file cannot be opened or created.
 */
__fast ufs_ReturnType ufs_OpenItem(UFS *ufs, uint8_t *name_file, ufs_Item_Type *item);

/**
 * @brief Deletes a file from the UFS system.
 *
 * This function removes a file from the UFS by cleaning the cluster list and
 * marking the file as free in the item zone. All associated data is erased
 * from the file system.
 *
 * @param[in]  item  Pointer to the UFS item structure that describes the file to be deleted.
 *
 * @return ufs_ReturnType UFS_OK on success, UFS_NOT_OK on failure.
 */
ufs_ReturnType ufs_DeleteItem(ufs_Item_Type *item);

/**
 * @brief   Closes a UFS item and releases allocated memory.
 *
 * This function releases the resources associated with a UFS item, including
 * freeing the allocated memory for its cluster list and resetting its metadata
 * fields. It ensures that the item is no longer used after closing.
 *
 * @param[in]   item    Pointer to the UFS item structure to be closed.
 *
 * @return      ufs_ReturnType  UFS_OK on success, UFS_NOT_OK if the item is invalid.
 */
ufs_ReturnType ufs_CloseItem(ufs_Item_Type *item);

/**
 * @brief   Counts the number of items in the UFS file system.
 *
 * This function traverses the item zone in the UFS file system and counts the
 * number of non-free items (used items) present in the system.
 *
 * @param[in]   ufs     Pointer to the UFS structure.
 *
 * @return      uint16_t  The number of used items in the UFS file system.
 */
uint16_t ufs_CountItem(UFS *ufs);

/**
 * @brief Checks the existence of a directory or file within the currently mounted folder in UFS.
 *
 * This function scans the item zone in the UFS to verify if a specified
 * directory or file name exists within the mounted folder. If found,
 * the provided item structure is populated with its details.
 *
 * @param[in] ufs    Pointer to the UFS structure.
 * @param[in] name   Name of the directory or file to check.
 * @param[out] item  Pointer to store detailed information of the item if found.
 *
 * @return ufs_ReturnType UFS_OK if the item exists within the mounted folder, UFS_NOT_OK otherwise.
 */
ufs_ReturnType ufs_CheckExistence(UFS *ufs, uint8_t *name, ufs_Item_Type *item);

/**
 * @brief   Retrieves a list of used items from the UFS file system.
 *
 * This function populates the provided array with information about non-free
 * items (used items) present in the UFS file system, up to the specified length.
 *
 * @param[in]   ufs         Pointer to the UFS structure.
 * @param[out]  item_info   Pointer to an array to hold the retrieved item information.
 * @param[in]   length      Maximum number of items to retrieve.
 *
 * @return      uint16_t    The number of items successfully retrieved.
 */
uint16_t ufs_GetListItem(UFS *ufs, ufs_ItemInfo_Type *item_info, uint16_t length);

/**
 * @brief   Calculates the total used space in the UFS file system.
 *
 * This function calculates the total space occupied by used items (files) in the
 * UFS file system, by summing the size of each used item.
 *
 * @param[in]   ufs     Pointer to the UFS structure.
 *
 * @return      uint32_t  The total used size in bytes.
 */
uint32_t ufs_GetUsedSize(UFS *ufs);

/**
 * @brief   Retrieves the total device size of the UFS file system.
 *
 * This function calculates the total usable size of the UFS device, taking into
 * account the size of each sector and the number of sectors available in the UFS.
 *
 * @param[in]   ufs     Pointer to the UFS structure.
 *
 * @return      uint32_t  The total device size in bytes.
 */
uint32_t ufs_GetDeviceSize(UFS *ufs);

/**
 * @brief   Reads data from a file in the UFS file system.
 *
 * This function reads data from the specified file starting from a given position
 * and copies the requested number of bytes into the provided data buffer.
 *
 * @param[in]   file      Pointer to the UFS file structure.
 * @param[in]   position  The position within the file to start reading.
 * @param[out]  data      Pointer to the buffer to store the read data.
 * @param[in]   length    The number of bytes to read from the file.
 *
 * @return      uint32_t  The number of bytes successfully read from the file.
 */
__fast uint32_t ufs_ReadFile(ufs_Item_Type *file, uint32_t position, uint8_t *data, uint32_t length);

/**
 * @brief   Writes data to a file in the UFS file system.
 *
 * This function writes the specified data into the file. If the file does not
 * have enough space, new clusters will be allocated as needed. The data is written
 * starting from the current file position.
 *
 * @param[in]   file     Pointer to the UFS file structure.
 * @param[in]   data     Pointer to the data buffer to be written.
 * @param[in]   length   The number of bytes to write to the file.
 *
 * @return      ufs_ReturnType  UFS_OK on success, UFS_NOT_OK on failure.
 */
__fast ufs_ReturnType ufs_WriteFile(ufs_Item_Type *file, uint8_t *data, uint32_t length, ufs_CheckSumStatus sumEnable);

/**
 * @brief   Appends data to a file in the UFS file system.
 *
 * This function appends the specified data to the end of the file. If the file
 * does not have enough space, new clusters will be allocated as needed. The data
 * is appended starting from the file's end position.
 *
 * @param[in]   file     Pointer to the UFS file structure.
 * @param[in]   data     Pointer to the data buffer to be appended.
 * @param[in]   length   The number of bytes to append to the file.
 *
 * @return      ufs_ReturnType  UFS_OK on success, UFS_NOT_OK on failure.
 */
__fast ufs_ReturnType ufs_WriteAppendFile(ufs_Item_Type *file, uint8_t *data, uint32_t length, ufs_CheckSumStatus sumEnable);

/**
 * @brief   Renames an item in the UFS (Universal File System).
 *
 * This function renames a given item in the UFS by searching through the item zone
 * and checking if the desired name already exists. If the name exists, the function
 * returns an error. If not, it updates the item's name and metadata.
 *
 * @param[in]   item      Pointer to the UFS item structure.
 * @param[in]   strName   New name for the item.
 *
 * @return      ufs_ReturnType  UFS_OK on success, UFS_NOT_OK on failure.
 */
ufs_ReturnType ufs_RenameItem(ufs_Item_Type *item, uint8_t *strName);

#if UFS_SUPPORT_FOLDER_MANAGER == UFS_OK

/**
 * @brief   Mounts a specified path in the UFS system, ensuring all directories in the path exist.
 *
 * This function parses a given path into individual directory components, checks if each directory exists,
 * and creates any missing directories along the path. It updates the UFS path structure upon success.
 *
 * @param[in]  ufs    Pointer to the UFS structure.
 * @param[in]  path   Pointer to the path string to be mounted.
 *
 * @return  ufs_ReturnType   UFS_OK if the path is successfully mounted, UFS_NOT_OK if an error occurs.
 */
ufs_ReturnType ufs_Mount(UFS *ufs, const uint8_t *path);

/**
 * @brief Deletes a folder and all its contents recursively.
 *
 * This function mounts to the specified folder path and recursively deletes
 * all items within, including subfolders and files.
 *
 * @param[in] ufs         Pointer to the UFS instance.
 * @param[in] directory   Path of the folder to delete.
 *
 * @return ufs_ReturnType UFS_OK if deletion is successful, UFS_NOT_OK otherwise.
 */
ufs_ReturnType ufs_DeleteFolder(UFS *ufs, uint8_t *directory);

#endif

#ifdef __cplusplus
}
#endif

#endif /* _UFS_H_ */
