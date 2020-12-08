#include "VirtualDisk.h"

VirtualDisk::VirtualDisk() : _handle(nullptr)
{
}

VirtualDisk::~VirtualDisk()
{
    close();
}

void VirtualDisk::create(
    const std::wstring&             virtualDiskPath,
    const std::wstring&             parentPath,
    const CREATE_VIRTUAL_DISK_FLAG& flags,
    ULONGLONG                       fileSize,
    DWORD                           blockSize,
    DWORD                           logicalSectorSize,
    DWORD                           physicalSectorSize)
{
    if (!_handle) {
        VIRTUAL_STORAGE_TYPE storageType = { 0 };
        CREATE_VIRTUAL_DISK_PARAMETERS parameters;
        DWORD opStatus;
        GUID uniqueId = { 0 };

        _diskPath = virtualDiskPath;
        _parentPath = parentPath;
        if (RPC_S_OK != UuidCreate(&uniqueId))
            throw std::bad_alloc();

        // Storage Type
        storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
        storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN;

        // Setup paramaters
        std::memset(&parameters, 0, sizeof(parameters));
        parameters.Version = CREATE_VIRTUAL_DISK_VERSION_2;
        parameters.Version2.UniqueId = uniqueId;
        parameters.Version2.MaximumSize = fileSize;
        parameters.Version2.BlockSizeInBytes = blockSize;
        parameters.Version2.SectorSizeInBytes = logicalSectorSize;
        parameters.Version2.PhysicalSectorSizeInBytes = physicalSectorSize;
        parameters.Version2.ParentPath = _parentPath.empty() ? nullptr : _parentPath.c_str();

        if (fileSize % 512 != 0)
            throw std::runtime_error("fileSize is not a multiple of 512");

        opStatus = CreateVirtualDisk(
            &storageType,
            _diskPath.c_str(),
            VIRTUAL_DISK_ACCESS_NONE,
            nullptr,
            flags,
            0,
            &parameters,
            nullptr,
            &_handle
        );

        if (opStatus != ERROR_SUCCESS || !_handle)
            throw std::runtime_error("Error while creating virtual disk, code: " + opStatus);
    } else {
        throw std::runtime_error("Disk already created.");
    }
}

void VirtualDisk::open(const std::wstring& diskPath, const VIRTUAL_DISK_ACCESS_MASK& accessMask, const OPEN_VIRTUAL_DISK_FLAG& openFlag)
{
    // Close if disk is already opened
    close();

    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    VIRTUAL_STORAGE_TYPE storageType = { 0 };
    DWORD opStatus;

    _diskPath = diskPath;
    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN;
    std::memset(&openParameters, 0, sizeof(openParameters));
    openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;

    opStatus = OpenVirtualDisk(
        &storageType,
        _diskPath.c_str(),
        accessMask,
        openFlag,
        &openParameters,
        &_handle);

    if (opStatus != ERROR_SUCCESS)
        throw std::runtime_error("Error while opening virtual disk, code: " + opStatus);
}

void VirtualDisk::waitDiskOperation(
    const HANDLE             handle,
    OVERLAPPED&              overlapped,
    const WaiterDiskHandler& progressHandler,
    int                      msWaits) const
{
    VIRTUAL_DISK_PROGRESS progress = { 0 };
    DWORD opStatus;

    while (true) {
        std::memset(&progress, 0, sizeof(progress));
        opStatus = GetVirtualDiskOperationProgress(handle, &overlapped, &progress);
        if (opStatus != ERROR_SUCCESS)
            throw std::runtime_error("Error while mirroring the virtual disk, code: " + opStatus);
        opStatus = progress.OperationStatus;
        if (opStatus != ERROR_IO_PENDING && opStatus != ERROR_SUCCESS)
            throw std::runtime_error("Error while mirroring the virtual disk, code: " + opStatus);
        if (progressHandler(opStatus, progress))
            break;
        std::this_thread::sleep_for(std::chrono::milliseconds(msWaits));
    }
}

DWORD VirtualDisk::getOperationStatusDisk(const HANDLE handle, OVERLAPPED& overlapped, VIRTUAL_DISK_PROGRESS &progress) const
{
    return (GetVirtualDiskOperationProgress(handle, &overlapped, &progress));
}

void VirtualDisk::mirror(const std::wstring& destinationPath)
{
    MIRROR_VIRTUAL_DISK_PARAMETERS mirrorParameters;
    VIRTUAL_DISK_PROGRESS progress = { 0 };
    VIRTUAL_STORAGE_TYPE storageType = { 0 };
    OVERLAPPED overlapped = { 0 };
    DWORD opStatus;

    overlapped.hEvent = CreateEvent(nullptr, true, false, nullptr);
    if (overlapped.hEvent == nullptr)
        throw std::runtime_error("Error: Can't create event, code: " + GetLastError());

    std::memset(&mirrorParameters, 0, sizeof(MIRROR_VIRTUAL_DISK_PARAMETERS));
    mirrorParameters.Version = MIRROR_VIRTUAL_DISK_VERSION_1;
    mirrorParameters.Version1.MirrorVirtualDiskPath = destinationPath.c_str();

    // async task
    opStatus = MirrorVirtualDisk(
        _handle,
        MIRROR_VIRTUAL_DISK_FLAG_NONE,
        &mirrorParameters,
        &overlapped
    );

    // wait to finish
    if (opStatus == ERROR_SUCCESS || opStatus == ERROR_IO_PENDING) {
        waitDiskOperation(_handle, overlapped, [](const DWORD& status, const VIRTUAL_DISK_PROGRESS& progress) -> bool {
            if (status == ERROR_IO_PENDING)
                if (progress.CurrentValue == progress.CompletionValue)
                    return (true);
            return (false);
        });
    }
    else
        throw std::runtime_error("Error while mirroring the virtual disk, code: " + opStatus);

    // Break the mirror.  Breaking the mirror will activate the new target and cause it to be
    // utilized in place of the original VHD/VHDX.
    // async task
    opStatus = BreakMirrorVirtualDisk(_handle);

    // wait to finish
    if (opStatus != ERROR_SUCCESS)
        throw std::runtime_error("Error while breaking mirror of the virtual disk, code: " + opStatus);
    else {
        waitDiskOperation(_handle, overlapped, [](const DWORD& status, const VIRTUAL_DISK_PROGRESS& progress) -> bool {
            if (status == ERROR_SUCCESS)
                return (true);
            return (false);
        });
    }
}

