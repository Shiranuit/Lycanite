#pragma once

#include <iostream>
#include <new>
#include <exception>
#include <windows.h>
#include <initguid.h>
#include <virtdisk.h>
#include <rpcdce.h>
#include <rpc.h>

class VirtualDisk {
    public:

    VirtualDisk(
        const std::wstring             &virtualDiskPath,
        const std::wstring             &parentPath,
        const CREATE_VIRTUAL_DISK_FLAG &flags,
        ULONGLONG                      fileSize,
        DWORD                          blockSize,
        DWORD                          logicalSectorSize,
        DWORD                          physicalSectorSize
    );

    ~VirtualDisk();

    const std::wstring &getDiskPath() const;

    const CREATE_VIRTUAL_DISK_PARAMETERS &getParameters() const;

    const VIRTUAL_STORAGE_TYPE &getStorageType() const;

    const HANDLE getHandle() const;

    void create();

    protected:
        std::wstring _diskPath;
        std::wstring _parentPath;
        CREATE_VIRTUAL_DISK_FLAG _flags;
        CREATE_VIRTUAL_DISK_PARAMETERS _parameters;
        VIRTUAL_STORAGE_TYPE _storageType;
        HANDLE _handle;
};