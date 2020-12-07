#include "DereferencingDisk.h"

DereferencingDisk::DereferencingDisk() : VirtualDisk(VIRTUAL_DISK_TYPE::DEREFERENCING, true)
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
        0
    );
}

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