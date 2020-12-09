#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <functional>
#include <exception>
#include <vector>
#include <windows.h>
#include <initguid.h>
#include <virtdisk.h>
#include <rpcdce.h>
#include <rpc.h>
#include <sddl.h>
#include <Shlwapi.h>

class VirtualDisk {
public:

    VirtualDisk();

    ~VirtualDisk();

    /**
    * Get the actual disk path
    * @return disk path
    */
    const std::wstring& getDiskPath() const;

    /**
    * Get the virtual disk handle
    * @return HANDLE of disk
    */
    const HANDLE getHandle() const;

    /**
    * Check if Virtual Disk is open
    * @param virtualDiskPath: path where the disk will be created
    * @param parentPath: path of the parent disk. If the string is empty disk will not have parent
    * @param flags: disk creation flags
    * @param fileSize: actual disk size in bytes
    * @param blockSize
    * @param logicalSectorSize
    * @param physicalSectorSize
    */
    virtual void create(
        const std::wstring&             virtualDiskPath,
        const std::wstring&             parentPath,
        const CREATE_VIRTUAL_DISK_FLAG& flags,
        ULONGLONG                       fileSize,
        DWORD                           blockSize,
        DWORD                           logicalSectorSize,
        DWORD                           physicalSectorSize);

    /**
    * Open the virtual disk
    * @param diskPath: path where the disk will be opened
    * @param accessMask: access flag mask
    * @param openFlag: open flag
    */
    void open(const std::wstring& diskPath, const VIRTUAL_DISK_ACCESS_MASK& accessMask = VIRTUAL_DISK_ACCESS_NONE, const OPEN_VIRTUAL_DISK_FLAG& openFlag = OPEN_VIRTUAL_DISK_FLAG_NONE);

    /**
    * Close the handle of virtual disk if handle is opened
    * @return a boolean if the disk is open
    */
    bool close();

    /**
    * Check if Virtual Disk is open
    * @return a boolean if the disk is open
    */
    bool isOpen() const;

    void deleteUserMetaData(const GUID& uniqueId);

    /**
    * Mirroring is a form of disk backup in which anything that is written to a disk is simultaneously written to a second disk.
    * This creates fault tolerance in the critical storage systems.
    * If a physical hardware failure occurs in a disk system, the data is not lost, as the other hard disk contains an exact copy of that data.
    * @param destinationPath vhd(x) destination path (ex: c:\\mirror.vhd)
    */
    void mirror(const std::wstring& destinationPath);

    /**
    * GetOperationStatusDisk
    * @param handle vhd handle
    * @param overlapped contains information for asynchronous IO
    * @param progress contains progress information
    * @return the operation status of the specified vhd handle
    */
    DWORD getOperationStatusDisk(const HANDLE handle, OVERLAPPED& overlapped, VIRTUAL_DISK_PROGRESS& progress) const;
    
    void setUserMetaData(const PVOID &data, const GUID &uniqueId, const ULONG &nbToWrite);

    void getUserMetaData(const GUID& uniqueId, ULONG& metaDataSize, const std::shared_ptr<VOID>& data) const;
  
    /// <summary>Enumerate the available metadata items of the opened vhdx file.
    /// <para>Warning, this method is heavy due to a vector instantiation. Do not use it without thinking</para>
    /// </summary>
    std::unique_ptr<std::vector<GUID>> enumerateUserMetaData() const;

    /**
    * GetOperationStatusDisk
    * @param handle vhd handle
    * @param overlapped contains information for asynchronous IO
    * @param progress contains progress information
    * @return the operation status of the specified vhd handle
    */
    DWORD getOperationStatusDisk(const HANDLE handle, OVERLAPPED& overlapped, VIRTUAL_DISK_PROGRESS& progress) const;

    void attachDisk(bool readOnly = true);

private:
    using WaiterDiskHandler = std::function<bool(const DWORD& status, const VIRTUAL_DISK_PROGRESS& progress)>;

    /**
    * Wait a disk operation (for asynchronous tasks)
    * @param handle virtual disk handle
    * @param overlapped contains information for asynchronous IO
    * @param progressHandler handler to know when the task is finished. If the handler return true task is completed else return false.
    * @param msWaits milliseconds to wait after each loop of waiting
    */
    void waitDiskOperation(
        const HANDLE             handle,
        OVERLAPPED&              overlapped,
        const WaiterDiskHandler& progressHandler,
        int                      msWaits = 1000) const;

protected:
    std::wstring _diskPath;
    std::wstring _parentPath;
    HANDLE _handle;
};