#include "DynamicDisk.h"

DynamicDisk::DynamicDisk(
    const std::wstring &virtualDiskPath,
    const std::wstring &parentPath,
    ULONGLONG          fileSize,
    DWORD              blockSize,
    DWORD              logicalSectorSize,
    DWORD              physicalSectorSize
    ) : VirtualDisk(virtualDiskPath,
                    parentPath,
                    CREATE_VIRTUAL_DISK_FLAG_NONE,
                    fileSize,
                    blockSize,
                    logicalSectorSize,
                    physicalSectorSize)
{
}

DynamicDisk::~DynamicDisk()
{
}