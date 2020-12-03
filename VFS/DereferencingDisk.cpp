#include "DereferencingDisk.h"

DereferencingDisk::DereferencingDisk(const std::wstring &virtualDiskPath,
                                     const std::wstring &parentPath
    ) : VirtualDisk(
        virtualDiskPath,
        parentPath,
        CREATE_VIRTUAL_DISK_FLAG_NONE,
        0, 0, 0, 0)
{
}

DereferencingDisk::~DereferencingDisk()
{
}