#include "ufs.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#if UFS_SUPPORT_FOLDER_MANAGER == UFS_OK
#include <stdio.h>
#endif

// Define constants
#define BOOT_SECTOR_ID      0x00   // ID for the boot sector

#define BYTE_CODEC_DEFAULT  0xAA   // byte use to decode / encode

/**
 * @brief   Compares two byte arrays for equality.
 *
 * This function compares two byte arrays, byte by byte, to check if they are
 * identical. It returns `E_OK` if the arrays are equal, otherwise `E_NOT_OK`.
 *
 * @param[in]   pSrcData      Pointer to the source byte array.
 * @param[in]   pDesData      Pointer to the destination byte array.
 * @param[in]   u16NumbOfByte Number of bytes to compare.
 *
 * @return      ufs_ReturnType  E_OK if arrays are equal, E_NOT_OK otherwise.
 */
ufs_ReturnType ufs_BytesCmp(uint8_t *pSrcData, uint8_t *pDesData, uint16_t u16NumbOfByte)
{
    // Iterate over each byte and compare
    for (uint16_t countByte = 0; countByte < u16NumbOfByte; countByte++)
    {
        // If any byte is different, return E_NOT_OK
        if (pSrcData[countByte] != pDesData[countByte])
        {
            return UFS_NOT_OK;
        }
    }

    // If all bytes are equal, return E_OK
    return UFS_OK;
}

/**
 * @brief   Calculates the 8-bit checksum of a byte array.
 *
 * This function calculates the checksum by summing all bytes in the array
 * and reducing the result to 8 bits (modulo 256).
 *
 * @param[in]   data   Pointer to the byte array.
 * @param[in]   len    Length of the byte array.
 *
 * @return      uint8_t  Calculated 8-bit checksum.
 */
static uint8_t ufs_CheckSum(uint8_t *data, uint32_t len)
{
    uint8_t checksum = 0;

    // Sum each byte in the array
    for (uint32_t i = 0; i < len; i++)
    {
        checksum += data[i];
    }

    return checksum ^ BYTE_CODEC_DEFAULT;
}

/**
 * @brief      Removes special characters from the input string.
 *
 * This helper function removes all characters from the input string that are
 * not alphanumeric or a dot ('.'). This sanitizes the string for further parsing.
 *
 * @param[in, out]  str  Pointer to the string to be cleaned.
 *
 * @pre        `str` should be a null-terminated string.
 * @post       `str` will contain only alphanumeric characters and dots.
 *
 * @limitations This function assumes ASCII characters for alphanumeric checks.
 */
