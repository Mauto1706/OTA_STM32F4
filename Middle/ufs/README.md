## UFS Library

UFS (Universal File System) is a lightweight file system designed for embedded systems. It provides essential functionalities for managing files and directories on a flash memory-like storage medium with minimal overhead. UFS ensures safe file operations in multi-threaded environments through the use of mutexes.

### Features

- **Basic file operations**: support for reading, writing, appending, and deleting files.
- **Folder management**: Allows mounting to specific paths and deleting entire folders with all their contents, including subfolders and files.
- **Multi-threading support**: mutex locks ensure thread safety.
- **Wear leveling mechanism**: improves the longevity of Flash memory by evenly distributing write/erase cycles across the memory.
- **Configurable sector size and device storage**: easily adapt to different hardware configurations.
- **File extension management**: supports specific file extensions for encoding.
- **Efficient cluster management**: optimizes memory usage for embedded systems.
- **Encoding/decoding special files**: supports encoding and decoding special files before writing to and after reading from memory, enhancing security.
- **Checksum validation and bad sector management**: provides an option for checksum verification and bad sector tracking to improve reliability.
- **Item existence check**: provides dedicated functionality to check if a file or folder exists without modifying or creating items.

### Wear Leveling Mechanism

Wear leveling is an important feature when working with Flash memory. It helps distribute the wear evenly across memory blocks to avoid overuse of specific areas. This ensures that no single block is erased or written too frequently, thereby increasing the overall lifespan of the Flash memory.

The UFS library implements wear leveling by:

- **Dynamic allocation of clusters**: clusters are allocated based on availability, avoiding repeated use of specific sectors.
- **Re-ordering clusters**: the system re-orders and reassigns clusters when files are modified, ensuring that unused areas of the Flash memory are utilized efficiently.
- **Erasure spreading**: by managing sectors and clusters in a balanced manner, the library ensures that write and erase cycles are spread evenly across the entire memory.

### Library Structure

#### Core Structures

- `UFS`: Represents the UFS system, containing configuration and state information.
- `ufs_Item_Type`: Represents a file item in UFS, including its metadata, cluster information, and status.
- `ufs_Api_Type`: Holds function pointers to the API functions used for managing memory operations and mutex locking/unlocking.

#### Key Functions

- **Initialization and Formatting:**
  - `UFS *newUFS(ufs_Cfg_Type *pUfsCfg)`: Initializes a new UFS instance.
  - `ufs_ReturnType ufs_FastFormat(UFS *ufs)`: Performs a fast format of the UFS device.

- **File Management:**
  - `ufs_ReturnType ufs_OpenItem(UFS *ufs, uint8_t *name_file, ufs_Item_Type *item)`: Opens or creates a file or folder.
  - `ufs_ReturnType ufs_WriteFile(ufs_Item_Type *file, uint8_t *data, uint32_t length)`: Writes data to a file.
  - `ufs_ReturnType ufs_WriteAppendFile(ufs_Item_Type *file, uint8_t *data, uint32_t length)`: Appends data to the end of a file.
  - `uint32_t ufs_ReadFile(ufs_Item_Type *file, uint16_t position, uint8_t *data, uint32_t length)`: Reads data from a file.
  - `ufs_ReturnType ufs_DeleteItem(ufs_Item_Type *item)`: Deletes a file from UFS.
  - `ufs_ReturnType ufs_CloseItem(ufs_Item_Type *item)`: Closes a file and releases allocated resources.

- **Folder Management:**
  - `ufs_ReturnType ufs_Mount(UFS *ufs, const uint8_t *path)`: Mounts a specified path, creating any missing directories.
  - `ufs_ReturnType ufs_DeleteFolder(UFS *ufs, uint8_t *directory)`: Deletes a specified folder and all its contents recursively. Requires mounting to the parent folder first.
  - `ufs_ReturnType ufs_CheckExistence(UFS *ufs, uint8_t *name, ufs_Item_Type *item)`: Checks if a file or folder with the specified name exists within the currently mounted folder. Does not create items, only verifies their existence.

- **Space Management:**
  - `uint32_t ufs_GetDeviceSize(UFS *ufs)`: Retrieves the total usable size of the UFS device.
  - `uint32_t ufs_GetUsedSize(UFS *ufs)`: Calculates the total used size in UFS.

### How to Use

#### 1. Configuration Setup

You need to configure the UFS with the necessary hardware-level APIs for reading, writing, erasing, and initializing the storage. Here is an example of how to set up the configuration:

