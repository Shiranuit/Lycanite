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
    //_parameters.Version2.ParentPath = parentPath.empty() ? nullptr : parentPath.c_str();
    _parameters.Version2.ParentPath = nullptr;


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

        if (opStatus != ERROR_SUCCESS || !_handle) {
            std::cout << opStatus << " : " << std::endl;
            printf("????1\n");
            throw std::runtime_error("Error while creating virtual disk, code: " + opStatus);
        }
    } else {
        printf("????2\n");
        throw std::runtime_error("Disk already created.");
    }
}

VirtualDisk::~VirtualDisk()
{
    if (_handle)
        CloseHandle(_handle);
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