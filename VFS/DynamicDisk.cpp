#include "DynamicDisk.h"

DynamicDisk::DynamicDisk() : VirtualDisk()
{
}

void DynamicDisk::create(
    const std::wstring& virtualDiskPath,
    ULONGLONG           fileSize,
    DWORD               blockSize,
    DWORD               logicalSectorSize,
    DWORD               physicalSectorSize)
{
    VirtualDisk::create(
        virtualDiskPath,
        L"",
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