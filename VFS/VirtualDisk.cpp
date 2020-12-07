#include "VirtualDisk.h"

VirtualDisk::VirtualDisk() : _handle(nullptr)
{
    _diskInfo = GET_VIRTUAL_DISK_INFO();
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
    }
    else {
        throw std::runtime_error("Disk already created.");
    }
}

void VirtualDisk::open(const std::wstring& diskPath, const VIRTUAL_DISK_ACCESS_MASK& access_mask, const OPEN_VIRTUAL_DISK_FLAG& open_flag)
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
        access_mask,
        open_flag,
        &openParameters,
        &_handle);

    if (opStatus != ERROR_SUCCESS)
        throw std::runtime_error("Error while opening virtual disk, code: " + opStatus);
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

    if (opStatus != ERROR_SUCCESS) {
        throw std::runtime_error("opStatus: " + opStatus);
    } else {
        return _diskInfo;
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
