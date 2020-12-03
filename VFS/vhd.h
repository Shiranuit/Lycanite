#pragma once

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <initguid.h>
#include <virtdisk.h>
#include <rpcdce.h>
#include <rpc.h>

typedef struct VHD_Handle {
    _In_ LPCWSTR diskPath;
    _In_ CREATE_VIRTUAL_DISK_FLAG flags;
    CREATE_VIRTUAL_DISK_PARAMETERS parameters;
    VIRTUAL_STORAGE_TYPE storageType;
    HANDLE handle;
} VHD_Handle;

VHD_Handle *VHD_create(
    _In_        LPCWSTR                     virtualDiskPath,
    _In_opt_    LPCWSTR                     parentPath,
    _In_        CREATE_VIRTUAL_DISK_FLAG    flags,
    _In_        ULONGLONG                   fileSize,
    _In_        DWORD                       blockSize,
    _In_        DWORD                       logicalSectorSize,
    _In_        DWORD                       physicalSectorSize,
    _Out_opt_   DWORD*                      outStatus);

VHD_Handle *VHD_create_differencing(
    _In_      LPCWSTR virtualDiskPath,
    _In_opt_  LPCWSTR parentPath,
    _Out_opt_ DWORD*  outStatus);

VHD_Handle *VHD_create_dynamic(
    _In_      LPCWSTR   virtualDiskPath,
    _In_opt_  LPCWSTR   parentPath,
    _In_      ULONGLONG fileSize,
    _In_      DWORD     blockSize,
    _In_      DWORD     logicalSectorSize,
    _In_      DWORD     physicalSectorSize,
    _Out_opt_ DWORD*    outStatus);

VHD_Handle *VHD_create_fixed(
    _In_        LPCWSTR   virtualDiskPath,
    _In_opt_    LPCWSTR   parentPath,
    _In_        ULONGLONG fileSize,
    _In_        DWORD     blockSize,
    _In_        DWORD     logicalSectorSize,
    _In_        DWORD     physicalSectorSize,
    _Out_opt_   DWORD*    outStatus);

int VHD_destroy(VHD_Handle *handle);