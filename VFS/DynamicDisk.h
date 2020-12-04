#pragma once

#include "VirtualDisk.h"

class DynamicDisk : public VirtualDisk {
public:

    DynamicDisk();

    void create(
        const std::wstring& virtualDiskPath,
        const std::wstring& parentPath,
        ULONGLONG           fileSize,
        DWORD               blockSize,
        DWORD               logicalSectorSize,
        DWORD               physicalSectorSize);

    ~DynamicDisk();
};

