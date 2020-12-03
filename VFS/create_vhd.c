#include "vhd.h"

VHD_Handle *VHD_create_differencing(
    _In_      LPCWSTR virtualDiskPath,
    _In_opt_  LPCWSTR parentPath,
    _Out_opt_ DWORD*  outStatus)
{
    return VHD_create(
        virtualDiskPath,
        parentPath,
        CREATE_VIRTUAL_DISK_FLAG_NONE,
        0,
        0,
        0,
        0,
        outStatus
    );
}

VHD_Handle *VHD_create_dynamic(
    _In_      LPCWSTR   virtualDiskPath,
    _In_opt_  LPCWSTR   parentPath,
    _In_      ULONGLONG fileSize,
    _In_      DWORD     blockSize,
    _In_      DWORD     logicalSectorSize,
    _In_      DWORD     physicalSectorSize,
    _Out_opt_ DWORD*    outStatus)
{
    return VHD_create(
        virtualDiskPath,
        parentPath,
        CREATE_VIRTUAL_DISK_FLAG_NONE,
        fileSize,
        blockSize,
        logicalSectorSize,
        physicalSectorSize,
        outStatus
    );
}

VHD_Handle *VHD_create_fixed(
    _In_      LPCWSTR   virtualDiskPath,
    _In_opt_  LPCWSTR   parentPath,
    _In_      ULONGLONG fileSize,
    _In_      DWORD     blockSize,
    _In_      DWORD     logicalSectorSize,
    _In_      DWORD     physicalSectorSize,
    _Out_opt_ DWORD*    outStatus)
{
    return VHD_create(
        virtualDiskPath,
        parentPath,
        CREATE_VIRTUAL_DISK_FLAG_FULL_PHYSICAL_ALLOCATION,
        fileSize,
        blockSize,
        logicalSectorSize,
        physicalSectorSize,
        outStatus
    );
}

VHD_Handle *VHD_create(
    _In_        LPCWSTR                     virtualDiskPath,
    _In_opt_    LPCWSTR                     parentPath,
    _In_        CREATE_VIRTUAL_DISK_FLAG    flags,
    _In_        ULONGLONG                   fileSize,
    _In_        DWORD                       blockSize,
    _In_        DWORD                       logicalSectorSize,
    _In_        DWORD                       physicalSectorSize,
    _Out_opt_   DWORD*                      outStatus)
{
    GUID uniqueId;
    DWORD opStatus;

    if (RPC_S_OK != UuidCreate((UUID *)&uniqueId)) {
        if (outStatus)
            *outStatus = ERROR_NOT_ENOUGH_MEMORY;
        return (NULL);
    }

    // Init VHD Handle
    VHD_Handle *vhd_handle = malloc(sizeof(VHD_Handle));

    if (vhd_handle == NULL) {
        if (outStatus)
            *outStatus = ERROR_NOT_ENOUGH_MEMORY;
        return (NULL);
    }
    vhd_handle->handle = INVALID_HANDLE_VALUE;
    vhd_handle->diskPath = virtualDiskPath;

    // Storage Type
    vhd_handle->storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    vhd_handle->storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN;

    // Flags
    vhd_handle->flags = flags;

    // Setup paramaters
    memset(&vhd_handle->parameters, 0, sizeof(vhd_handle->parameters));
    vhd_handle->parameters.Version = CREATE_VIRTUAL_DISK_VERSION_2;
    vhd_handle->parameters.Version2.UniqueId = uniqueId;
    vhd_handle->parameters.Version2.MaximumSize = fileSize; // if parent is specified must be equals to zero
    vhd_handle->parameters.Version2.BlockSizeInBytes = blockSize || CREATE_VIRTUAL_DISK_PARAMETERS_DEFAULT_BLOCK_SIZE;
    vhd_handle->parameters.Version2.SectorSizeInBytes = logicalSectorSize;
    vhd_handle->parameters.Version2.PhysicalSectorSizeInBytes = physicalSectorSize;
    vhd_handle->parameters.Version2.ParentPath = parentPath;

    opStatus = CreateVirtualDisk(
        &vhd_handle->storageType,
        vhd_handle->diskPath,
        VIRTUAL_DISK_ACCESS_NONE,
        NULL,
        vhd_handle->flags,
        0,
        &vhd_handle->parameters,
        NULL,
        &vhd_handle->handle
    );

    if (opStatus != ERROR_SUCCESS || !vhd_handle->handle) {
        if (outStatus)
            *outStatus = opStatus;
        free(vhd_handle);
        return (NULL);
    }
    return (vhd_handle);
}