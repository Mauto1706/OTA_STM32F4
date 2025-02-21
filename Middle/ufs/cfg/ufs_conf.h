#ifndef _UFS_CONF_H_
#define _UFS_CONF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "ufs_types.h"

#define MAX_PATH_PARTS      5       // Maximum number of parts in the path

#define UFS_SUPPORT_FOLDER_MANAGER    UFS_OK

#define __fast __attribute__((optimize("Ofast")))

/**
 * @brief Value used to indicate the state of a byte after erasure.
 *        Typically, this is the value of flash memory cells after an erase operation.
 */
#define UFS_BYTE_VALUE_AFTER_ERASE     0xFF

/**
 * @brief Number of supported encoded file extensions.
 *        This defines how many different file extensions can be handled by the UFS system.
 */
#define UFS_NUMB_OF_ENCODE_EXTENSION   3

/**
 * @brief UFS configuration structure.
 *        This structure contains all configuration settings and API mappings for UFS.
 *        It is defined externally in the system configuration.
 */
extern ufs_Cfg_Type Ufs_Cfg;

#ifdef __cplusplus
}
#endif

#endif /* _UFS_CONF_H_ */