void ufs_RemoveSpecialChars(uint8_t *str)
{
    uint8_t *src = str, *dst = str;
    while (*src)
    {
        // Keep only alphanumeric characters and dots
        if (isalnum(*src) || *src == '.')
        {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0'; // Null-terminate the cleaned string
}

/**
 * @brief Parses the file name into its name and extension components.
 *
 * This function splits the file name string by the '.' character, extracting
 * the base name and extension, and stores them in the name_parser structure.
 *
 * @param[in]   name         Pointer to the file name string.
 * @param[out]  name_parser  Pointer to the structure where the parsed name and extension are stored.
 */
static void ufs_ParseNameFile(uint8_t *name, ufs_Name_Type *name_parser)
{
	// Remove special characters from the filename
	ufs_RemoveSpecialChars(name);

    // Ensure name_parser is initialized to avoid any undefined behavior
    memset(name_parser->head, 0x00, MAX_NAME_LENGTH);
    memset(name_parser->extention, 0x00, 3);
    name_parser->length = 0;

    // Use strtok_r for thread-safe string tokenization
    char *saveptr = NULL;
    char *token = strtok_r((char *)name, ".", &saveptr);

    if (token != NULL)
    {
        // Copy the base file name, ensuring it doesn't exceed MAX_NAME_LENGTH
    	if(saveptr != NULL)
    	{
            name_parser->length = (uint8_t *)saveptr -  (uint8_t *)name - 1;
    		strncpy((char *)name_parser->head, token, name_parser->length);

    		// Get the file extension, if it exists
    		token = strtok_r(NULL, ".", &saveptr);
    		if (token != NULL)
    		{
    			// Copy the extension, ensuring it fits into 3 characters
    			strncpy((char *)name_parser->extention, token, 3);
    		}
    	}
    	else
    	{
    		strncpy((char *)name_parser->head, token,strlen((char *)name));
    		name_parser->length = strlen((char *)name_parser->head);
    	}
    }
}

/**
 * @brief   Retrieves the list of clusters associated with a file.
 *
 * This function reads the cluster chain for the given file, starting from
 * the first cluster and following the chain until the end. It updates the
 * cluster array in the `item` structure.
 *
 * @param[in]   ufs   Pointer to the UFS structure.
 * @param[in]   item  Pointer to the UFS item structure that contains file information.
 *
 * @return      ufs_ReturnType    UFS_OK on success, UFS_NOT_OK on failure.
 */
static ufs_ReturnType ufs_GetListCluster(UFS *ufs, ufs_Item_Type *item)
{
    uint16_t *valueSlot;
    uint16_t idSector = 0;
    uint16_t idSector_old = 0;
    uint16_t position = 0;

    // Allocate memory for reading data from one sector
    uint8_t data_sector[ufs->conf->api->u16numberByteOfSector];

    // Calculate the number of clusters needed based on the file size
    uint32_t file_size_in_bytes = item->info.comp.size;
    uint32_t cluster_size_in_bytes = ufs->conf->api->u16numberByteOfSector * ufs->NumberSectorOfCluster;
    item->clusters.length = (file_size_in_bytes + cluster_size_in_bytes - 1) / cluster_size_in_bytes + 1;

    if(file_size_in_bytes == 0)
    {
    	item->clusters.length ++;
    }

    // Reallocate memory for the clusters array
    item->clusters.value = (uint16_t *)realloc(item->clusters.value, item->clusters.length * sizeof(uint16_t));
    if (item->clusters.value == NULL)
    {
        return UFS_NOT_OK;   // Memory allocation failure
    }
    // Initialize the clusters array to invalid values (0xFFFF)
    memset(item->clusters.value, 0xFF, item->clusters.length * sizeof(uint16_t));

    // Set the first cluster of the file from the file metadata
    item->clusters.value[0] = (item->info.comp.first_cluster.sector_id *
                               (ufs->conf->api->u16numberByteOfSector / 2)) +
                               item->info.comp.first_cluster.position;

    // Get the initial sector and position for the first cluster
    idSector = item->clusters.value[0] / (ufs->conf->api->u16numberByteOfSector / 2);
    position = item->clusters.value[0] % (ufs->conf->api->u16numberByteOfSector / 2);
    idSector_old = idSector;

    // Read the initial sector
    ufs->conf->api->ReadSector(ufs->ClusterMappingZoneFirstSector + idSector, data_sector, ufs->conf->api->u16numberByteOfSector);

    // Iterate through the clusters and build the cluster chain
    for (uint16_t countSlot = 1; countSlot < item->clusters.length; countSlot++)
    {
        // Update sector and position for the current cluster
        idSector = item->clusters.value[countSlot - 1] / (ufs->conf->api->u16numberByteOfSector / 2);
        position = item->clusters.value[countSlot - 1] % (ufs->conf->api->u16numberByteOfSector / 2);

        // If the sector has changed, read the new sector
        if (idSector_old != idSector)
        {
            idSector_old = idSector;
            ufs->conf->api->ReadSector(ufs->ClusterMappingZoneFirstSector + idSector, data_sector, ufs->conf->api->u16numberByteOfSector);
        }

        // Get the value of the next cluster in the chain
        valueSlot = (uint16_t *)&data_sector[position * 2];
        item->clusters.value[countSlot] = *valueSlot;

        // Handle different cluster states
        if (item->clusters.value[countSlot] == UFS_CLUSTER_END)
        {
            return UFS_OK;
        }
        else if (item->clusters.value[countSlot] == UFS_CLUSTER_FREE)
        {
            // If a free cluster is found, mark it as the end of the chain
            item->clusters.value[countSlot] = UFS_CLUSTER_END;
            return UFS_OK;
        }
        else if (item->clusters.value[countSlot] == UFS_CLUSTER_BAD)
        {
            // If a bad cluster is encountered, set an error in the item
            item->err = UFS_ERROR_MEM_SECTOR_BAD;
            return UFS_NOT_OK;
        }
    }

    // Ensure the last cluster in the list is marked as the end of the chain
    item->clusters.value[item->clusters.length - 1] = UFS_CLUSTER_END;

    return UFS_OK;
}

/**
 * @brief   Frees the list of clusters associated with a file.
 *
 * This function marks all clusters in the list as free and updates
 * the cluster mapping accordingly. The clusters are processed in reverse
 * order, and the corresponding sectors are written back to the UFS.
 *
 * @param[in]   ufs       Pointer to the UFS structure.
 * @param[in]   clusters  Pointer to the list of cluster IDs.
 * @param[in]   length    Number of clusters in the list.
 *
 * @return      ufs_ReturnType    UFS_OK on success, UFS_NOT_OK on failure.
 */
static ufs_ReturnType ufs_CleanClusters(UFS *ufs, uint16_t *clusters, uint16_t length)
{
    // If the cluster list is too short, there's nothing to clean
    if (length < 2)
    {
        return UFS_NOT_OK;
    }

    uint16_t idSector = 0, idSector_old = 0, position = 0;
    uint16_t *valueSlot;

    // Allocate memory for sector data
    uint8_t data_sector[ufs->conf->api->u16numberByteOfSector];

    // Start with the second-to-last cluster
    idSector = clusters[length - 2] / (ufs->conf->api->u16numberByteOfSector / 2);
    position = clusters[length - 2] % (ufs->conf->api->u16numberByteOfSector / 2);
    idSector_old = idSector;

    // Read the corresponding sector for the second-to-last cluster
    ufs->conf->api->ReadSector(ufs->ClusterMappingZoneFirstSector + idSector, data_sector, ufs->conf->api->u16numberByteOfSector);

    // Loop through clusters in reverse order, freeing them
    for (int16_t countSlot = length - 1; countSlot > 0; countSlot--)
    {
        // Calculate the sector and position of the current cluster
        idSector = clusters[countSlot - 1] / (ufs->conf->api->u16numberByteOfSector / 2);
        position = clusters[countSlot - 1] % (ufs->conf->api->u16numberByteOfSector / 2);

        // If the sector changes, write the previous sector and read the new one
        if (idSector_old != idSector)
        {
            // Write the modified sector back to the UFS
        	ufs->conf->api->EraseSector(ufs->ClusterMappingZoneFirstSector + idSector_old);
            ufs->conf->api->WriteSector(ufs->ClusterMappingZoneFirstSector + idSector_old, data_sector, ufs->conf->api->u16numberByteOfSector);

            // Read the new sector
            idSector_old = idSector;
            ufs->conf->api->ReadSector(ufs->ClusterMappingZoneFirstSector + idSector, data_sector, ufs->conf->api->u16numberByteOfSector);
        }

        // Mark the cluster as free
        valueSlot = (uint16_t *)&data_sector[position * 2];
        if(*valueSlot != UFS_CLUSTER_BAD)
        {
        	*valueSlot = UFS_CLUSTER_FREE;
        }

        // Update the used size
        ufs->UsedSize -= ufs->conf->api->u16numberByteOfSector * ufs->NumberSectorOfCluster;
        if(ufs->UsedSize < 0)
        {
        	ufs->UsedSize = 0;
        }
    }

    // Write the last modified sector back to the UFS
    ufs->conf->api->EraseSector(ufs->ClusterMappingZoneFirstSector + idSector);
    ufs->conf->api->WriteSector(ufs->ClusterMappingZoneFirstSector + idSector, data_sector, ufs->conf->api->u16numberByteOfSector);

    return UFS_OK;
}

/**
 * @brief   Orders and allocates clusters in the UFS.
 *
 * This function finds free clusters in the UFS, orders them, and writes the
 * necessary mapping to the cluster mapping zone. It updates the latest cluster
 * information and ensures that the clusters are linked sequentially.
 *
 * @param[in]   ufs       Pointer to the UFS structure.
 * @param[out]  clusters  Pointer to the array that will hold the ordered cluster IDs.
 * @param[in]   length    Number of clusters to allocate and order.
 *
 * @return      ufs_ReturnType    UFS_OK on success, UFS_NOT_OK on failure.
 */
static ufs_ReturnType ufs_OrderClusters(UFS *ufs, uint16_t *clusters, uint16_t length)
{
    if (length < 2)
    {
        return UFS_NOT_OK;  // No need to order if fewer than two clusters
    }

    uint8_t data_sector[ufs->conf->api->u16numberByteOfSector];

    uint16_t sector_old = 0;
    uint16_t countSector = ufs->latest_cluster.sector_id;
    uint16_t countSegment = ufs->latest_cluster.position;
    uint16_t *valueCluster;

    // Find and allocate free clusters
    for (uint16_t count_cluster = 0; count_cluster < (length - 1); count_cluster++)
    {
        clusters[count_cluster] = 0xFFFF;  // Initialize as invalid

        // Search for free cluster
        do
        {
            if (++countSector >= (ufs->ClusterDataZoneFirstSector - ufs->ClusterMappingZoneFirstSector))
            {
                countSector = 0x00;  // Wrap around if sector exceeds its range
            }

            if(countSector * (ufs->conf->api->u16numberByteOfSector / 2) >= \
               (ufs->conf->api->u32numberSectorOfDevice - ufs->ClusterDataZoneFirstSector) / ufs->NumberSectorOfCluster)
            {
            	countSector = 0;
            }

            ufs->conf->api->ReadSector(ufs->ClusterMappingZoneFirstSector + countSector, data_sector, ufs->conf->api->u16numberByteOfSector);

            countSegment = ufs->latest_cluster.position;
            do
            {
                if (++countSegment >= (ufs->conf->api->u16numberByteOfSector / 2))
                {
                    countSegment = 0x00;  // Wrap around if segment exceeds its range
                }

                if(countSegment >= (ufs->conf->api->u32numberSectorOfDevice - ufs->ClusterDataZoneFirstSector) / ufs->NumberSectorOfCluster)
                {
                	countSegment = 0;
                }

                valueCluster = (uint16_t *)&data_sector[countSegment * 2];
                if (*valueCluster == UFS_CLUSTER_FREE)
                {
                    clusters[count_cluster] = countSector * (ufs->conf->api->u16numberByteOfSector / 2) + countSegment;
                    ufs->latest_cluster.sector_id = countSector;
                    ufs->latest_cluster.position = countSegment;
                    if(ufs->NumberSectorOfCluster == ufs->conf->api->u16numberSectorOfBlock)
                    {
                    	uint16_t sectorID = ufs->ClusterDataZoneFirstSector + clusters[count_cluster] * ufs->NumberSectorOfCluster;
                    	ufs->conf->api->EraseBlock(sectorID / ufs->NumberSectorOfCluster);
                    }
                    break;
                }
            } while (countSegment != ufs->latest_cluster.position);

        } while (countSector != ufs->latest_cluster.sector_id);

        if (clusters[count_cluster] == 0xFFFF)  // If no free cluster found, fail
        {
            return UFS_NOT_OK;
        }
    }

    // Order and link clusters
    clusters[length - 1] = UFS_CLUSTER_END;  // Mark end of cluster chain
    countSector = clusters[0] / (ufs->conf->api->u16numberByteOfSector / 2);
    countSegment = clusters[0] % (ufs->conf->api->u16numberByteOfSector / 2);

    ufs->conf->api->ReadSector(ufs->ClusterMappingZoneFirstSector + countSector, data_sector, ufs->conf->api->u16numberByteOfSector);
    sector_old = countSector;

    for (uint16_t count_cluster = 0; count_cluster < (length - 1); count_cluster++)
    {
        countSector = clusters[count_cluster] / (ufs->conf->api->u16numberByteOfSector / 2);
        countSegment = clusters[count_cluster] % (ufs->conf->api->u16numberByteOfSector / 2);

        if (sector_old != countSector)  // Write previous sector and read new one
        {
        	ufs->conf->api->EraseSector(ufs->ClusterMappingZoneFirstSector + sector_old);
            ufs->conf->api->WriteSector(ufs->ClusterMappingZoneFirstSector + sector_old, data_sector, ufs->conf->api->u16numberByteOfSector);
            ufs->conf->api->ReadSector(ufs->ClusterMappingZoneFirstSector + countSector, data_sector, ufs->conf->api->u16numberByteOfSector);
            sector_old = countSector;
        }

        // Link the current cluster to the next
        valueCluster = (uint16_t *)&data_sector[countSegment * 2];
        *valueCluster = clusters[count_cluster + 1];

        // Update used size
        ufs->UsedSize += ufs->conf->api->u16numberByteOfSector * ufs->NumberSectorOfCluster;
    }

    // Write the final sector
    ufs->conf->api->EraseSector(ufs->ClusterMappingZoneFirstSector + countSector);
    ufs->conf->api->WriteSector(ufs->ClusterMappingZoneFirstSector + countSector, data_sector, ufs->conf->api->u16numberByteOfSector);

    return UFS_OK;
}

/**
 * @brief   Sets a value in the cluster map of the UFS.
 *
 * This function updates the cluster mapping table by writing the provided value at the
 * specified cluster index. It calculates the sector and offset within the sector where
 * the value should be written, reads the sector, updates the value, and writes the
 * sector back to the UFS.
 *
 * @param[in]   ufs             Pointer to the UFS structure.
 * @param[in]   cluster_index   The index of the cluster to update in the cluster map.
 * @param[in]   value           The value to set in the cluster map.
 *
 * @return      ufs_ReturnType  UFS_OK on success, UFS_NOT_OK on failure.
 */
ufs_ReturnType ufs_SetClusterMap(UFS *ufs, uint16_t cluster_index, uint16_t value)
{
    // Calculate the sector index in the cluster map where the cluster index resides
    uint16_t sector_index = cluster_index / (ufs->conf->api->u16numberByteOfSector / 2);  // Divide by half the sector size (because of 16-bit values)
    uint16_t offset_within_sector = cluster_index % (ufs->conf->api->u16numberByteOfSector / 2);  // Offset within the sector for the specific cluster entry

    // Allocate memory for the sector buffer
    uint8_t data_sector[ufs->conf->api->u16numberByteOfSector];

    uint16_t *valueCluster = NULL;

    // Read the sector that contains the cluster map entry
    ufs->conf->api->ReadSector(ufs->ClusterMappingZoneFirstSector + sector_index, data_sector, ufs->conf->api->u16numberByteOfSector);

    // Locate the exact position of the cluster map entry in the sector
    valueCluster = (uint16_t *)&data_sector[offset_within_sector * 2];  // Each entry is 2 bytes, so multiply offset by 2

    // Update the value in the cluster map
    *valueCluster = value;

    // Write the updated sector back to the UFS
    ufs->conf->api->EraseSector(ufs->ClusterMappingZoneFirstSector + sector_index);
    ufs->conf->api->WriteSector(ufs->ClusterMappingZoneFirstSector + sector_index, data_sector, ufs->conf->api->u16numberByteOfSector);

    // Return success
    return UFS_OK;
}

/**
 * @brief   Updates file information in the UFS file system.
 *
 * This function reads the sector where the file (item) information is stored,
 * updates the information in memory, and writes the updated sector back to the
 * UFS (Universal File System).
 *
 * @param[in]   ufs   Pointer to the UFS (Universal File System) structure.
 * @param[in]   item  Pointer to the item (file) whose information will be updated.
 *
 * @return      ufs_ReturnType    UFS_OK on success, UFS_NOT_OK on failure.
 */
static ufs_ReturnType ufs_UpdateItemInfo(UFS *ufs, ufs_Item_Type *item)
{
    // Allocate memory for the sector data
    uint8_t data_sector[ufs->conf->api->u16numberByteOfSector];

    // Check if the item has a valid sector ID and no prior error
    if (item->err != UFS_ERROR_NONE || item->location.sector_id == 0xFFFF)
    {
        item->err = UFS_ERROR_INVALID_SECTOR;
        return UFS_NOT_OK;
    }

    // Read the sector containing the item information
    ufs->conf->api->ReadSector(ufs->ItemZoneFirstSector + item->location.sector_id, data_sector, ufs->conf->api->u16numberByteOfSector);
    // Decode Header
    for(uint16_t countByte = 0; countByte < ufs->conf->api->u16numberByteOfSector; countByte ++)
    {
    	if(data_sector[countByte] != 0x00)
    	{
    		data_sector[countByte] ^= BYTE_CODEC_DEFAULT;
    	}
    }

    // Update the item information in the sector data
    memcpy(&data_sector[item->location.position * sizeof(ufs_ItemInfo_Type)], item->info.data, sizeof(ufs_ItemInfo_Type));

    // Encode Header
    for(uint16_t countByte = 0; countByte < ufs->conf->api->u16numberByteOfSector; countByte ++)
    {
    	if(data_sector[countByte] != 0x00)
    	{
    		data_sector[countByte] ^= BYTE_CODEC_DEFAULT;
    	}
    }

    // Write the updated sector back to the UFS
    ufs->conf->api->EraseSector(ufs->ItemZoneFirstSector + item->location.sector_id);
    ufs->conf->api->WriteSector(ufs->ItemZoneFirstSector + item->location.sector_id, data_sector, ufs->conf->api->u16numberByteOfSector);

    // Return success
    return UFS_OK;
}

/**
 * @brief   Performs a fast format of the UFS device.
 *
 * This function formats the UFS by erasing and re-initializing the boot sector,
 * item zone, and cluster mapping sectors. It also sets up necessary fields such as
 * the device ID and sector information.
 *
 * @param[in]   ufs   Pointer to the UFS (Universal File System) structure.
 *
 * @return      ufs_ReturnType    UFS_OK on success, UFS_NOT_OK on failure.
 */
ufs_ReturnType ufs_FastFormat(UFS *ufs)
{
    // Allocate memory for one sector's worth of data
    uint8_t data_sector[ufs->conf->api->u16numberByteOfSector];

    // Format the boot sector by erasing it
    ufs->conf->api->EraseBlock(BOOT_SECTOR_ID);
    ufs->conf->api->EraseBlock(BOOT_SECTOR_ID + 1);
    //ufs->conf->api->EraseSector(BOOT_SECTOR_ID);

    // Calculate key values to avoid redundant calculations
    uint16_t sector_size = ufs->conf->api->u16numberByteOfSector;
    uint16_t total_sectors = ufs->conf->api->u32numberSectorOfDevice;
    uint16_t max_files = ufs->conf->u8NumberFileMaxOfDevice;

    // Set up the item zone and cluster mapping zone
    ufs->ItemZoneFirstSector = 0x01;
    ufs->ClusterMappingZoneFirstSector = ((sizeof(ufs_ItemInfo_Type) * max_files) / sector_size) + ufs->ItemZoneFirstSector + 1;

    // Calculate sectors required for cluster mapping
    uint16_t numberClusterMappingOfSector = sector_size / 2;
    uint16_t numberSectorForClusterMapping = ((total_sectors - (ufs->ClusterMappingZoneFirstSector + 1)) / numberClusterMappingOfSector) + 1;

    // Calculate maximum sectors for cluster mapping and set up the number of sectors per cluster
    uint16_t numberSectorMaxForClusterMapping = total_sectors / 50;
    ufs->NumberSectorOfCluster = numberSectorForClusterMapping / numberSectorMaxForClusterMapping + 1;

    if(ufs->conf->api->u16numberSectorOfBlock != 0 && ufs->NumberSectorOfCluster < ufs->conf->api->u16numberSectorOfBlock)
    {
    	ufs->NumberSectorOfCluster = ufs->conf->api->u16numberSectorOfBlock;
    }

    // Determine the start of the cluster data zone
    ufs->ClusterDataZoneFirstSector = ufs->ClusterMappingZoneFirstSector +
                                      ((ufs->NumberSectorOfCluster != 1) ? numberSectorMaxForClusterMapping : numberSectorForClusterMapping) + 1;

    ufs->ClusterDataZoneFirstSector = (ufs->ClusterDataZoneFirstSector + 0x0F) & ~0x0F;

    // Read and store the unique device ID
    ufs->conf->api->ReadUniqueID(ufs->DeviceId, 8);

    // Initialize boot sector metadata
    memset(data_sector, 0x00, sector_size);
    data_sector[0] = 'U';
    data_sector[1] = 'F';
    data_sector[2] = 'S';
    data_sector[4] = (ufs->ItemZoneFirstSector >> 8) & 0xFF;
    data_sector[5] = ufs->ItemZoneFirstSector & 0xFF;
    data_sector[6] = (ufs->ClusterMappingZoneFirstSector >> 8) & 0xFF;
    data_sector[7] = ufs->ClusterMappingZoneFirstSector & 0xFF;
    data_sector[8] = (ufs->ClusterDataZoneFirstSector >> 8) & 0xFF;
    data_sector[9] = ufs->ClusterDataZoneFirstSector & 0xFF;
    data_sector[10] = (ufs->NumberSectorOfCluster >> 8) & 0xFF;
    data_sector[11] = ufs->NumberSectorOfCluster & 0xFF;

    // Copy the device ID into the boot sector
    memcpy(&data_sector[12], ufs->DeviceId, 8);

    // Add end-of-sector markers
    data_sector[sector_size - 3] = '\r';
    data_sector[sector_size - 2] = '\n';
    data_sector[sector_size - 1] = ufs_CheckSum(data_sector, sector_size - 1);

    // Write the boot sector to the device
    ufs->conf->api->WriteSector(BOOT_SECTOR_ID, data_sector, sector_size);

    // Format the item zone by erasing and initializing each sector
    memset(data_sector, 0x00, sector_size);
    for (uint16_t countSector = 0; countSector < (ufs->ClusterMappingZoneFirstSector - ufs->ItemZoneFirstSector); countSector++)
    {
    	if(countSector == 0)
    	{
    		// Folder '/' for root in first sector of item zone
    		data_sector[0] = (uint8_t)'/' ^ BYTE_CODEC_DEFAULT;
    	}
    	//ufs->conf->api->EraseSector(ufs->ItemZoneFirstSector + countSector);
        ufs->conf->api->WriteSector(ufs->ItemZoneFirstSector + countSector, data_sector, sector_size);
    }

    // Format the cluster mapping zone by erasing and initializing each sector
    memset(data_sector, 0xFF, sector_size);  // Pre-fill data_sector with 0xFF for efficiency
    for (uint16_t countSector = 0; countSector < (ufs->ClusterDataZoneFirstSector - ufs->ClusterMappingZoneFirstSector); countSector++)
    {
    	if(countSector == 0)
    	{
    		// Cluter Mapping for root is UFS_CLUSTER_END
    		data_sector[0] = 0xFF ^ BYTE_CODEC_DEFAULT;
    		data_sector[1] = 0xFD ^ BYTE_CODEC_DEFAULT;
    	}
    	//ufs->conf->api->EraseBlock(ufs->ClusterMappingZoneFirstSector + countSector);
        ufs->conf->api->WriteSector(ufs->ClusterMappingZoneFirstSector + countSector, data_sector, sector_size);
    }

    ufs->path.id = 0;
    ufs->path.name = (uint8_t *)"/";
    // Set the used size to zero since the device has been formatted
    ufs->UsedSize = 0;

    return UFS_OK;
}

/**
 * @brief   Initializes a new UFS instance.
 *
 * This function sets up a new UFS (Universal File System) instance by
 * initializing the UFS configuration, reading the boot sector, and
 * configuring internal parameters.
 *
 * @param[in]  pUfsCfg   Pointer to the UFS configuration structure.
 *
 * @return     UFS*      Pointer to the initialized UFS instance, or NULL if
 *                       initialization failed.
 */
UFS *newUFS(ufs_Cfg_Type *pUfsCfg)
{
    // Check if the required API functions are set
    if (pUfsCfg->api->Init == NULL || pUfsCfg->api->ReadSector == NULL ||
        pUfsCfg->api->WriteSector == NULL || pUfsCfg->api->EraseSector == NULL ||
        pUfsCfg->api->EraseBlock == NULL || pUfsCfg->api->EraseChip == NULL||
		pUfsCfg->api->ReadUniqueID == NULL)
    {
        return NULL;  // Return NULL if any essential function is missing
    }

    // Allocate memory for sector data and UFS instance
    uint8_t data_sector[pUfsCfg->api->u16numberByteOfSector];

    UFS *ufs = (UFS *)malloc(sizeof(UFS));
    if (!ufs)
    {
        return NULL;
    }

    // Initialize UFS configuration
    ufs->conf = pUfsCfg;
    ufs->conf->api->Init();

    // Read boot sector
    ufs->conf->api->ReadSector(BOOT_SECTOR_ID, data_sector, ufs->conf->api->u16numberByteOfSector);

    // Check if the boot sector is valid
    if (UFS_OK != ufs_BytesCmp(data_sector, (uint8_t *)"UFS", 3) ||
        UFS_OK != ufs_BytesCmp(&data_sector[ufs->conf->api->u16numberByteOfSector - 3], (uint8_t *)"\r\n", 2) ||
        data_sector[ufs->conf->api->u16numberByteOfSector - 1] != ufs_CheckSum(data_sector, ufs->conf->api->u16numberByteOfSector - 1))
    {
        // Perform fast format if boot sector is invalid
        ufs_FastFormat(ufs);
        return ufs;
    }

    // Initialize UFS parameters from the boot sector
    ufs->latest_cluster.sector_id = 0x00;
    ufs->latest_cluster.position = 0x00;

    ufs->ItemZoneFirstSector = (data_sector[4] << 8) | data_sector[5];
    ufs->ClusterMappingZoneFirstSector = (data_sector[6] << 8) | data_sector[7];
    ufs->ClusterDataZoneFirstSector = (data_sector[8] << 8) | data_sector[9];
    ufs->NumberSectorOfCluster = (data_sector[10] << 8) | data_sector[11];

    // Copy device ID from the boot sector
    memcpy(ufs->DeviceId, &data_sector[12], 8);

    ufs->conf->api->ReadSector(ufs->ClusterMappingZoneFirstSector + 0, data_sector, ufs->conf->api->u16numberByteOfSector);

    // Calculate the used size of the UFS
    ufs->UsedSize = ufs_GetUsedSize(ufs);

    ufs->path.id = 0x00;
    ufs->path.name   = (uint8_t *)"/";

    return ufs;
}

/**
 * @brief Opens a file in the UFS system.
 *
 * This function searches for a file in the UFS system by its name. If the file
 * exists, it loads its metadata and cluster information. If it doesn't exist,
 * it attempts to create a new file entry in the UFS, allocating necessary clusters.
 *
 * @param[in]  ufs        Pointer to the UFS structure.
 * @param[in]  name_file  Pointer to the file name string.
 * @param[in]  item       Pointer to the UFS item structure where metadata will be stored.
 *
 * @return ufs_ReturnType UFS_OK on success, UFS_NOT_OK if the file cannot be opened.
 */
__fast
ufs_ReturnType ufs_OpenItem(UFS *ufs, uint8_t *name_file, ufs_Item_Type *item)
{
    ufs_Location_Type slotItem; // Temporary variable to hold available slot information.
    uint8_t data_sector[ufs->conf->api->u16numberByteOfSector]; // Allocates memory to read sector data.

    // Check for memory allocation failure or invalid UFS pointer.
    if (ufs == NULL)
    {
        item->err = UFS_ERROR_ALLOCATE_MEM;
        return UFS_NOT_OK;
    }

    // Lock the mutex to ensure thread safety (check LockMutex and mutex)
    if (ufs->conf->api->LockMutex && ufs->conf->api->mutex)
    {
        ufs->conf->api->LockMutex((void *)ufs->conf->api->mutex);  // Lock the mutex
    }

    // Initialize the slotItem and item structures to default values.
    slotItem.sector_id = 0xFFFF;
    slotItem.position  = 0xFFFF;

    item->location.sector_id = 0xFFFF;
    item->status = UFS_ITEM_FREE;
    item->ufs = NULL;

    // Parse the name of the file and store it in the item structure.
    ufs_ParseNameFile(name_file, &item->info.comp.name);

    // Iterate through the sectors in the item zone to find the file or an empty slot.
    for (uint16_t countSector = 0; countSector < (ufs->ClusterMappingZoneFirstSector - ufs->ItemZoneFirstSector); countSector++)
    {
        ufs->conf->api->ReadSector(ufs->ItemZoneFirstSector + countSector, data_sector, ufs->conf->api->u16numberByteOfSector);
        //Decode
        for(uint16_t countByte = 0; countByte < ufs->conf->api->u16numberByteOfSector; countByte ++)
        {
        	if(data_sector[countByte] != 0x00)
        	{
        		data_sector[countByte] ^= BYTE_CODEC_DEFAULT;
        	}
        }
        // Iterate through segments within the sector to search for the file.
        for (uint16_t countSegment = 0; countSegment < (ufs->conf->api->u16numberByteOfSector / sizeof(ufs_ItemInfo_Type)); countSegment++)
        {
            // Check if the file name matches the current segment.
        	uint16_t *path_id = (uint16_t *)&data_sector[countSegment * sizeof(ufs_ItemInfo_Type) + 24u];
            if (
                UFS_OK == ufs_BytesCmp(item->info.comp.name.head, &data_sector[countSegment * sizeof(ufs_ItemInfo_Type)], item->info.comp.name.length) &&
                UFS_OK == ufs_BytesCmp(item->info.comp.name.extention, &data_sector[countSegment * sizeof(ufs_ItemInfo_Type) + MAX_NAME_LENGTH], 3) &&
				ufs->path.id == *path_id &&
                item->info.comp.name.length == data_sector[countSegment * sizeof(ufs_ItemInfo_Type) + MAX_NAME_LENGTH + 3])
            {
                // If the file is found, update item metadata and exit the loop.
                item->location.sector_id = countSector;
                item->location.position  = countSegment;
                memcpy(item->info.data, &data_sector[countSegment * sizeof(ufs_ItemInfo_Type)], sizeof(ufs_ItemInfo_Type));
                if(item->info.comp.name.extention[0] != 0x00)
                {
                	ufs_GetListCluster(ufs, item);
                }

                break;
            }

            // Save the first empty slot in case we need to create a new file.
            if (data_sector[countSegment * sizeof(ufs_ItemInfo_Type)] == UFS_ITEM_FREE)
            {
                slotItem.sector_id = countSector;
                slotItem.position = countSegment;
            }
        }

        // If the file was found, break out of the outer loop.
        if (item->location.sector_id != 0xFFFF)
        {
            break;
        }
    }

    // If no existing file was found, attempt to create a new item in the UFS.
    if (item->location.sector_id == 0xFFFF && slotItem.sector_id != 0xFFFF)
    {
    	if(item->info.comp.name.extention[0] != 0x00)
    	{
			uint16_t *value_cluster;
			uint16_t countSector = 0;
			uint16_t countSegment = 0;

			// Initialize item metadata for the new file.
			item->info.comp.size = 0;
			item->info.comp.parent = ufs->path.id;

			item->info.comp.first_cluster.sector_id = 0xFFFF;

			// Search for available clusters to assign to the new file.
			countSector = ufs->latest_cluster.sector_id;
			do
			{
				// Loop through the cluster mapping zone to find an available cluster.
				if (++countSector >= (ufs->ClusterDataZoneFirstSector - ufs->ClusterMappingZoneFirstSector))
				{
					countSector = 0x00;
				}

				if(countSector * (ufs->conf->api->u16numberByteOfSector / 2) > \
				   (ufs->conf->api->u32numberSectorOfDevice - ufs->ClusterDataZoneFirstSector) / ufs->NumberSectorOfCluster)
				{
					countSector = 0;
				}

				ufs->conf->api->ReadSector(ufs->ClusterMappingZoneFirstSector + countSector, data_sector, ufs->conf->api->u16numberByteOfSector);

				countSegment = ufs->latest_cluster.position;
				do
				{
					if (++countSegment >= (ufs->conf->api->u16numberByteOfSector / 2))
					{
						countSegment = 0x00;
					}

					if(countSegment >= (ufs->conf->api->u32numberSectorOfDevice - ufs->ClusterDataZoneFirstSector) / ufs->NumberSectorOfCluster)
					{
						countSegment = 0x00;
					}

					value_cluster = (uint16_t *)&data_sector[countSegment * 2];
					if (*value_cluster == UFS_CLUSTER_FREE)
					{
						// Assign the found cluster to the new file.
						item->info.comp.first_cluster.sector_id = countSector;
						item->info.comp.first_cluster.position = countSegment;

						ufs->latest_cluster.sector_id = countSector;
						ufs->latest_cluster.position = countSegment;
						break;
					}

				} while (countSegment != ufs->latest_cluster.position);

			} while (countSector != ufs->latest_cluster.sector_id);

			// If a valid cluster was found, finalize the file creation.
			if (item->info.comp.first_cluster.sector_id != 0xFFFF)
			{
				*value_cluster = UFS_CLUSTER_END;
				ufs->conf->api->EraseSector(ufs->ClusterMappingZoneFirstSector + item->info.comp.first_cluster.sector_id);
				ufs->conf->api->WriteSector(ufs->ClusterMappingZoneFirstSector + item->info.comp.first_cluster.sector_id, data_sector, ufs->conf->api->u16numberByteOfSector);

				// Update the item zone with the new file entry.
				item->location.sector_id = slotItem.sector_id;
				item->location.position  = slotItem.position;

				ufs->conf->api->ReadSector(ufs->ItemZoneFirstSector + item->location.sector_id, data_sector, ufs->conf->api->u16numberByteOfSector);
				for(uint16_t countByte = 0; countByte < ufs->conf->api->u16numberByteOfSector; countByte ++)
				{
					if(data_sector[countByte] != 0x00)
					{
						data_sector[countByte] ^= BYTE_CODEC_DEFAULT;
					}
				}

				item->info.comp.parent = ufs->path.id;
				memcpy(&data_sector[item->location.position * sizeof(ufs_ItemInfo_Type)], item->info.data, sizeof(ufs_ItemInfo_Type));

				for(uint16_t countByte = 0; countByte < ufs->conf->api->u16numberByteOfSector; countByte ++)
				{
					if(data_sector[countByte] != 0x00)
					{
						data_sector[countByte] ^= BYTE_CODEC_DEFAULT;
					}
				}

				ufs->conf->api->EraseSector(ufs->ItemZoneFirstSector + item->location.sector_id);
				ufs->conf->api->WriteSector(ufs->ItemZoneFirstSector + item->location.sector_id, data_sector, ufs->conf->api->u16numberByteOfSector);

				ufs_GetListCluster(ufs, item);

				ufs->UsedSize += ufs->conf->api->u16numberByteOfSector * ufs->NumberSectorOfCluster;
			}
			else
			{
				item->err = UFS_ERROR_FULL_CLUSTER;

				// Unlock the mutex after the file operation (check UnlockMutex and mutex)
				if (ufs->conf->api->UnlockMutex && ufs->conf->api->mutex)
				{
					ufs->conf->api->UnlockMutex((void *)ufs->conf->api->mutex);  // Unlock the mutex
				}
				return UFS_NOT_OK;
			}
    	}
    	else
    	{
			// Update the item zone with the new file entry.
			item->location.sector_id = slotItem.sector_id;
			item->location.position  = slotItem.position;
			item->clusters.length = 0;
			item->info.comp.first_cluster.sector_id = 0x00;
			item->info.comp.first_cluster.position = 0x00;
			item->err = UFS_ERROR_NONE;

			ufs_UpdateItemInfo(ufs, item);
    	}
    }
    else if (item->location.sector_id == 0xFFFF && slotItem.sector_id == 0xFFFF)
    {
        // No file and no available space for a new file.
        item->err = UFS_ERROR_FULL_FILE;

        // Unlock the mutex after the file operation (check UnlockMutex and mutex)
        if (ufs->conf->api->UnlockMutex && ufs->conf->api->mutex)
        {
            ufs->conf->api->UnlockMutex((void *)ufs->conf->api->mutex);  // Unlock the mutex
        }
        return UFS_NOT_OK;
    }

    for(uint8_t count = 0; count < ufs->conf->u8NumberEncodeFileExtension; count ++)
    {
    	if(UFS_OK == ufs_BytesCmp((uint8_t *)ufs->conf->pExtensionEncodeFileList[count].pListExtensionName, item->info.comp.name.extention, 3))
    	{
    		item->EncodeEnable = UFS_ENCODE_ENABLE;
    	}
    }

    // Mark the item as successfully opened.
    item->ufs = ufs;
    item->err = UFS_ERROR_NONE;
    if(item->info.comp.name.extention[0] != 0x00)
    {
    	item->status = UFS_FILE_EXIST;
    }
    else
    {
    	item->status = UFS_FOLDER_EXIST;
    }

    // Unlock the mutex after the file operation (check UnlockMutex and mutex)
    if (ufs->conf->api->UnlockMutex && ufs->conf->api->mutex)
    {
        ufs->conf->api->UnlockMutex((void *)ufs->conf->api->mutex);  // Unlock the mutex
    }
    return UFS_OK;
}

/**
 * @brief Closes a UFS item and releases allocated memory.
 *
 * This function frees memory associated with the cluster list of the item
 * and resets all relevant fields to indicate that the item is no longer in use.
 *
 * @param[in] item Pointer to the UFS item structure.
 *
 * @return ufs_ReturnType UFS_OK on success, UFS_NOT_OK if the item is invalid.
 */
ufs_ReturnType ufs_CloseItem(ufs_Item_Type *item)
{
    if (item == NULL)
    {
        item->err = UFS_ERROR_ALLOCATE_MEM;
        return UFS_NOT_OK;
    }

    // Free the cluster list and reset length
    free(item->clusters.value);
    item->clusters.length = 0;

    // Reset item info to default values
    item->info.data[0] = UFS_ITEM_FREE;
    item->info.comp.name.length = 0;
    item->info.comp.size = 0;
    item->info.comp.first_cluster.sector_id = 0xFFFF;
    item->info.comp.first_cluster.position = 0;

    // Reset item location
    item->location.sector_id = 0xFFFF;
    item->location.position = 0;

    // Mark the item as free and detach it from UFS
    item->status = UFS_ITEM_FREE;
    item->ufs = NULL;

    return UFS_OK;
}

/**
 * @brief Deletes a file from the UFS system.
 *
 * This function cleans the cluster list associated with the file and resets
 * the item fields, marking it as deleted in the UFS.
 *
 * @param[in] item Pointer to the UFS item structure.
 *
 * @return ufs_ReturnType UFS_OK on success, UFS_NOT_OK if the item is invalid.
 */
ufs_ReturnType ufs_DeleteItem(ufs_Item_Type *item)
{
    if (item == NULL || item->ufs == NULL)
    {
        item->err = UFS_ERROR_ALLOCATE_MEM;
        return UFS_NOT_OK;
    }

    if(item->err != UFS_ERROR_NONE)
    {
    	item->err = UFS_ERROR_NOT_EXISTED;
    	return UFS_NOT_OK;
    }

    // Update cluter  list
    ufs_GetListCluster(item->ufs, item);

    // Clean up the cluster list
    ufs_CleanClusters(item->ufs, item->clusters.value, item->clusters.length);

    free(item->clusters.value);
    item->clusters.length = 0;

    // Reset item info to indicate deletion
    item->info.data[0] = UFS_ITEM_FREE;
    item->info.comp.name.length = 0;
    item->info.comp.size = 0;
    item->info.comp.first_cluster.sector_id = 0xFFFF;
    item->info.comp.first_cluster.position = 0;

    // Update UFS with the changes
    ufs_UpdateItemInfo(item->ufs, item);

    // Reset item location
    item->location.sector_id = 0xFFFF;
    item->location.position = 0;

    // Mark the item as free and detach it from UFS
    item->status = UFS_ITEM_FREE;
    item->ufs = NULL;

    return UFS_OK;
}

/**
 * @brief   Counts the number of used items in the UFS item zone.
 *
 * This function iterates over the sectors in the item zone and counts
 * the number of non-free items. The item zone is specified by the
 * starting and ending sector numbers defined in the UFS structure.
 *
 * @param[in]   ufs   Pointer to the UFS (Universal File System) structure.
 *                    It contains configuration and function pointers for accessing
 *                    the hardware API and memory mapping.
 *
 * @return      uint16_t  The number of used items found in the UFS item zone.
 *                        Returns 0 if there was an error (e.g., memory allocation failure).
 *
 * @note        The function allocates memory dynamically for sector data. Ensure
 *              that there is enough system memory available, and the allocated memory
 *              is properly freed after usage.
 */
uint16_t ufs_CountItem(UFS *ufs)
{
    uint16_t count = 0;
    // Allocate memory for one sector worth of data
    uint8_t data_sector[ufs->conf->api->u16numberByteOfSector];

    uint16_t totalSectors = ufs->ClusterMappingZoneFirstSector - ufs->ItemZoneFirstSector;
    uint16_t itemsPerSector = ufs->conf->api->u16numberByteOfSector / sizeof(ufs_ItemInfo_Type);

    for (uint16_t sector = 0; sector < totalSectors; sector ++)
    {
        // Read sector data into the allocated memory
        ufs->conf->api->ReadSector(ufs->ItemZoneFirstSector + sector, data_sector, ufs->conf->api->u16numberByteOfSector);
        for(uint16_t countByte = 0; countByte < ufs->conf->api->u16numberByteOfSector; countByte ++)
        {
        	if(data_sector[countByte] != 0x00)
        	{
        		data_sector[countByte] ^= BYTE_CODEC_DEFAULT;
        	}
        }

        // Iterate through each item in the sector
        for (uint16_t segment = 0; segment < itemsPerSector; segment++)
        {
            uint8_t  itemStatus = data_sector[segment * sizeof(ufs_ItemInfo_Type)];
            uint16_t parent = data_sector[segment * sizeof(ufs_ItemInfo_Type) + 24u];

            // If the item is not free, count it
            if (itemStatus != UFS_ITEM_FREE && parent == ufs->path.id)
            {
                count++;
            }
        }
    }

    return count;       // Return the count of items
}

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
ufs_ReturnType ufs_CheckExistence(UFS *ufs, uint8_t *name, ufs_Item_Type *item)
{
    uint8_t data_sector[ufs->conf->api->u16numberByteOfSector]; // Buffer for reading sector data

    // Parse the file or directory name
    ufs_ParseNameFile(name, &item->info.comp.name);

    item->err = UFS_ERROR_NONE;
    // Traverse each sector in the item zone to search for the item
    for (uint16_t countSector = 0; countSector < (ufs->ClusterMappingZoneFirstSector - ufs->ItemZoneFirstSector); countSector++)
    {
        // Read data from the current sector
        ufs->conf->api->ReadSector(ufs->ItemZoneFirstSector + countSector, data_sector, ufs->conf->api->u16numberByteOfSector);

        // Decode the sector data
        for (uint16_t countByte = 0; countByte < ufs->conf->api->u16numberByteOfSector; countByte++)
        {
            if (data_sector[countByte] != 0x00)
            {
                data_sector[countByte] ^= BYTE_CODEC_DEFAULT;
            }
        }

        // Traverse each segment within the sector
        for (uint16_t countSegment = 0; countSegment < (ufs->conf->api->u16numberByteOfSector / sizeof(ufs_ItemInfo_Type)); countSegment++)
        {
            // Retrieve the path ID of the current segment
            uint16_t *path_id = (uint16_t *)&data_sector[countSegment * sizeof(ufs_ItemInfo_Type) + 24u];

            // Compare the item name, type, and path ID to check for a match within the mounted folder
            if (
                UFS_OK == ufs_BytesCmp(item->info.comp.name.head, &data_sector[countSegment * sizeof(ufs_ItemInfo_Type)], item->info.comp.name.length) &&
                UFS_OK == ufs_BytesCmp(item->info.comp.name.extention, &data_sector[countSegment * sizeof(ufs_ItemInfo_Type) + MAX_NAME_LENGTH], 3) &&
				item->info.comp.name.length == data_sector[countSegment * sizeof(ufs_ItemInfo_Type) + MAX_NAME_LENGTH + 3] &&
                ufs->path.id == *path_id)
            {
                // Populate the item structure with details of the found item
                item->location.sector_id = countSector;
                item->location.position = countSegment;
                item->status = (item->info.comp.name.extention[0] == 0x00) ? UFS_FOLDER_EXIST : UFS_FILE_EXIST;
                item->err = UFS_ERROR_NONE;
                item->ufs = ufs;
                memcpy(item->info.data, &data_sector[countSegment * sizeof(ufs_ItemInfo_Type)], sizeof(ufs_ItemInfo_Type));
                return UFS_OK;
            }
        }
    }

    // If the item was not found, mark it as not existing and return failure
    item->location.sector_id = 0xFFFF;
    item->location.position = 0xFFFF;
    item->status = UFS_ITEM_FREE;
    item->err = UFS_ERROR_NOT_EXISTED;
    return UFS_NOT_OK;
}

/**
 * @brief   Retrieves a list of used items from the UFS item zone.
 *
 * This function reads sectors from the item zone and copies the information of
 * non-free items into the provided array of `ufs_ItemInfo_Type`. It stops when
 * the specified number of items (`length`) has been read or when there are no
 * more items in the item zone.
 *
 * @param[in]   ufs         Pointer to the UFS (Universal File System) structure,
 *                          which contains configuration and hardware API details.
 *
 * @param[out]  item_info   Pointer to an array of `ufs_ItemInfo_Type` structures
 *                          where the retrieved item information will be stored.
 *
 * @param[in]   length      The maximum number of items to retrieve. This determines
 *                          how many items will be copied to the `item_info` array.
 *
 * @return      uint16_t    The number of items successfully read and copied into
 *                          the `item_info` array.
 *
 * @note        The function allocates memory for sector data and frees it after
 *              use. Ensure that the `item_info` array is large enough to hold the
 *              number of items specified by `length`.
 */
uint16_t ufs_GetListItem(UFS *ufs, ufs_ItemInfo_Type *item_info, uint16_t length)
{
    uint16_t item_read = 0;
    // Allocate memory for one sector worth of data
    uint8_t data_sector[ufs->conf->api->u16numberByteOfSector];

    uint16_t totalSectors = ufs->ClusterMappingZoneFirstSector - ufs->ItemZoneFirstSector;
    uint16_t itemsPerSector = ufs->conf->api->u16numberByteOfSector / sizeof(ufs_ItemInfo_Type);

    for (uint16_t sector = 0; sector < totalSectors; sector++)
    {
        // Read sector data into the allocated memory
        ufs->conf->api->ReadSector(ufs->ItemZoneFirstSector + sector, data_sector, ufs->conf->api->u16numberByteOfSector);
        for(uint16_t countByte = 0; countByte < ufs->conf->api->u16numberByteOfSector; countByte ++)
        {
        	if(data_sector[countByte] != 0x00)
        	{
        		data_sector[countByte] ^= BYTE_CODEC_DEFAULT;
        	}
        }
        // Iterate through each item in the sector
        for (uint16_t segment = 0; segment < itemsPerSector; segment++)
        {
            uint8_t itemStatus = data_sector[segment * sizeof(ufs_ItemInfo_Type)];
            uint16_t parent = data_sector[segment * sizeof(ufs_ItemInfo_Type) + 24u];

            // If the item is not free, copy its data
            if (itemStatus != UFS_ITEM_FREE &&  parent == ufs->path.id)
            {
                memcpy(item_info[item_read].data, &data_sector[segment * sizeof(ufs_ItemInfo_Type)], sizeof(ufs_ItemInfo_Type));

                // If the desired number of items has been read, return immediately
                if (++item_read == length)
                {
                    return length;
                }
            }
        }
    }

    return item_read;   // Return the actual number of items read
}

/**
 * @brief   Calculates the total used size in the UFS item zone.
 *
 * This function computes the total size used by items in the UFS item zone.
 * It first counts the number of non-free items, retrieves the list of items,
 * and then sums their sizes in terms of the number of sectors used.
 *
 * @param[in]   ufs     Pointer to the UFS (Universal File System) structure,
 *                      which contains configuration and hardware API details.
 *
 * @return      uint32_t    The total size of used items in bytes. If there is
 *                          an error (e.g., memory allocation failure), the function
 *                          will return the calculated size up to the failure point.
 *
 * @note        The function dynamically allocates memory for storing item information.
 *              Ensure sufficient memory is available.
 */
uint32_t ufs_GetUsedSize(UFS *ufs)
{
    uint32_t used_size = 0;
    ufs_ItemInfo_Type *items_info = NULL;

    // Get the number of items in the item zone
    uint16_t numberItem = ufs_CountItem(ufs);

    // Allocate memory to store information about each item
    items_info = (ufs_ItemInfo_Type *)calloc(numberItem, sizeof(ufs_ItemInfo_Type));
    if (!items_info)
    {
        // Handle memory allocation failure
        return used_size;
    }

    // Lock the mutex to ensure thread safety (check LockMutex and mutex)
    if (ufs->conf->api->LockMutex && ufs->conf->api->mutex)
    {
        ufs->conf->api->LockMutex((void *)ufs->conf->api->mutex);  // Lock the mutex
    }

    // Retrieve the list of used items
    ufs_GetListItem(ufs, items_info, numberItem);

    // Iterate through the items and calculate their used size
    for (uint16_t countItem = 0; countItem < numberItem; countItem++)
    {
        uint32_t itemSize = items_info[countItem].comp.size;
        uint32_t sectorSize = ufs->conf->api->u16numberByteOfSector;

        // Calculate the total size for this item in terms of full sectors
        used_size += ((itemSize / sectorSize) + 1) * sectorSize;
    }

    // Free the allocated memory for items_info
    free(items_info);

    ufs->UsedSize = used_size;

    // Unlock the mutex after the file operation (check UnlockMutex and mutex)
    if (ufs->conf->api->UnlockMutex && ufs->conf->api->mutex)
    {
        ufs->conf->api->UnlockMutex((void *)ufs->conf->api->mutex);  // Unlock the mutex
    }
    return used_size;
}

/**
 * @brief   Retrieves the total usable size of the UFS device.
 *
 * This function calculates the usable size of the device based on the number of
 * sectors available in the device, starting from the first data zone sector.
 *
 * @param[in]   ufs     Pointer to the UFS (Universal File System) structure,
 *                      which contains configuration and hardware API details.
 *
 * @return      uint32_t    The total usable device size in bytes. If the input
 *                          `ufs` or `ufs->conf` is NULL, the function returns 0.
 */
uint32_t ufs_GetDeviceSize(UFS *ufs)
{
    // Ensure the input pointers are valid
    if (ufs != NULL && ufs->conf != NULL)
    {
        uint16_t sectorSize = ufs->conf->api->u16numberByteOfSector;
        uint32_t totalSectors = ufs->conf->api->u32numberSectorOfDevice;
        uint32_t dataZoneStartSector = ufs->ClusterDataZoneFirstSector;

        // Calculate the usable size starting from the data zone
        return sectorSize * (totalSectors - dataZoneStartSector);
    }

    // Return 0 if input pointers are invalid
    return 0;
}

/**
 * @brief   Reads data from a file in UFS.
 *
 * This function reads data from the specified file, starting from the given position
 * and continuing for the specified length. The data is copied into the provided buffer.
 *
 * @param[in]   file      Pointer to the UFS file structure.
 * @param[in]   position  The starting position within the file from where to begin reading.
 * @param[out]  data      Pointer to the buffer where the read data will be stored.
 * @param[in]   length    The number of bytes to read from the file.
 *
 * @return      uint32_t  The number of bytes successfully read from the file.
 */
__fast
uint32_t ufs_ReadFile(ufs_Item_Type *file, uint32_t position, uint8_t *data, uint32_t length)
{
	if(file->ufs == NULL || file->err != UFS_ERROR_NONE)
	{
		 return UFS_NOT_OK;
	}

    if(file->status != UFS_FILE_EXIST)
    {
    	file->err = UFS_ERROR_ITEM_NOT_FILE;
    	return UFS_NOT_OK;
    }

    uint32_t bytes_read = 0;
    uint32_t cluster_index = position / (file->ufs->conf->api->u16numberByteOfSector * file->ufs->NumberSectorOfCluster);
    uint32_t offset_within_cluster = position % (file->ufs->conf->api->u16numberByteOfSector * file->ufs->NumberSectorOfCluster);

    uint8_t data_sector[file->ufs->conf->api->u16numberByteOfSector];

    // Lock the mutex to ensure thread safety (check LockMutex and mutex)
    if (file->ufs->conf->api->LockMutex && file->ufs->conf->api->mutex)
    {
        file->ufs->conf->api->LockMutex((void *)file->ufs->conf->api->mutex);  // Lock the mutex
    }

    // Loop through clusters and sectors to read the requested data
    while (bytes_read < length && cluster_index < file->clusters.length)
    {
        // Read each sector in the current cluster
    	uint16_t sector_start = offset_within_cluster / file->ufs->conf->api->u16numberByteOfSector;
    	uint16_t offset_within_sector = offset_within_cluster % file->ufs->conf->api->u16numberByteOfSector;

        for (uint16_t sector_in_cluster = sector_start; sector_in_cluster < file->ufs->NumberSectorOfCluster; sector_in_cluster++)
        {
            // Read the current sector into the buffer
            file->ufs->conf->api->ReadSector(file->ufs->ClusterDataZoneFirstSector + file->clusters.value[cluster_index] * file->ufs->NumberSectorOfCluster + sector_in_cluster,
                                             data_sector,
                                             file->ufs->conf->api->u16numberByteOfSector);

            // Copy data from sector to the output buffer
            for (uint16_t countByte = offset_within_sector; countByte < file->ufs->conf->api->u16numberByteOfSector; countByte++)
            {
                data[bytes_read] = data_sector[countByte];
                if(file->EncodeEnable == UFS_ENCODE_ENABLE)
                {
                	data[bytes_read] ^= file->ufs->DeviceId[0] |  BYTE_CODEC_DEFAULT;
                }
                bytes_read++;

                if (bytes_read == length || bytes_read == file->info.comp.size)  // Stop reading once we've read the requested amount
                {
                    // Unlock the mutex after the file operation (check UnlockMutex and mutex)
                    if (file->ufs->conf->api->UnlockMutex && file->ufs->conf->api->mutex)
                    {
                        file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);  // Unlock the mutex
                    }
                    return bytes_read;
                }
            }

            // Reset the offset for the next sector
            offset_within_sector =  0;
        }
        offset_within_cluster = 0;

        // Move to the next cluster
        cluster_index++;
    }

    // Unlock the mutex after the file operation (check UnlockMutex and mutex)
    if (file->ufs->conf->api->UnlockMutex && file->ufs->conf->api->mutex)
    {
        file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);  // Unlock the mutex
    }
    return bytes_read;  // Return the number of bytes successfully read
}

