#include "DereferencingDisk.h"

DereferencingDisk::DereferencingDisk() : VirtualDisk()
{
}

void DereferencingDisk::create(const std::wstring& virtualDiskPath, const std::wstring& parentPath)
{
    VirtualDisk::create(
        virtualDiskPath,
        parentPath,
        CREATE_VIRTUAL_DISK_FLAG_NONE,
        0,
        0,
        0,
        0
    );
}


DereferencingDisk::~DereferencingDisk()
{
}