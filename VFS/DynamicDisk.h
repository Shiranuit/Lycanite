#pragma once

#include "VirtualDisk.h"

class DynamicDisk : public VirtualDisk {
public:

    DynamicDisk();

    ~DynamicDisk();

    void create(
        const std::wstring& virtualDiskPath,
        ULONGLONG           diskSize,
        DWORD               blockSize,
        DWORD               logicalSectorSize,
        DWORD               physicalSectorSize);

    const VIRTUAL_DISK_TYPE& getType() const;
};

