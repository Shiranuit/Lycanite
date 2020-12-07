#include "FixedDisk.h"

FixedDisk::FixedDisk() : VirtualDisk()
{
}

void FixedDisk::create(
    const std::wstring& virtualDiskPath,
    ULONGLONG           fileSize,
    DWORD               blockSize,
    DWORD               logicalSectorSize,
    DWORD               physicalSectorSize)
{
    VirtualDisk::create(
        virtualDiskPath,
        L"",
        CREATE_VIRTUAL_DISK_FLAG_FULL_PHYSICAL_ALLOCATION,
        fileSize,
        blockSize,
        logicalSectorSize,
        physicalSectorSize
    );
}

FixedDisk::~FixedDisk()
{
}