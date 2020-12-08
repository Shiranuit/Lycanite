#pragma once

#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <functional>
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

    /**
    * Query storage dependency information
    */
    std::unique_ptr<STORAGE_DEPENDENCY_INFO, decltype(std::free)*> getStorageDependencyInfo() const;

    /**
    * Query storage dependency information
    * @param diskLetter letter or number of virtual disk (can be 0-9 A-Z)
    * @return unique_ptr pointing to a valid STORAGE_DEPENDENCY_INFO structure
    */
    static std::unique_ptr<STORAGE_DEPENDENCY_INFO, decltype(std::free)*> getStorageDependencyInfo(WCHAR diskLetter);

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

    /**
    * Simple get storage dependency
    * Returns the relationships between virtual hard disks (VHDs) or CD or DVD image file (ISO)
    * or the volumes contained within those disks and their parent disk or volume.
    * @param pInfo: a reference from a unique_ptr (custom free because it must allocate with malloc) poiting to a
    * (maybe) non valid STORAGE_DEPENDENCY_INFO structure, the unique_ptr will be allocate in the function
    * @param infoSize: size to allocate in the unique_ptr
    * @param cbSize: a size that receives the size used.
    * @param driveHandle: a handle to a volume or root directory (opened by CreateFile)
    * @param flags: flags
    * @return unique_ptr pointing to a STORAGE_DEPENDENCY_INFO structure (can be non valid)
    */
    static DWORD simpleGetStorageDependency(
        std::unique_ptr<STORAGE_DEPENDENCY_INFO, decltype(std::free)*>& pInfo,
        DWORD                                                           infoSize,
        DWORD&                                                          cbSize,
        const HANDLE                                                    driveHandle,
        GET_STORAGE_DEPENDENCY_FLAG                                     flags);

protected:
    std::wstring _diskPath;
    std::wstring _parentPath;
    HANDLE _handle;
};