/**
 * @brief   Writes data to a file in UFS.
 *
 * This function writes the provided data to the specified file. It clears the old
 * clusters, reallocates memory for new clusters if needed, and writes the data
 * into the sectors of the file. If new clusters are required, they will be allocated
 * accordingly. The file's metadata, including the cluster count and size, will be updated.
 *
 * @param[in]   file    Pointer to the UFS file structure.
 * @param[in]   data    Pointer to the data buffer to be written.
 * @param[in]   length  Length of the data to be written (in bytes).
 *
 * @return      ufs_ReturnType  UFS_OK on success, UFS_NOT_OK on failure.
 */
__fast
ufs_ReturnType ufs_WriteFile(ufs_Item_Type *file, uint8_t *data, uint32_t length, ufs_CheckSumStatus sumEnable)
{
	if(file->ufs == NULL || file->err != UFS_ERROR_NONE)
	{
		 return UFS_NOT_OK;
	}

    if(file->status != UFS_FILE_EXIST)
    {
    	file->err = UFS_ERROR_ITEM_NOT_FILE;
    	return UFS_NOT_OK;
    }

    uint32_t cluster_size = file->ufs->conf->api->u16numberByteOfSector * file->ufs->NumberSectorOfCluster;
    uint32_t number_clusters = (length + cluster_size - 1) / cluster_size + 1;  // Calculate number of clusters needed
    uint16_t cluster_index = 0;
    uint32_t bytes_written = 0;

    uint8_t sumSector      = 0;
    uint8_t sector_buffer[file->ufs->conf->api->u16numberByteOfSector];

    // Lock the mutex to ensure thread safety (check LockMutex and mutex)
    if (file->ufs->conf->api->LockMutex && file->ufs->conf->api->mutex)
    {
        file->ufs->conf->api->LockMutex((void *)file->ufs->conf->api->mutex);  // Lock the mutex
    }

    // Clean up any old clusters used by the file
    ufs_CleanClusters(file->ufs, file->clusters.value, file->clusters.length);

    // Reallocate memory for the new cluster list
    file->clusters.value = (uint16_t *)realloc(file->clusters.value, number_clusters * sizeof(uint16_t));
    if (file->clusters.value == NULL)
    {
        // Memory allocation failure
        file->err = UFS_ERROR_ALLOCATE_MEM;

        // Unlock the mutex after the file operation (check UnlockMutex and mutex)
        if (file->ufs->conf->api->UnlockMutex && file->ufs->conf->api->mutex)
        {
            file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);  // Unlock the mutex
        }
        return UFS_NOT_OK;
    }

    // Update the cluster length to reflect the new number of clusters
    file->clusters.length = number_clusters;

    // Order the clusters for the new data
    if (ufs_OrderClusters(file->ufs, file->clusters.value, number_clusters) != UFS_OK)
    {
        // Cluster allocation failure
        file->err = UFS_ERROR_FULL_MEM;

        // Unlock the mutex after the file operation (check UnlockMutex and mutex)
        if (file->ufs->conf->api->UnlockMutex && file->ufs->conf->api->mutex)
        {
            file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);  // Unlock the mutex
        }
        return UFS_NOT_OK;
    }

    // Start writing the data to the file's clusters
    for (cluster_index = 0; cluster_index < number_clusters; cluster_index++)
    {
        for (uint16_t sector_in_cluster = 0; sector_in_cluster < file->ufs->NumberSectorOfCluster; sector_in_cluster++)
        {
            uint32_t cluster_offset = file->ufs->ClusterDataZoneFirstSector +
                                      (file->clusters.value[cluster_index] * file->ufs->NumberSectorOfCluster) + sector_in_cluster;

            // Write data into the buffer, one sector at a time
            for (uint16_t byte_in_sector = 0; byte_in_sector < file->ufs->conf->api->u16numberByteOfSector; byte_in_sector++)
            {
                if (bytes_written < length)
                {
                    sector_buffer[byte_in_sector] = data[bytes_written++];
                    if(file->EncodeEnable == UFS_ENCODE_ENABLE)
                    {
                    	sector_buffer[byte_in_sector] ^= file->ufs->DeviceId[0] | BYTE_CODEC_DEFAULT;
                    }
                }
                else
                {
                    sector_buffer[byte_in_sector] = UFS_BYTE_VALUE_AFTER_ERASE;  // Pad remaining sector bytes with 0 if needed
                }
            }

            if(sumEnable == CHECKSUM_ENABLE)
            {
            	sumSector = ufs_CheckSum(sector_buffer, file->ufs->conf->api->u16numberByteOfSector);
            }

            // Write the buffer to the current sector
            file->ufs->conf->api->WriteSector(cluster_offset, sector_buffer, file->ufs->conf->api->u16numberByteOfSector);

            if(sumEnable == CHECKSUM_ENABLE)
            {
            	file->ufs->conf->api->ReadSector(cluster_offset, sector_buffer, file->ufs->conf->api->u16numberByteOfSector);
            	if(sumSector != ufs_CheckSum(sector_buffer, file->ufs->conf->api->u16numberByteOfSector))
            	{
            		file->info.comp.size += bytes_written;
            		ufs_CleanClusters(file->ufs, &file->clusters.value[cluster_index], number_clusters - cluster_index);
            		ufs_SetClusterMap(file->ufs ,file->clusters.value[cluster_index], UFS_CLUSTER_BAD);
            	    file->err = UFS_ERROR_SUM_SECTOR_FAIL;

            	    // Unlock the mutex after the file operation
            	    if (file->ufs->conf->api->UnlockMutex && file->ufs->conf->api->mutex)
            	    {
            	        file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);
            	    }
            	    return UFS_NOT_OK;
            	}
            }

            if (bytes_written == length)  // Stop if all data has been written
            {
                break;
            }
        }

        if (bytes_written == length)
        {
            break;
        }
    }

    // Update file metadata to reflect the new size
    file->info.comp.size = length;
    file->info.comp.first_cluster.sector_id = file->clusters.value[0] / (file->ufs->conf->api->u16numberByteOfSector / 2);
    file->info.comp.first_cluster.position = file->clusters.value[0] % (file->ufs->conf->api->u16numberByteOfSector / 2);

    // Write updated metadata to the file system
    ufs_UpdateItemInfo(file->ufs, file);

    // Unlock the mutex after the file operation (check UnlockMutex and mutex)
    if (file->ufs->conf->api->UnlockMutex && file->ufs->conf->api->mutex)
    {
        file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);  // Unlock the mutex
    }

    return UFS_OK;
}

