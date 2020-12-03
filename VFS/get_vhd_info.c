#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <initguid.h>
#include <strsafe.h>
#include <virtdisk.h>

void cleanupInfo(DWORD opStatus, PGET_VIRTUAL_DISK_INFO diskInfo, HANDLE vhdHandle)
{
    if (opStatus == ERROR_SUCCESS) {
        wprintf(L"success\n");
    } else {
        wprintf(L"error = %u\n", opStatus);
    }
    
    if (vhdHandle != INVALID_HANDLE_VALUE) {
        CloseHandle(vhdHandle);
    }

    if (diskInfo != NULL) {
        free(diskInfo);
    }
}

PGET_VIRTUAL_DISK_INFO get_vhd_info(_In_ LPCWSTR VirtualDiskPath)
{
    OPEN_VIRTUAL_DISK_PARAMETERS openParameters;
    VIRTUAL_STORAGE_TYPE storageType;
    PGET_VIRTUAL_DISK_INFO diskInfo;
    ULONG diskInfoSize;
    DWORD opStatus;

    HANDLE vhdHandle;

    vhdHandle = INVALID_HANDLE_VALUE;
    diskInfo = NULL;
    diskInfoSize = sizeof(GET_VIRTUAL_DISK_INFO);

    diskInfo = (PGET_VIRTUAL_DISK_INFO)malloc(diskInfoSize);
    if (diskInfo == NULL) {
        opStatus = ERROR_NOT_ENOUGH_MEMORY;
        cleanupInfo(opStatus, diskInfo, vhdHandle);
    }
    
    storageType.DeviceId = VIRTUAL_STORAGE_TYPE_DEVICE_UNKNOWN;
    storageType.VendorId = VIRTUAL_STORAGE_TYPE_VENDOR_UNKNOWN;
    
    memset(&openParameters, 0, sizeof(openParameters));
    openParameters.Version = OPEN_VIRTUAL_DISK_VERSION_2;
    openParameters.Version2.GetInfoOnly = TRUE;

    opStatus = OpenVirtualDisk(
        &storageType,
        VirtualDiskPath,
        VIRTUAL_DISK_ACCESS_NONE,
        OPEN_VIRTUAL_DISK_FLAG_NO_PARENTS,
        &openParameters,
        &vhdHandle);

    if (opStatus != ERROR_SUCCESS) {
        cleanupInfo(opStatus, diskInfo, vhdHandle);
    }

    diskInfo->Version = GET_VIRTUAL_DISK_INFO_PROVIDER_SUBTYPE;

    opStatus = GetVirtualDiskInformation(
        vhdHandle,
        &diskInfoSize,
        diskInfo,
        NULL);

    if (opStatus != ERROR_SUCCESS) {
        cleanupInfo(opStatus, diskInfo, vhdHandle);
    }
    return (diskInfo);
}