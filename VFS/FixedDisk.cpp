#include "FixedDisk.h"

FixedDisk::FixedDisk() : VirtualDisk(false)
{
}

FixedDisk::~FixedDisk()
{
}

void FixedDisk::create(
    const std::wstring& virtualDiskPath,
    ULONGLONG           diskSize,
    DWORD               blockSize,
    DWORD               logicalSectorSize,
    DWORD               physicalSectorSize)
{
    VirtualDisk::create(
        virtualDiskPath,
        L"",
        CREATE_VIRTUAL_DISK_FLAG_FULL_PHYSICAL_ALLOCATION,
        diskSize,
        blockSize,
        logicalSectorSize,
        physicalSectorSize
    );
}

const VirtualDisk::VIRTUAL_DISK_TYPE& FixedDisk::getType() const
{
    return (VIRTUAL_DISK_TYPE::FIXED);
}