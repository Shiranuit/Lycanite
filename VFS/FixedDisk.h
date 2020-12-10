#pragma once

#include "VirtualDisk.h"

class FixedDisk : public VirtualDisk {
public:

    FixedDisk();

    ~FixedDisk();

    void create(
        const std::wstring& virtualDiskPath,
        ULONGLONG           diskSize,
        DWORD               blockSize,
        DWORD               logicalSectorSize,
        DWORD               physicalSectorSize);

    const VIRTUAL_DISK_TYPE& getType() const;
};