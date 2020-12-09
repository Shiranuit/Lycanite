#include "VirtualDisk.h"

VirtualDisk::VirtualDisk() : _handle(nullptr), _diskInfo()
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

const GET_VIRTUAL_DISK_INFO &VirtualDisk::getDiskInfo()
{
    ULONG diskInfoSize = sizeof(GET_VIRTUAL_DISK_INFO);
    DWORD opStatus;
    GUID identifier;

    // Get the VHD/VHDX type.
    _diskInfo.Version = GET_VIRTUAL_DISK_INFO_PROVIDER_SUBTYPE;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        &_diskInfo,
        NULL);
    
    // Get the VHD/VHDX format. 
    _diskInfo.Version = GET_VIRTUAL_DISK_INFO_VIRTUAL_STORAGE_TYPE;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        &_diskInfo,
        NULL);

    // Get the VHD/VHDX virtual disk size.
    _diskInfo.Version = GET_VIRTUAL_DISK_INFO_SIZE;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        &_diskInfo,
        NULL);

    // Get the VHD physical sector size.
    _diskInfo.Version = GET_VIRTUAL_DISK_INFO_VHD_PHYSICAL_SECTOR_SIZE;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        &_diskInfo,
        NULL);

    // Get the virtual disk ID.
    _diskInfo.Version = GET_VIRTUAL_DISK_INFO_IDENTIFIER;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        &_diskInfo,
        NULL);

    // Get the VHD parent path.
    if (_diskInfo.ProviderSubtype == 0x4)
    {
        // Get parent location
        _diskInfo.Version = GET_VIRTUAL_DISK_INFO_PARENT_LOCATION;
        opStatus = GetVirtualDiskInformation(
            _handle,
            &diskInfoSize,
            &_diskInfo,
            NULL);

        // Get parent ID.
        _diskInfo.Version = GET_VIRTUAL_DISK_INFO_PARENT_IDENTIFIER;
        opStatus = GetVirtualDiskInformation(
            _handle,
            &diskInfoSize,
            &_diskInfo,
            NULL);

        // Get parent timestamp.
        _diskInfo.Version = GET_VIRTUAL_DISK_INFO_PARENT_TIMESTAMP;
        opStatus = GetVirtualDiskInformation(
            _handle,
            &diskInfoSize,
            &_diskInfo,
            NULL);
    }
    
    // Get the VHD minimum internal size.
    _diskInfo.Version = GET_VIRTUAL_DISK_INFO_SMALLEST_SAFE_VIRTUAL_SIZE;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        &_diskInfo,
        NULL);

    // Get the VHD fragmentation percentage.
    _diskInfo.Version = GET_VIRTUAL_DISK_INFO_FRAGMENTATION;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        &_diskInfo,
        NULL);

    // Get the VHD alignement.
    _diskInfo.Version = GET_VIRTUAL_DISK_INFO_IS_4K_ALIGNED;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        &_diskInfo,
        NULL);

    // Get the VHD physical disk.
    _diskInfo.Version = GET_VIRTUAL_DISK_INFO_PHYSICAL_DISK;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        &_diskInfo,
        NULL);

    // Get the VHD loaded state.
    _diskInfo.Version = GET_VIRTUAL_DISK_INFO_IS_LOADED;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        &_diskInfo,
        NULL);

    // get virtual disk id
    _diskInfo.Version = GET_VIRTUAL_DISK_INFO_VIRTUAL_DISK_ID;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        &_diskInfo,
        NULL);

    // get tracking state
    _diskInfo.Version = GET_VIRTUAL_DISK_INFO_CHANGE_TRACKING_STATE;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        &_diskInfo,
        NULL);

    if (opStatus != ERROR_SUCCESS) {
        throw std::runtime_error("Error getInfos: " + opStatus);
    } else {
        return _diskInfo;
    }
}

void VirtualDisk::setDiskInfo(SET_VIRTUAL_DISK_INFO diskInfo)
{
    DWORD opStatus = SetVirtualDiskInformation(_handle, &diskInfo);

    if (opStatus != ERROR_SUCCESS)
        throw std::runtime_error("Error setInfos: " + opStatus);
}

void VirtualDisk::setDiskInfo(std::wstring parentPath, DWORD physicalSectorSize)
{
    SET_VIRTUAL_DISK_INFO diskInfo;

    diskInfo.Version = SET_VIRTUAL_DISK_INFO_PARENT_PATH_WITH_DEPTH;
    diskInfo.ParentPathWithDepthInfo.ChildDepth = 1;
    diskInfo.ParentPathWithDepthInfo.ParentFilePath = parentPath.c_str();

    DWORD opStatus = SetVirtualDiskInformation(_handle, &diskInfo);
    
    if (opStatus != ERROR_SUCCESS)
        throw std::runtime_error("Error setInfos: " + opStatus);


    diskInfo.Version = SET_VIRTUAL_DISK_INFO_PHYSICAL_SECTOR_SIZE;
    diskInfo.VhdPhysicalSectorSize = physicalSectorSize;

    opStatus = SetVirtualDiskInformation(_handle, &diskInfo);

    if (opStatus != ERROR_SUCCESS)
        throw std::runtime_error("Error setInfos: " + opStatus);
}

void VirtualDisk::setUserMetaData(const PVOID &data, const GUID &uniqueId, const ULONG& nbToWrite)
{
    DWORD status;

    status = SetVirtualDiskMetadata(
        _handle,
        &uniqueId,
        nbToWrite,
        data);

    if (status != ERROR_SUCCESS) {
        throw std::runtime_error("error = " + status);
    }
}

void VirtualDisk::getUserMetaData(const GUID &uniqueId, ULONG &metaDataSize, const std::shared_ptr<VOID> &data) const
{
    DWORD status;

    status = GetVirtualDiskMetadata(
        _handle,
        &uniqueId,
        &metaDataSize,
        data.get());

    if (status != ERROR_SUCCESS) {
        throw std::runtime_error("error = " + status);
    }
}

void VirtualDisk::deleteUserMetaData(const GUID &uniqueId)
{
    DWORD status;
    status = DeleteVirtualDiskMetadata(_handle, &uniqueId);
    if (status != ERROR_SUCCESS) {
        throw std::runtime_error("error in deleteUserMetaData. code = " + status);
    }
}

std::unique_ptr<std::vector<GUID>> VirtualDisk::enumerateUserMetaData() const
{
    std::unique_ptr<std::vector<GUID>> guids;
    DWORD status;
    ULONG numberOfItems = 0;

    status = EnumerateVirtualDiskMetadata(_handle, &numberOfItems, nullptr);
    if (status != ERROR_SUCCESS && status != ERROR_MORE_DATA) {
        throw std::runtime_error("error in enumerateUserMetaData. code = " + status);
    }
    guids = std::make_unique<std::vector<GUID>>(numberOfItems);
    status = EnumerateVirtualDiskMetadata(_handle, &numberOfItems, guids.get()->data());
    if (status != ERROR_SUCCESS && status != ERROR_MORE_DATA) {
        throw std::runtime_error("error in enumerateUserMetaData. code = " + status);
    }
    return (std::move(guids));
}
