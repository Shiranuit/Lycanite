#include "DynamicDisk.h"

DynamicDisk::DynamicDisk() : VirtualDisk()
{
}

void DynamicDisk::create(
    const std::wstring& virtualDiskPath,
    std::wstring        parentPath,
    ULONGLONG           fileSize,
    DWORD               blockSize,
    DWORD               logicalSectorSize,
    DWORD               physicalSectorSize)
{
    VirtualDisk::create(
        virtualDiskPath,
        parentPath,
        CREATE_VIRTUAL_DISK_FLAG_NONE,
        fileSize,
        blockSize,
        logicalSectorSize,
        physicalSectorSize
    );
}

DynamicDisk::~DynamicDisk()
{
}