```c
ufs_ExtensionName_Type ExtensionList[UFS_NUMB_OF_ENCODE_EXTENSION] = 
{
    {(uint8_t *)"usr"},  // User extension
    {(uint8_t *)"sys"},  // System extension
    {(uint8_t *)"bin"}   // Binary extension
};

ufs_Api_Type Api_Mapping = 
{
    .Init              = (ufs_Init *)MemFlash_Init,               // Initialization function
    .WriteSector       = (ufs_WriteSector *)MemFlash_WriteSector, // Write sector function
    .ReadSector        = (ufs_ReadSector *)MemFlash_ReadSector,   // Read sector function
    .EraseSector       = (ufs_EraseSector *)MemFlash_EraseSector, // Erase sector function
    .EraseChip         = (ufs_EraseChip *)MemFlash_EraseChip,     // Erase entire chip function
    .ReadUniqueID      = (ufs_ReadUniqueID *)MemFlash_ReadID,     // Read unique ID function
    .u16numberByteOfSector   = 512,                               // Number of bytes per sector
    .u32numberSectorOfDevice = 512                                // Total number of sectors in the device
};

ufs_Cfg_Type Ufs_Cfg = 
{
    .api                         = &Api_Mapping,                   // Pointer to the UFS API mappings
    .pExtensionEncodeFileList     = ExtensionList,                 // List of supported file extensions
    .u8NumberFileMaxOfDevice      = 20,                            // Maximum number of files supported by he device
    .u8NumberEncodeFileExtension  = UFS_NUMB_OF_ENCODE_EXTENSION   // Number of supported encoded file extensions
};
```
### Usage
#### Initializing the UFS
To initialize the UFS system, call the newUFS() function with the configuration structure. This function sets up the file system and prepares it for file operations.
```c
UFS *ufs;
ufs = newUFS(&Ufs_Cfg);
```
#### Mount a Folder
The **ufs_Mount** function allows you to "mount" a specified path within the UFS system. It checks each directory in the path and creates any missing directories. If the path exceeds the **MAX_PATH_PARTS** limit, the function will return an error.
```c
ufs_ReturnType result = ufs_Mount(ufs, (const uint8_t *)"/home/user/docs");
if (result != UFS_OK)
{
    // Handle error if unable to mount
}
```
#### Delete a Folder
The **ufs_DeleteFolder** function deletes an entire folder, including all subdirectories and files within it. Note that this function only accepts the name of the folder to be deleted, not the full path. To delete a folder, you must first **mount** to its parent directory, and then call **ufs_DeleteFolder**.
```c
// First, mount to the parent directory
ufs_Mount(ufs, (const uint8_t *)"/home/user");

// Then delete the "docs" folder within "/home/user"
ufs_DeleteFolder(ufs, (uint8_t *)"docs");
```
#### Checking for Item Existence
To check whether a file or folder exists within the currently mounted folder, use ufs_CheckExistence:
```c
ufs_Item_Type item;
ufs_ReturnType result = ufs_CheckExistence(ufs, (uint8_t *)"docs", &item);
if (result == UFS_OK) 
{
    if (item.status == UFS_FOLDER_EXIST) 
    {
        // It's a folder
    } else if (item.status == UFS_FILE_EXIST) 
    {
        // It's a file
    }
} 
else 
{
    // Item does not exist
}
```
#### Writing to a File
To write data to a file, use the ufs_WriteFile() function. This function writes data to a specified file and handles cluster management.
```c
ufs_Item_Type item;
ufs_OpenItem(ufs, (uint8_t *)"example.txt", &item);
ufs_WriteFile(&item, (uint8_t *)"Hello World", 11, CHECKSUM_DISABLE);
```
#### Appending Data to a File
You can append data to the end of an existing file using the ufs_WriteAppendFile() function.
```c
ufs_WriteAppendFile(&item, (uint8_t *)" Appended Data", 14, CHECKSUM_ENABLE);
```
#### Reading from a File
To read data from a file, use the ufs_ReadFile() function. You must specify the file, the position to start reading from, and the buffer to store the read data.
```c
uint8_t data_read[100] = {0};
uint32_t bytes_read = ufs_ReadFile(&item, 0, data_read, 100);
```
#### Deleting a File
To delete a file and free up its associated clusters, use the ufs_DeleteItem() function.
```c
ufs_DeleteItem(&item);
```
#### Closing a File
Once file operations are complete, use the ufs_CloseItem() function to release the resources associated with the file.
```c
ufs_CloseItem(&item);
```
#### Counting Used Files
To count the number of used files (non-free files) in the UFS, use the ufs_CountItem() function.
```c
uint16_t item_count = ufs_CountItem(ufs);
```
#### Retrieving the Used Space
To retrieve the total used space in the UFS, use the ufs_GetUsedSize() function.
```c
uint32_t used_size = ufs_GetUsedSize(ufs);
```
#### Getting Device Size
To get the total usable size of the UFS device, use the ufs_GetDeviceSize() function.
```c
uint32_t device_size = ufs_GetDeviceSize(ufs);
```
#### Error Handling
The UFS system provides error codes for various failure conditions. These error codes are defined in the ufs_ErrorCodes enum. Examples include:

- **UFS_ERROR_NONE**: No error.
- **UFS_ERROR_ALLOCATE_MEM**: Memory allocation error.
- **UFS_ERROR_FULL_MEM**: Device memory is full.
- **UFS_ERROR_API_NOT_FOUND**: API function is missing.
- **v.v...**
  
Error codes can be checked using the err field of the ufs_Item_Type structure. For example:
```c
if (item.err != UFS_ERROR_NONE)
{
    // Handle error
}
```
#### Thread Safety
The UFS system supports multi-threading environments using mutex locks to ensure thread safety. When enabled, the LockMutex and UnlockMutex functions are called before and after critical file operations. Here’s how you can configure mutexes in the UFS:
```c
void LockMutex(void *mutex);
void UnlockMutex(void *mutex);

ufs_Api_Type Api_Mapping = 
{
    // Other functions...
    .LockMutex         = LockMutex,    // Lock mutex function
    .UnlockMutex       = UnlockMutex,  // Unlock mutex function
    .mutex             = &my_mutex     // Pointer to the mutex object
};
```
#### Limitations
The maximum number of files that can be stored is determined by the **u8NumberFileMaxOfDevice** configuration.
Restricts the maximum number of parts a path can contain, as defined by **MAX_PATH_PARTS**.
```c
#define MAX_PATH_PARTS 5  // Maximum number of allowed path parts
```
The file system is optimized for small, embedded devices and may not scale well for large datasets.