/**
 * @brief   Appends data to the end of a file in UFS.
 *
 * This function appends the provided data to the specified file. It manages clusters
 * by extending the cluster list if necessary and writing the new data to the correct
 * sectors. The function also handles partially filled clusters, writing to the free
 * space in the last cluster if available.
 *
 * @param[in]   file        Pointer to the UFS file structure.
 * @param[in]   data        Pointer to the data buffer to be appended.
 * @param[in]   length      The number of bytes to append to the file.
 * @param[in]   sumEnable   Indicates if checksum should be enabled (CHECKSUM_ENABLE/CHECKSUM_DISABLE).
 *
 * @return      ufs_ReturnType  UFS_OK on success, UFS_NOT_OK on failure.
 */
__fast
ufs_ReturnType ufs_WriteAppendFile(ufs_Item_Type *file, uint8_t *data, uint32_t length, ufs_CheckSumStatus sumEnable)
{
    if(file->ufs == NULL || file->err != UFS_ERROR_NONE)
	{
		return UFS_NOT_OK;
	}

    if(file->status != UFS_FILE_EXIST)
    {
    	file->err = UFS_ERROR_ITEM_NOT_FILE;
    	return UFS_NOT_OK;
    }

    uint32_t current_file_size = file->info.comp.size;  // Get current file size
    uint32_t new_size = current_file_size + length;     // Calculate new size after appending
    uint32_t bytes_written = 0;                         // Track how many bytes have been written
    uint32_t cluster_size = file->ufs->conf->api->u16numberByteOfSector * file->ufs->NumberSectorOfCluster; // Calculate total cluster size
    uint16_t cluster_index = 0;
    uint32_t offset_within_cluster = current_file_size % cluster_size;  // Get the offset within the last cluster
    uint8_t sumSector = 0;  // Variable to store the checksum of the sector

    // Calculate current and new number of clusters required
    uint16_t current_cluster_count = (current_file_size + cluster_size - 1) / cluster_size + 1;
    uint16_t new_cluster_count = (new_size + cluster_size - 1) / cluster_size + 1;

    // Allocate memory for sector-level buffer
    uint8_t data_sector[file->ufs->conf->api->u16numberByteOfSector];

    // Lock mutex for thread safety
    if (file->ufs->conf->api->LockMutex && file->ufs->conf->api->mutex)
    {
        file->ufs->conf->api->LockMutex((void *)file->ufs->conf->api->mutex);  // Lock the mutex
    }

    // If new clusters are needed, expand the file->clusters.value array
    if (new_cluster_count > current_cluster_count)
    {
        uint16_t additional_clusters = new_cluster_count - current_cluster_count + 1;

        // Reallocate memory for the new clusters
        file->clusters.value = (uint16_t *)realloc(file->clusters.value, new_cluster_count * sizeof(uint16_t));
        if (file->clusters.value == NULL)
        {
            file->err = UFS_ERROR_ALLOCATE_MEM;  // Handle memory allocation failure
            if (file->ufs->conf->api->LockMutex != NULL && file->ufs->conf->api->mutex)
            {
            	file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);  // Unlock the mutex
            }
            return UFS_NOT_OK;
        }

        // Allocate new clusters for the file
        if (ufs_OrderClusters(file->ufs, &file->clusters.value[current_cluster_count - 1], additional_clusters) != UFS_OK)
        {
            file->err = UFS_ERROR_FULL_MEM;  // Handle cluster allocation failure
            if (file->ufs->conf->api->LockMutex != NULL && file->ufs->conf->api->mutex)
            {
            	file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);  // Unlock the mutex
            }
            return UFS_NOT_OK;
        }

        // Set cluster map to link the newly allocated clusters
        ufs_SetClusterMap(file->ufs, file->clusters.value[current_cluster_count - 2], file->clusters.value[current_cluster_count - 1]);

        // Update the cluster length
        file->clusters.length = new_cluster_count;
    }

    // Start writing data from the last cluster
    cluster_index = current_file_size / cluster_size;
    offset_within_cluster = current_file_size % cluster_size;

    // Write data to the last cluster if it has available space
    if (offset_within_cluster > 0)
    {
        uint16_t sector_offset = offset_within_cluster / file->ufs->conf->api->u16numberByteOfSector;  // Calculate sector offset
        uint32_t cluster_offset = file->ufs->ClusterDataZoneFirstSector +
                                  file->clusters.value[cluster_index] * file->ufs->NumberSectorOfCluster + sector_offset;
        uint32_t cluster_offset_old = cluster_offset;

        // Read the sector to update the existing data
        file->ufs->conf->api->ReadSector(cluster_offset, data_sector, file->ufs->conf->api->u16numberByteOfSector);

        // Write data into the free space of the last sector
        for (uint32_t countByte = offset_within_cluster; countByte < cluster_size && bytes_written < length; countByte++)
        {
            sector_offset = countByte / file->ufs->conf->api->u16numberByteOfSector;
            cluster_offset = file->ufs->ClusterDataZoneFirstSector + file->clusters.value[cluster_index] * file->ufs->NumberSectorOfCluster + sector_offset;

            if (cluster_offset != cluster_offset_old)
            {
                // Write the old sector before moving to a new one
                if (sumEnable == CHECKSUM_ENABLE)
                {
                    sumSector = ufs_CheckSum(data_sector, file->ufs->conf->api->u16numberByteOfSector);  // Calculate checksum for the sector
                }

                file->ufs->conf->api->WriteSector(cluster_offset_old, data_sector, file->ufs->conf->api->u16numberByteOfSector);

                if (sumEnable == CHECKSUM_ENABLE)
                {
                    file->ufs->conf->api->ReadSector(cluster_offset_old, data_sector, file->ufs->conf->api->u16numberByteOfSector);
                    if (sumSector != ufs_CheckSum(data_sector, file->ufs->conf->api->u16numberByteOfSector))
                    {
                        file->info.comp.size += bytes_written;  // Update file size to account for the error
                        ufs_CleanClusters(file->ufs, &file->clusters.value[cluster_index], file->clusters.length - cluster_index);  // Clean the bad clusters
                        ufs_SetClusterMap(file->ufs, file->clusters.value[cluster_index], UFS_CLUSTER_BAD);  // Mark as bad
                        file->err = UFS_ERROR_SUM_SECTOR_FAIL;  // Set error for checksum failure

                        // Unlock the mutex after the file operation
                        if (file->ufs->conf->api->UnlockMutex && file->ufs->conf->api->mutex)
                        {
                            file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);
                        }
                        return UFS_NOT_OK;
                    }
                }

                cluster_offset_old = cluster_offset;
                file->ufs->conf->api->ReadSector(cluster_offset, data_sector, file->ufs->conf->api->u16numberByteOfSector);
            }

            // Write the data into the buffer
            data_sector[countByte % file->ufs->conf->api->u16numberByteOfSector] = data[bytes_written++];

            // Apply encoding if enabled
            if (file->EncodeEnable == UFS_ENCODE_ENABLE)
            {
                data_sector[countByte % file->ufs->conf->api->u16numberByteOfSector] ^= file->ufs->DeviceId[0] | BYTE_CODEC_DEFAULT;
            }
        }

        if (sumEnable == CHECKSUM_ENABLE)
        {
            sumSector = ufs_CheckSum(data_sector, file->ufs->conf->api->u16numberByteOfSector);  // Calculate final checksum for the sector
        }

        // Write back the sector after appending data
        file->ufs->conf->api->WriteSector(cluster_offset, data_sector, file->ufs->conf->api->u16numberByteOfSector);

        // Verify the written data with checksum
        if (sumEnable == CHECKSUM_ENABLE)
        {
            file->ufs->conf->api->ReadSector(cluster_offset, data_sector, file->ufs->conf->api->u16numberByteOfSector);
            if (sumSector != ufs_CheckSum(data_sector, file->ufs->conf->api->u16numberByteOfSector))
            {
                file->info.comp.size += bytes_written;
                ufs_CleanClusters(file->ufs, &file->clusters.value[cluster_index], file->clusters.length - cluster_index);
                ufs_SetClusterMap(file->ufs, file->clusters.value[cluster_index], UFS_CLUSTER_BAD);
                file->err = UFS_ERROR_SUM_SECTOR_FAIL;

                // Unlock the mutex after the file operation
                if (file->ufs->conf->api->UnlockMutex && file->ufs->conf->api->mutex)
                {
                    file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);
                }
                return UFS_NOT_OK;
            }
        }

        // If all data has been written, clean up and return success
        if (bytes_written == length)
        {
            file->info.comp.size = new_size;  // Update the file size
            ufs_UpdateItemInfo(file->ufs, file);  // Update file metadata

            // Unlock the mutex after completing the file operation
            if (file->ufs->conf->api->UnlockMutex && file->ufs->conf->api->mutex)
            {
                file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);
            }

            return UFS_OK;
        }
    }

    // Write remaining data into new clusters
    for (cluster_index = (current_cluster_count - 1); cluster_index < file->clusters.length; cluster_index++)
    {
        // Check if the cluster is valid
        if (file->clusters.value[cluster_index] == UFS_CLUSTER_END || file->clusters.value[cluster_index] == UFS_CLUSTER_BAD)
        {
            ufs_CleanClusters(file->ufs, &file->clusters.value[cluster_index], new_cluster_count - current_cluster_count + 1);
            file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);  // Unlock the mutex before returning

            // Unlock the mutex after the file operation
            if (file->ufs->conf->api->UnlockMutex && file->ufs->conf->api->mutex)
            {
                file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);
            }
            return UFS_NOT_OK;
        }

        // Write data into each sector of the cluster
        for (uint16_t sector_in_cluster = 0; sector_in_cluster < file->ufs->NumberSectorOfCluster; sector_in_cluster++)
        {
            uint32_t cluster_offset = file->ufs->ClusterDataZoneFirstSector +
                                      file->clusters.value[cluster_index] * file->ufs->NumberSectorOfCluster + sector_in_cluster;

            // Write the data to the sector buffer
            for (uint16_t countByte = 0; countByte < file->ufs->conf->api->u16numberByteOfSector; countByte++)
            {
                if (bytes_written < length)
                {
                    data_sector[countByte] = data[bytes_written++];  // Write data to the buffer

                    // Apply encoding if enabled
                    if (file->EncodeEnable == UFS_ENCODE_ENABLE)
                    {
                        data_sector[countByte] ^= file->ufs->DeviceId[0] | BYTE_CODEC_DEFAULT;
                    }
                }
                else
                {
                	data_sector[countByte] = UFS_BYTE_VALUE_AFTER_ERASE;
                }
            }

            // If checksum is enabled, calculate and verify it
            if (sumEnable == CHECKSUM_ENABLE)
            {
                sumSector = ufs_CheckSum(data_sector, file->ufs->conf->api->u16numberByteOfSector);
            }

            // Write the buffer back to the current sector
            file->ufs->conf->api->WriteSector(cluster_offset, data_sector, file->ufs->conf->api->u16numberByteOfSector);

            if (sumEnable == CHECKSUM_ENABLE)
            {
                file->ufs->conf->api->ReadSector(cluster_offset, data_sector, file->ufs->conf->api->u16numberByteOfSector);
                if (sumSector != ufs_CheckSum(data_sector, file->ufs->conf->api->u16numberByteOfSector))
                {
                    file->info.comp.size += bytes_written;  // Update file size to account for failure
                    ufs_CleanClusters(file->ufs, &file->clusters.value[cluster_index], file->clusters.length - cluster_index);  // Clean bad clusters
                    ufs_SetClusterMap(file->ufs, file->clusters.value[cluster_index], UFS_CLUSTER_BAD);  // Mark the cluster as bad
                    file->err = UFS_ERROR_SUM_SECTOR_FAIL;  // Set error for checksum failure

                    if (file->ufs->conf->api->UnlockMutex && file->ufs->conf->api->mutex)
                    {
                        file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);
                    }
                    return UFS_NOT_OK;
                }
            }

            // Stop writing if all data is written
            if (bytes_written >= length)
            {
                break;
            }
        }

        // Stop if all data is written
        if (bytes_written >= length)
        {
            break;
        }
    }

    // Update the file's size and metadata after writing all data
    file->info.comp.size = new_size;
    ufs_UpdateItemInfo(file->ufs, file);

    // Unlock the mutex after the file operation
    if (file->ufs->conf->api->UnlockMutex && file->ufs->conf->api->mutex)
    {
        file->ufs->conf->api->UnlockMutex((void *)file->ufs->conf->api->mutex);
    }

    return UFS_OK;
}

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
ufs_ReturnType ufs_RenameItem(ufs_Item_Type *item, uint8_t *strName)
{
    // Check if the UFS structure is valid
	if(item->ufs == NULL || item->err != UFS_ERROR_NONE)
    {
        return UFS_NOT_OK;
    }

    ufs_Name_Type nameChecker;  // Temporary storage for the parsed name

    // Allocate memory for sector-level buffer to read item zone data
    uint8_t data_sector[item->ufs->conf->api->u16numberByteOfSector];

    // Parse the new name to check if it is valid
    ufs_ParseNameFile(strName, &nameChecker);

    // Iterate through the item zone to find if the new name already exists
    for (uint16_t countSector = 0; countSector < (item->ufs->ClusterMappingZoneFirstSector - item->ufs->ItemZoneFirstSector); countSector++)
    {
        // Read the sector from the item zone
        item->ufs->conf->api->ReadSector(item->ufs->ItemZoneFirstSector + countSector, data_sector, item->ufs->conf->api->u16numberByteOfSector);
        for(uint16_t countByte = 0; countByte < item->ufs->conf->api->u16numberByteOfSector; countByte ++)
        {
        	if(data_sector[countByte] != 0x00)
        	{
        		data_sector[countByte] ^= BYTE_CODEC_DEFAULT;
        	}
        }
        // Iterate through the segments in the sector
        for (uint16_t countSegment = 0; countSegment < (item->ufs->conf->api->u16numberByteOfSector / sizeof(ufs_ItemInfo_Type)); countSegment++)
        {
            // Check if the file name and extension match any existing file in the segment
            if (
                UFS_OK == ufs_BytesCmp(nameChecker.head, &data_sector[countSegment * sizeof(ufs_ItemInfo_Type)], nameChecker.length) &&
                UFS_OK == ufs_BytesCmp(nameChecker.extention, &data_sector[countSegment * sizeof(ufs_ItemInfo_Type) + MAX_NAME_LENGTH], 3) &&
                nameChecker.length == data_sector[countSegment * sizeof(ufs_ItemInfo_Type) + MAX_NAME_LENGTH + 3]
            )
            {
                // File with the same name exists, return error
                item->err = UFS_ERROR_EXISTED;
                return UFS_NOT_OK;
            }
        }
    }

    // If no existing file with the same name, update the item's name
    ufs_ParseNameFile(strName, &item->info.comp.name);
    ufs_UpdateItemInfo(item->ufs, item);  // Update item information in the system

    // Return success
    return UFS_OK;
}

