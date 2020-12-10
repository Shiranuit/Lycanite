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

class VirtualDisk {
public:

    enum class VIRTUAL_DISK_TYPE {
        DYNAMIC,
        FIXED,
        DEREFERENCING
    };

    explicit VirtualDisk();

    /**
    * Destructor of VirtualDisk
    */
    ~VirtualDisk();

    /**
    * Get the actual disk path
    * @return disk path
    */
    virtual const std::wstring& getDiskPath() const;

    /**
    * Get the virtual disk handle
    * @return HANDLE of disk
    */
    virtual const HANDLE getHandle() const;

    /**
    * Check if Virtual Disk is open
    * @param virtualDiskPath: path where the disk will be created
    * @param parentPath: path of the parent disk. If the string is empty disk will not have parent
    * @param flags: disk creation flags
    * @param diskSize: actual disk size in bytes, must be multiple of 512
    * @param blockSize
    * @param logicalSectorSize
    * @param physicalSectorSize
    * @param resizable know if disk can be resizable
    */
    virtual void create(
        const std::wstring&             virtualDiskPath,
        const std::wstring&             parentPath,
        const CREATE_VIRTUAL_DISK_FLAG& flags,
        ULONGLONG                       diskSize,
        DWORD                           blockSize,
        DWORD                           logicalSectorSize,
        DWORD                           physicalSectorSize
    );

    /**
    * Open the virtual disk
    * @param diskPath: path where the disk will be opened
    * @param accessMask: access flag mask
    * @param openFlag: open flag
    */
    virtual void open(const std::wstring& diskPath, const VIRTUAL_DISK_ACCESS_MASK& accessMask = VIRTUAL_DISK_ACCESS_NONE, const OPEN_VIRTUAL_DISK_FLAG& openFlag = OPEN_VIRTUAL_DISK_FLAG_NONE);

    /**
    * Close the handle of virtual disk if handle is opened
    * @return a boolean if the disk is open
    */
    virtual bool close();

    /**
    * Check if Virtual Disk is open
    * @return a boolean if the disk is open
    */
    virtual bool isOpen() const;

    /**
    * GetDiskInfo
    * @return the struction containing the information about the disk
    */
    const GET_VIRTUAL_DISK_INFO &getDiskInfo(GET_VIRTUAL_DISK_INFO_VERSION flag);

    /**
    * SetDiskInfo
    * @param diskInfo structure with all the information to be changed
    */ 
    void setDiskInfo(SET_VIRTUAL_DISK_INFO &diskInfo);

    void deleteUserMetaData(const GUID& uniqueId);

    /**
    * Mirroring is a form of disk backup in which anything that is written to a disk is simultaneously written to a second disk.
    * This creates fault tolerance in the critical storage systems.
    * If a physical hardware failure occurs in a disk system, the data is not lost, as the other hard disk contains an exact copy of that data.
    * @param destinationPath vhd(x) destination path (ex: c:\\mirror.vhd)
    */
    virtual void mirror(const std::wstring& destinationPath);

    /**
    * Get operation status of the virtual disk
    * @param handle vhd handle
    * @param overlapped contains information for asynchronous IO
    * @param progress contains progress information
    * @return the operation status of the specified vhd handle
    */
    virtual DWORD getOperationStatusDisk(const HANDLE handle, OVERLAPPED& overlapped, VIRTUAL_DISK_PROGRESS& progress) const;

    /**
    * Resize the virtual disk
    * @param newDiskSize new size of the virtual disk in bytes, must be multiple of 512
    * @return true is resized else false
    */
    virtual bool resize(ULONGLONG newDiskSize);

    /**
    * Check if virtual disk is resizable
    * @return true is resizable else false
    */
    virtual bool isResizable() const = 0;

    /**
    * Get type of the virtual disk
    * @return type
    */
    virtual const VIRTUAL_DISK_TYPE getType() const = 0;

    void setUserMetaData(const PVOID &data, const GUID &uniqueId, const ULONG &nbToWrite);

    void getUserMetaData(const GUID& uniqueId, ULONG& metaDataSize, const std::shared_ptr<VOID>& data) const;

    /// <summary>Enumerate the available metadata items of the opened vhdx file.
    /// <para>Warning, this method is heavy due to a vector instantiation. Do not use it without thinking</para>
    /// </summary>
    std::unique_ptr<std::vector<GUID>> enumerateUserMetaData() const;

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
    GET_VIRTUAL_DISK_INFO _diskInfo;
    HANDLE _handle;
};