bool VirtualDisk::isOpen() const
{
    return (_handle && _handle != INVALID_HANDLE_VALUE);
}

void VirtualDisk::getStorageDependencyInfo() const
{
    PSTORAGE_DEPENDENCY_INFO pInfo = { 0 }; // should a vector
    DWORD infoSize;
    DWORD cbSize;
    HANDLE driveHandle = nullptr;
    GET_STORAGE_DEPENDENCY_FLAG flags;
    BOOL isDisk;
    DWORD opStatus;
    DWORD entry;
    TCHAR szVolume[8] = L"\\\\.\\C:\\";
    TCHAR szDisk[19] = L"\\\\.\\PhysicalDrive0";

    std::wstring Disk(L"...");

    if (Disk[0] >= L'0' && Disk[0] <= L'9') {
        // Assume the user is specifying a disk between 0 and 9
        isDisk = TRUE;
        szDisk[17] = Disk[0];
        flags = GET_STORAGE_DEPENDENCY_FLAG_PARENTS | GET_STORAGE_DEPENDENCY_FLAG_DISK_HANDLE;
    } else {
        // Assume the user is specifying a drive letter between A: and Z:
        isDisk = FALSE;
        szVolume[4] = Disk[0];
        flags = GET_STORAGE_DEPENDENCY_FLAG_PARENTS;
    }

    driveHandle = INVALID_HANDLE_VALUE;
    pInfo = nullptr;

    // Allocate enough memory for most basic case.
    infoSize = sizeof(STORAGE_DEPENDENCY_INFO);

    pInfo = (PSTORAGE_DEPENDENCY_INFO)std::malloc(infoSize);
    if (pInfo == nullptr)
        throw std::bad_alloc();

    std::memset(pInfo, 0, infoSize);

    // Open the drive
    driveHandle = CreateFile(
        (isDisk ? szDisk : szVolume),
        GENERIC_READ,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        nullptr,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS,
        nullptr);

    if (driveHandle == INVALID_HANDLE_VALUE)
        throw std::runtime_error("Error while getting storage dependency information, code: " + GetLastError());

    // Determine the size actually required.
    pInfo->Version = STORAGE_DEPENDENCY_INFO_VERSION_2;
    cbSize = 0;
    opStatus = GetStorageDependencyInformation(
        driveHandle,
        flags,
        infoSize,
        pInfo,
        &cbSize
    );

    if (opStatus == ERROR_INSUFFICIENT_BUFFER) {
        // Allocate a large enough buffer.
        free(pInfo);
        infoSize = cbSize;
        pInfo = (PSTORAGE_DEPENDENCY_INFO)std::malloc(infoSize);
        if (pInfo == nullptr)
            throw std::bad_alloc();

        std::memset(pInfo, 0, infoSize);

        // Retry with large enough buffer.
        pInfo->Version = STORAGE_DEPENDENCY_INFO_VERSION_2;
        cbSize = 0;

        opStatus = GetStorageDependencyInformation(
            driveHandle,
            GET_STORAGE_DEPENDENCY_FLAG_PARENTS,
            infoSize,
            pInfo,
            &cbSize
        );
    }

    if (opStatus != ERROR_SUCCESS)
        throw std::runtime_error("Error while getting storage dependency information. It is most likely due that disk is not mounted.");

    // Display the relationship between the specified volume and the underlying disks.
    for (DWORD entry = 0; entry < pInfo->NumberEntries; entry++) {
        wprintf(L"%u:\n", entry);
        wprintf(L"   %u\n", pInfo->Version2Entries[entry].AncestorLevel);
        wprintf(L"   %s\n", pInfo->Version2Entries[entry].DependencyDeviceName);
        wprintf(L"   %s\n", pInfo->Version2Entries[entry].HostVolumeName);
        wprintf(L"   %s\n", pInfo->Version2Entries[entry].DependentVolumeName);
        wprintf(L"   %s\n", pInfo->Version2Entries[entry].DependentVolumeRelativePath);
    }
}

bool VirtualDisk::close()
{
    bool closed = false;

    if (isOpen()) {
        closed = CloseHandle(_handle);
        _handle = nullptr;
        return (closed);
    }
    return (closed);
}

const std::wstring& VirtualDisk::getDiskPath() const
{
    return (_diskPath);
}

const HANDLE VirtualDisk::getHandle() const
{
    return (_handle);
}