#if UFS_SUPPORT_FOLDER_MANAGER == UFS_OK

/**
 * @brief      Adds a new part to the end of the linked list.
 *
 * @param[in]  head_ref   Reference to the head of the linked list.
 * @param[in]  part       Part of the path to add to the list.
 */
void ufs_AddPathPart(ufs_PathNode **head_ref, const char *part)
{
    ufs_PathNode *new_node = (ufs_PathNode *)malloc(sizeof(ufs_PathNode));
    if (new_node == NULL)
    {
    	return;
    }

    strncpy((char *)new_node->part, part, strlen(part));
    new_node->part[strlen(part)] = '\0'; // Null-terminate the string
    new_node->next = NULL;

    // If list is empty, new node becomes the head
    if (*head_ref == NULL)
    {
        *head_ref = new_node;
    }
    else
    {
        // Traverse to the last node and add the new node at the end
        ufs_PathNode *current = *head_ref;
        while (current->next != NULL)
        {
            current = current->next;
        }
        current->next = new_node;
    }
}

/**
 * @brief      Frees the linked list from memory.
 *
 * @param[in]  head   Pointer to the head of the linked list.
 */
void ufs_FreePathList(ufs_PathNode *head)
{
    ufs_PathNode *temp;
    while (head != NULL)
    {
        temp = head;
        head = head->next;
        free(temp);
    }
}

