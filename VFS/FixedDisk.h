#pragma once

#include "VirtualDisk.h"

class FixedDisk : public VirtualDisk {
public:

    FixedDisk();

    void create(
        const std::wstring& virtualDiskPath,
        std::wstring        parentPath,
        ULONGLONG           fileSize,
        DWORD               blockSize,
        DWORD               logicalSectorSize,
        DWORD               physicalSectorSize);

    ~FixedDisk();
};