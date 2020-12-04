#pragma once

#include <iostream>
#include <string>
#include <new>
#include <exception>
#include <windows.h>
#include <initguid.h>
#include <virtdisk.h>
#include <rpcdce.h>
#include <rpc.h>

class VirtualDisk {
public:

    VirtualDisk();

    ~VirtualDisk();

    const std::wstring& getDiskPath() const;

    const HANDLE getHandle() const;

    virtual void create(
        const std::wstring&             virtualDiskPath,
        const std::wstring&             parentPath,
        const CREATE_VIRTUAL_DISK_FLAG& flags,
        ULONGLONG                       fileSize,
        DWORD                           blockSize,
        DWORD                           logicalSectorSize,
        DWORD                           physicalSectorSize);

    void open(const std::wstring& diskPath, const VIRTUAL_DISK_ACCESS_MASK& access_mask = VIRTUAL_DISK_ACCESS_NONE, const OPEN_VIRTUAL_DISK_FLAG& open_flag = OPEN_VIRTUAL_DISK_FLAG_NONE);

    bool close();

    bool isOpen() const;

    void setUserMetaData(const PVOID &data, const GUID &uniqueId, const ULONG &nbToWrite);

    std::shared_ptr<VOID> getUserMetaData(const GUID& uniqueId, ULONG &metaDataSize) const;

protected:
    std::wstring _diskPath;
    std::wstring _parentPath;
    HANDLE _handle;
};