/**
 * @brief Normalize a path by replacing multiple slashes '//' with a single slash '/'.
 *
 * This function takes an input path and removes redundant slashes, ensuring the path
 * contains only single slashes between directories.
 *
 * @param[in,out] path   The path to be normalized. The normalized result is stored back in the same variable.
 */
void ufs_NormalizePath(uint8_t *path)
{
    uint8_t *src = path;       // Pointer to traverse the original path
    uint8_t *dest = path;      // Pointer to construct the normalized path

    while (*src != '\0')
    {
        *dest = *src;

        // Skip over multiple slashes
        if (*src == '/')
        {
            while (*(src + 1) == '/')
            {
                src++;
            }
        }

        src++;
        dest++;
    }

    // Null-terminate the result
    *dest = '\0';
}

/**
 * @brief      Splits a path into separate directory components and stores them in a linked list.
 *
 * This function parses a path and separates it into individual directories.
 * Each directory component is stored in a linked list.
 *
 * @param[in]  path     The path string to be parsed (e.g., "/user/chungnt").
 * @param[out] head     Pointer to the head of the linked list containing path parts.
 */
ufs_ReturnType ufs_ParsePath(const uint8_t *path, ufs_PathNode **head, int max_parts)
{
    // Create a copy of the path to prevent modifying the original string
    char path_copy[0xFF];
    strncpy(path_copy, (char *)path, sizeof(path_copy));
    path_copy[sizeof(path_copy) - 1] = '\0'; // Ensure null-termination

    // Tokenize the path based on '/'
    char *token = strtok(path_copy, "/");
    int part_count = 0;

    while (token != NULL)
    {
        // Check if part count exceeds the max limit
        if (part_count >= max_parts)
        {
            // Free any previously allocated nodes
            ufs_FreePathList(*head);
            return UFS_NOT_OK;
        }

        // Add each part to the linked list
        ufs_AddPathPart(head, token);
        part_count++;
        token = strtok(NULL, "/");
    }

    return UFS_OK;
}

