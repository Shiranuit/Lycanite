#include "DereferencingDisk.h"

DereferencingDisk::DereferencingDisk() : VirtualDisk()
{
}

void DereferencingDisk::create(const std::wstring& virtualDiskPath, const std::wstring& parentPath)
{
    VirtualDisk::create(
        virtualDiskPath,
        parentPath,
        CREATE_VIRTUAL_DISK_FLAG_NONE,
        0,
        0,
        0,
        0,
        OPEN_VIRTUAL_DISK_FLAG_NONE | OPEN_VIRTUAL_DISK_FLAG_CUSTOM_DIFF_CHAIN
    );
}

void DereferencingDisk::open(const std::wstring& diskPath, const VIRTUAL_DISK_ACCESS_MASK& accessMask, const OPEN_VIRTUAL_DISK_FLAG& openFlag)
{
    VirtualDisk::open(
        diskPath,
        accessMask,
        openFlag | OPEN_VIRTUAL_DISK_FLAG_CUSTOM_DIFF_CHAIN
    );
}

//
// DO NOT attempt to perform a live merge of a leaf (a)VHD or (a)VHDX of a VM as the
// operation will not update the virtual machine configuration file.
//
void DereferencingDisk::mergeToParent() const
{
    MERGE_VIRTUAL_DISK_PARAMETERS mergeParameters;
    DWORD opStatus;

    memset(&mergeParameters, 0, sizeof(mergeParameters));
    mergeParameters.Version = MERGE_VIRTUAL_DISK_VERSION_2;

    // Actual disk is being merged so the source depth is 1.
    mergeParameters.Version2.MergeSourceDepth = 1;

    // Actual disk is being merged only to it's parent so the target depth is 2
    mergeParameters.Version2.MergeTargetDepth = 2;

    opStatus = MergeVirtualDisk(
        _handle,
        MERGE_VIRTUAL_DISK_FLAG_NONE,
        &mergeParameters,
        nullptr);

    if (opStatus != ERROR_SUCCESS)
        throw std::runtime_error("Error while merging to its parent, code: " + opStatus);
}

DereferencingDisk::~DereferencingDisk()
{
}

void DereferencingDisk::addVirtualDiskParent(const std::wstring& parentPath)
{
    DWORD status;

    status = AddVirtualDiskParent(_handle, parentPath.c_str());
    if (status != ERROR_SUCCESS) {
        throw std::runtime_error("error in addVirtualDiskParent. code = " + status);
    }
}