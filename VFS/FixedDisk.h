#pragma once

#include "VirtualDisk.h"

class FixedDisk : public VirtualDisk {
public:

    FixedDisk();

    void create(
        const std::wstring& virtualDiskPath,
        ULONGLONG           diskSize,
        DWORD               blockSize,
        DWORD               logicalSectorSize,
        DWORD               physicalSectorSize);

    ~FixedDisk();
};