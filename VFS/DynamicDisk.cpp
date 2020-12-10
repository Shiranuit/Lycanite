#include "DynamicDisk.h"

DynamicDisk::DynamicDisk() : VirtualDisk(true)
{
}

DynamicDisk::~DynamicDisk()
{
}

void DynamicDisk::create(
    const std::wstring& virtualDiskPath,
    ULONGLONG           diskSize,
    DWORD               blockSize,
    DWORD               logicalSectorSize,
    DWORD               physicalSectorSize)
{
    VirtualDisk::create(
        virtualDiskPath,
        L"",
        CREATE_VIRTUAL_DISK_FLAG_NONE,
        diskSize,
        blockSize,
        logicalSectorSize,
        physicalSectorSize
    );
}

const VirtualDisk::VIRTUAL_DISK_TYPE& DynamicDisk::getType() const
{
    return (VIRTUAL_DISK_TYPE::DYNAMIC);
}