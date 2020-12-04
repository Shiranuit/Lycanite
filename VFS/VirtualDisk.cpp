#include "VirtualDisk.h"

VirtualDisk::VirtualDisk(
    const std::wstring              &virtualDiskPath,
    const std::wstring              &parentPath,
    const CREATE_VIRTUAL_DISK_FLAG  &flags,
    ULONGLONG                       fileSize,
    DWORD                           blockSize,
    DWORD                           logicalSectorSize,
    DWORD                           physicalSectorSize
    ) :
    _diskPath(virtualDiskPath),
    _parentPath(parentPath),
    _flags(flags),
    _handle(nullptr)
{
    GUID uniqueId = { 0 };

    if (RPC_S_OK != UuidCreate(reinterpret_cast<UUID *>(&uniqueId)))
        throw std::bad_alloc();

    // Storage Type
    _storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    _storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN;

    // Setup paramaters
    std::memset(&_parameters, 0, sizeof(_parameters));
    _parameters.Version = CREATE_VIRTUAL_DISK_VERSION_2;
    _parameters.Version2.UniqueId = uniqueId;
    _parameters.Version2.MaximumSize = fileSize;
    _parameters.Version2.BlockSizeInBytes = blockSize;
    _parameters.Version2.SectorSizeInBytes = logicalSectorSize;
    _parameters.Version2.PhysicalSectorSizeInBytes = physicalSectorSize;
    _parameters.Version2.ParentPath = _parentPath.empty() ? nullptr : _parentPath.c_str();

    if (fileSize % 512 != 0)
        throw std::runtime_error("fileSize is not a multiple of 512");
}

void VirtualDisk::create()
{
    if (!_handle) {
        DWORD opStatus = CreateVirtualDisk(
            &_storageType,
            _diskPath.c_str(),
            VIRTUAL_DISK_ACCESS_NONE,
            nullptr,
            _flags,
            0,
            &_parameters,
            nullptr,
            &_handle
        );

        if (opStatus != ERROR_SUCCESS || !_handle)
            throw std::runtime_error("Error while creating virtual disk, code: " + opStatus);
    } else {
        throw std::runtime_error("Disk already created.");
    }
}

VirtualDisk::~VirtualDisk()
{
    if (_handle)
        CloseHandle(_handle);
}

PGET_VIRTUAL_DISK VirtualDisk::getDiskInfo()
{
    PGET_VIRTUAL_DISK_INFO diskInfo;
    ULONG diskInfoSize;
    DWORD opStatus;
    GUID identifier;

    diskInfo = NULL;
    diskInfoSize = sizeof(GET_VIRTUAL_DISK_INFO);
    diskInfo = (PGET_VIRTUAL_DISK_INFO)malloc(diskInfoSize);
    if (diskInfo == NULL)
        opStatus = ERROR_NOT_ENOUGH_MEMORY;

    // Get the VHD/VHDX type.
    diskInfo->Version = GET_VIRTUAL_DISK_INFO_PROVIDER_SUBTYPE;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        diskInfo,
        NULL);
    
    // Get the VHD/VHDX format. 
    diskInfo->Version = GET_VIRTUAL_DISK_INFO_VIRTUAL_STORAGE_TYPE;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        diskInfo,
        NULL);

    // Get the VHD/VHDX virtual disk size.
    diskInfo->Version = GET_VIRTUAL_DISK_INFO_SIZE;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        diskInfo,
        NULL);

    // Get the VHD physical sector size.
    diskInfo->Version = GET_VIRTUAL_DISK_INFO_VHD_PHYSICAL_SECTOR_SIZE;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        diskInfo,
        NULL);

    // Get the virtual disk ID.
    diskInfo->Version = GET_VIRTUAL_DISK_INFO_IDENTIFIER;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        diskInfo,
        NULL);

    // Get the VHD parent path.
    if (diskInfo->ProviderSubtype == 0x4)
    {
        // Get parent location
        diskInfo->Version = GET_VIRTUAL_DISK_INFO_PARENT_LOCATION;
        opStatus = GetVirtualDiskInformation(
            _handle,
            &diskInfoSize,
            diskInfo,
            NULL);

        // Get parent ID.
        diskInfo->Version = GET_VIRTUAL_DISK_INFO_PARENT_IDENTIFIER;
        opStatus = GetVirtualDiskInformation(
            _handle,
            &diskInfoSize,
            diskInfo,
            NULL);
    }
    
    // Get the VHD minimum internal size.
    diskInfo->Version = GET_VIRTUAL_DISK_INFO_SMALLEST_SAFE_VIRTUAL_SIZE;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        diskInfo,
        NULL);

    // Get the VHD fragmentation percentage.
    diskInfo->Version = GET_VIRTUAL_DISK_INFO_FRAGMENTATION;
    opStatus = GetVirtualDiskInformation(
        _handle,
        &diskInfoSize,
        diskInfo,
        NULL);

    if (opStatus != ERROR_SUCCESS) {
        throw std::runtime_error("opStatus: " + std::to_string(opStatus));
        return nullptr;
    } else {
        return diskInfo;
    }

}

const std::wstring &VirtualDisk::getDiskPath() const
{
    return (_diskPath);
}

const CREATE_VIRTUAL_DISK_PARAMETERS &VirtualDisk::getParameters() const
{
    return (_parameters);
}

const VIRTUAL_STORAGE_TYPE &VirtualDisk::getStorageType() const
{
    return (_storageType);
}

const HANDLE VirtualDisk::getHandle() const
{
    return (_handle);
}
