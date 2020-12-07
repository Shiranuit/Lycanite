#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <functional>
#include <exception>
#include <windows.h>
#include <initguid.h>
#include <virtdisk.h>
#include <rpcdce.h>
#include <rpc.h>

class VirtualDisk {
public:

    enum class VIRTUAL_DISK_TYPE {
        DEFAULT,
        DYNAMIC,
        FIXED,
        DEREFERENCING
    };

    VirtualDisk(VIRTUAL_DISK_TYPE type, bool resizable);

    VirtualDisk(bool resizable);

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
    * @param fileSize: actual disk size in bytes
    * @param blockSize
    * @param logicalSectorSize
    * @param physicalSectorSize
    * @param resizable know if disk can be resizable
    */
    virtual void create(
        const std::wstring&             virtualDiskPath,
        const std::wstring&             parentPath,
        const CREATE_VIRTUAL_DISK_FLAG& flags,
        ULONGLONG                       fileSize,
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
    * @param newFileSize new size of the virtual disk
    * @return true is resized else false
    */
    virtual bool resize(ULONGLONG newFileSize);

    /**
    * Check if virtual disk is resizable
    * @return true is resizable else false
    */
    virtual bool isResizable() const;

    /**
    * Get type of the virtual disk
    * @return type
    */
    virtual const VIRTUAL_DISK_TYPE& getType() const;

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
    VIRTUAL_DISK_TYPE _type;
    bool _resizable;
};