/**
 * @brief   Finds an available slot in the storage area for a new item.
 *
 * This function scans through the item storage area and returns the first available
 * `slotID` for storing a new item.
 *
 * @param[in]  ufs     Pointer to the UFS structure.
 * @param[out] slotID  Pointer to store the location of the available slot (sector_id and position).
 *
 * @return  ufs_ReturnType   UFS_OK if an available slot is found, UFS_NOT_OK if not.
 */
ufs_ReturnType ufs_FindFreeSlot(UFS *ufs, ufs_Location_Type *slotID)
{
    // Allocate memory to hold data for one sector
    uint8_t data_sector[ufs->conf->api->u16numberByteOfSector];

    // Calculate the total number of sectors in the item zone
    uint16_t totalSectors = ufs->ClusterMappingZoneFirstSector - ufs->ItemZoneFirstSector;
    uint16_t itemsPerSector = ufs->conf->api->u16numberByteOfSector / sizeof(ufs_ItemInfo_Type);

    // Iterate through each sector to locate a free slot
    for (uint16_t sector = 0; sector < totalSectors; sector++)
    {
        // Read data from the current sector
        ufs->conf->api->ReadSector(ufs->ItemZoneFirstSector + sector, data_sector, ufs->conf->api->u16numberByteOfSector);

        // Decode Header
        for(uint16_t countByte = 0; countByte < ufs->conf->api->u16numberByteOfSector; countByte ++)
        {
        	if(data_sector[countByte] != 0x00)
        	{
        		data_sector[countByte] ^= BYTE_CODEC_DEFAULT;
        	}
        }

        // Traverse each segment within the sector to find an available slot
        for (uint16_t segment = 0; segment < itemsPerSector; segment++)
        {
            uint8_t itemStatus = data_sector[segment * sizeof(ufs_ItemInfo_Type)];

            // Check if the current item slot is free
            if (itemStatus == UFS_ITEM_FREE)
            {
                // Save the available slot's location in `slotID`
                slotID->sector_id = sector;
                slotID->position = segment;

                return UFS_OK;
            }
        }
    }

    return UFS_NOT_OK; // No available slot found
}

