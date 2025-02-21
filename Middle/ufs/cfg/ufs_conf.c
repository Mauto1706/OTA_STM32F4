
#include "ufs_conf.h"
#include "MemFlash.h"

/**
 * @brief List of supported file extensions for encoding.
 *
 * This array holds the supported file extensions that can be encoded within the UFS system.
 */
ufs_ExtensionName_Type ExtensionList[UFS_NUMB_OF_ENCODE_EXTENSION] =
{
    {(uint8_t *)"usr"}, /**< User extension */
    {(uint8_t *)"sys"}, /**< System extension */
    {(uint8_t *)"bin"}  /**< Binary extension */
};

/**
 * @brief Mapping of UFS API functions to their corresponding memory flash implementations.
 *
 * This structure initializes the function pointers for various UFS operations by mapping them to
 * the actual implementations provided by the memory flash module.
 */
ufs_Api_Type Api_Mapping =
{
    .Init              = (ufs_Init *)MemFlash_Init,               /**< Initialization function */
    .WriteSector       = (ufs_WriteSector *)MemFlash_WriteSector, /**< Function to write data to a sector */
    .ReadSector        = (ufs_ReadSector *)MemFlash_ReadSector,   /**< Function to read data from a sector */
    .EraseSector       = (ufs_EraseSector *)MemFlash_EraseSector, /**< Function to erase a sector */
	.EraseBlock		   = (ufs_EraseSector *)MemFlash_EraseBlock,  /**< Function to erase a block */
    .EraseChip         = (ufs_EraseChip *)MemFlash_EraseChip,     /**< Function to erase the entire chip */
    .ReadUniqueID      = (ufs_ReadUniqueID *)MemFlash_ReadID,     /**< Function to read the unique ID of the device */
    .u16numberByteOfSector   = 4096,                                /**< Number of bytes per sector */
	.u16numberSectorOfBlock  = 16,                                /**< Number of sector per Block */
    .u32numberSectorOfDevice = 4096                                /**< Total number of sectors in the device */
};

/**
 * @brief Configuration structure for the UFS system.
 *
 * This structure holds the configuration parameters required to initialize and manage the UFS system,
 * including API mappings, supported file extensions, and device capacity settings.
 */
ufs_Cfg_Type Ufs_Cfg = {
    .api                          = &Api_Mapping,                          /**< Pointer to the UFS API mappings */
    .pExtensionEncodeFileList     = ExtensionList,                         /**< Pointer to the list of supported file extensions */
    .u8NumberFileMaxOfDevice      = 20,                                    /**< Maximum number of files supported by the device */
    .u8NumberEncodeFileExtension  = UFS_NUMB_OF_ENCODE_EXTENSION           /**< Number of supported encoded file extensions */
};
