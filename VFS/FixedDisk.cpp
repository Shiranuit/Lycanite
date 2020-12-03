#include "FixedDisk.h"

FixedDisk::FixedDisk(
    const std::wstring &virtualDiskPath,
    const std::wstring &parentPath,
    ULONGLONG          fileSize,
    DWORD              blockSize,
    DWORD              logicalSectorSize,
    DWORD              physicalSectorSize
    ) : VirtualDisk(virtualDiskPath,
                    parentPath,
                    CREATE_VIRTUAL_DISK_FLAG_FULL_PHYSICAL_ALLOCATION,
                    fileSize,
                    blockSize,
                    logicalSectorSize,
                    physicalSectorSize)
{
}

FixedDisk::~FixedDisk()
{
}