/**
 * @brief Cleans the directory name by removing special characters and returns a standardized name.
 *
 * This function takes an input directory name, removes any special characters like '@', '/', '.',
 * and stores the cleaned name in the output buffer.
 *
 * @param[in]  directory       The input directory name to be cleaned.
 * @param[out] clean_directory The buffer to store the cleaned directory name.
 * @param[in]  max_length      The maximum length of the cleaned directory name buffer.
 *
 * @return     ufs_ReturnType  UFS_OK if successful, UFS_NOT_OK if an error occurs.
 */
ufs_ReturnType ufs_GetCleanDirectoryName(const uint8_t *directory, uint8_t *clean_directory, size_t max_length)
{
    if (directory == NULL || clean_directory == NULL || max_length == 0)
    {
        return UFS_NOT_OK; // Return error if inputs are invalid
    }

    size_t dest_index = 0;
    for (size_t src_index = 0; directory[src_index] != '\0' && dest_index < max_length - 1; src_index++)
    {
        // Only copy alphanumeric characters
        if ((directory[src_index] >= 'A' && directory[src_index] <= 'Z') ||
            (directory[src_index] >= 'a' && directory[src_index] <= 'z') ||
            (directory[src_index] >= '0' && directory[src_index] <= '9'))
        {
            clean_directory[dest_index++] = directory[src_index];
        }
    }

    // Null-terminate the cleaned directory name
    clean_directory[dest_index] = '\0';

    return UFS_OK;
}

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
ufs_ReturnType ufs_Mount(UFS *ufs, const uint8_t *path)
{
    ufs_PathNode *path_list = NULL;
    ufs_Path_Type path_backup;

    // Parse the path into individual parts, stored as a linked list in `path_list`
    if(UFS_NOT_OK == ufs_ParsePath((const uint8_t *)path, &path_list, MAX_PATH_PARTS))
    {
    	ufs_FreePathList(path_list);
    	return UFS_NOT_OK;
    }

    // Lock mutex for thread safety
    if (ufs->conf->api->LockMutex && ufs->conf->api->mutex)
    {
        ufs->conf->api->LockMutex((void *)ufs->conf->api->mutex);  // Lock the mutex
    }

    ufs_PathNode *current = path_list;
    ufs_Item_Type item;
    ufs_ReturnType result;

    path_backup.id = ufs->path.id;
    path_backup.name = (uint8_t *)ufs->path.name;

    ufs->path.id = 0;
    ufs->path.name = (uint8_t *)"/";

    memset((uint8_t *)&item, 0x00, sizeof(ufs_Item_Type));
    // Traverse each part of the path
    while (current != NULL)
    {
        // Check if the current directory exists in the UFS
        result = ufs_CheckExistence(ufs, (uint8_t *)current->part, &item);

        if (result != UFS_OK)
        {
            // If the directory does not exist, find an available slotID for the new directory
            ufs_Location_Type slotID;
            result = ufs_FindFreeSlot(ufs, &slotID);  // Assuming `ufs_FindFreeSlot` is implemented

            if (result != UFS_OK)
            {
            	ufs->path.id = path_backup.id;
            	ufs->path.name = path_backup.name;
                ufs_FreePathList(path_list);  // Free path list to prevent memory leaks

                // Unlock the mutex after the file operation
                if (ufs->conf->api->UnlockMutex && ufs->conf->api->mutex)
                {
                    ufs->conf->api->UnlockMutex((void *)ufs->conf->api->mutex);
                }

                return UFS_NOT_OK;  // Return error if no slot is available
            }

            // Initialize new directory with `slotID` and set necessary attributes
            item.location = slotID;
            item.info.comp.size = 0;
            item.info.comp.parent = ufs->path.id;  // Assign the parent ID to the new directory
            item.err    = UFS_ERROR_NONE;
            item.status = UFS_FOLDER_EXIST;

            // Copy the directory name to the item structure
            strncpy((char *)item.info.comp.name.head, (char *)current->part, MAX_NAME_LENGTH);

            // Update the UFS with the new directory information
            result = ufs_UpdateItemInfo(ufs, &item);
            if (result != UFS_OK)
            {
            	ufs->path.id = path_backup.id;
            	ufs->path.name = path_backup.name;
                ufs_FreePathList(path_list);  // Free path list on failure

                // Unlock the mutex after the file operation
                if (ufs->conf->api->UnlockMutex && ufs->conf->api->mutex)
                {
                    ufs->conf->api->UnlockMutex((void *)ufs->conf->api->mutex);
                }
                return UFS_NOT_OK;
            }
        }

        // Update the current path ID in UFS to navigate through subdirectories
        ufs->path.id = (item.location.sector_id * (ufs->conf->api->u16numberByteOfSector / sizeof(ufs_ItemInfo_Type))) + item.location.position;

        // Move to the next directory in the path
        current = current->next;
    }

    // Free the path list and update the UFS path name
    ufs_FreePathList(path_list);
    ufs->path.name = (uint8_t *)path;

    // Unlock the mutex after the file operation
    if (ufs->conf->api->UnlockMutex && ufs->conf->api->mutex)
    {
        ufs->conf->api->UnlockMutex((void *)ufs->conf->api->mutex);
    }
    return UFS_OK;
}

/**
 * @brief Deletes a folder and all its contents, including the folder itself.
 *
 * This function deletes the target directory and all sub-items, including subfolders and files,
 * using a stack-based approach to optimize the speed of deletion.
 *
 * @param[in] ufs         Pointer to the UFS object.
 * @param[in] directory   Path of the directory to delete.
 *
 * @return ufs_ReturnType UFS_OK if deletion is successful, UFS_NOT_OK if deletion fails.
 */
/**
 * @brief Recursively deletes a folder and all its contents.
 *
 * This function deletes the target directory and all sub-items, including subfolders and files,
 * using recursion to ensure all contents are removed before deleting the directory itself.
 *
 * @param[in] ufs         Pointer to the UFS object.
 * @param[in] directory   Path of the directory to delete.
 *
 * @return ufs_ReturnType UFS_OK if deletion is successful, UFS_NOT_OK if deletion fails.
 */
/**
 * @brief Recursively deletes a folder and all its contents.
 *
 * This function deletes the target directory and all sub-items, including subfolders and files.
 * After all contents are deleted, the directory itself is also removed.
 *
 * @param[in] ufs         Pointer to the UFS object.
 * @param[in] directory   Path of the directory to delete.
 *
 * @return ufs_ReturnType UFS_OK if deletion is successful, UFS_NOT_OK if deletion fails.
 */
ufs_ReturnType ufs_DeleteFolder(UFS *ufs, uint8_t *directory)
{
    ufs_Item_Type item;
    ufs_ReturnType result;

    /* Backup the current path information */
    ufs_Path_Type path_backup = ufs->path;

    // Lock mutex for thread safety
    if (ufs->conf->api->LockMutex && ufs->conf->api->mutex)
    {
        ufs->conf->api->LockMutex((void *)ufs->conf->api->mutex);  // Lock the mutex
    }

    /* Normalize the directory path to ensure it's valid */
    uint8_t full_path[MAX_PATH_LENGTH];
    snprintf((char *)full_path, sizeof(full_path), "%s/%s", ufs->path.name, directory);
    ufs_NormalizePath(full_path);

    /* Mount to the target directory */
    result = ufs_Mount(ufs, full_path);
    if (result != UFS_OK)
    {
        // Unlock the mutex after the file operation
        if (ufs->conf->api->UnlockMutex && ufs->conf->api->mutex)
        {
            ufs->conf->api->UnlockMutex((void *)ufs->conf->api->mutex);
        }
        /* Return error if mounting fails */
        return UFS_NOT_OK;
    }

    /* Count the number of items in the directory */
    uint16_t item_count = ufs_CountItem(ufs);
    if (item_count > 0)
    {
        /* Allocate memory to store the list of directory items */
        ufs_ItemInfo_Type *items_info = malloc(item_count * sizeof(ufs_ItemInfo_Type));
        if (items_info == NULL)
        {
            /* Restore the original path before returning */
            ufs->path = path_backup;

            // Unlock the mutex after the file operation
            if (ufs->conf->api->UnlockMutex && ufs->conf->api->mutex)
            {
                ufs->conf->api->UnlockMutex((void *)ufs->conf->api->mutex);
            }
            return UFS_NOT_OK;
        }

        /* Retrieve the list of items in the directory */
        ufs_GetListItem(ufs, items_info, item_count);

        /* Recursively delete each item */
        for (uint16_t i = 0; i < item_count; i++)
        {
            /* Check if the item is a subdirectory (no extension) */
            if (items_info[i].comp.name.extention[0] == 0x00)
            {
                /* Recursively delete the subdirectory */
                result = ufs_DeleteFolder(ufs, items_info[i].comp.name.head);
                if (result != UFS_OK)
                {
                    /* Free allocated memory and restore path on error */
                    free(items_info);
                    ufs->path = path_backup;

                    // Unlock the mutex after the file operation
                    if (ufs->conf->api->UnlockMutex && ufs->conf->api->mutex)
                    {
                        ufs->conf->api->UnlockMutex((void *)ufs->conf->api->mutex);
                    }
                    return UFS_NOT_OK;
                }
            }
            else /* Item is a file */
            {
                /* Build the file name with extension */
                uint8_t file_name[MAX_NAME_LENGTH + 4];
                snprintf((char *)file_name, sizeof(file_name), "%s.%s", items_info[i].comp.name.head, items_info[i].comp.name.extention);
                file_name[items_info[i].comp.name.length + 4] = '\0';

                /* Open and delete the file */
                result = ufs_CheckExistence(ufs, file_name, &item);
                if (result == UFS_OK)
                {
                    ufs_DeleteItem(&item);
                }
            }
        }
        /* Free allocated memory after processing items */
        free(items_info);
    }

    /* Restore the original path after deletion */
    ufs->path = path_backup;
    uint8_t dir_clean[MAX_NAME_LENGTH];
    ufs_GetCleanDirectoryName((const uint8_t *)directory, dir_clean, MAX_NAME_LENGTH);

    /* Delete the target directory itself */
    result = ufs_CheckExistence(ufs, dir_clean, &item);
    if (result == UFS_OK)
    {
        ufs_DeleteItem(&item);
    }

    // Unlock the mutex after the file operation
    if (ufs->conf->api->UnlockMutex && ufs->conf->api->mutex)
    {
        ufs->conf->api->UnlockMutex((void *)ufs->conf->api->mutex);
    }
    return UFS_OK;
}

#endif

