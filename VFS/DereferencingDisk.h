#pragma once

#include "VirtualDisk.h"

class DereferencingDisk : public VirtualDisk {
public:

    DereferencingDisk();

    ~DereferencingDisk();

    void create(const std::wstring& virtualDiskPath, const std::wstring& parentPath);

    /**
    * Merge Dereferencing disk to its parent
    * DO NOT attempt to perform a live merge of a leaf (a)VHD or (a)VHDX of a VM as the
    * operation will not update the virtual machine configuration file.
    */
    void mergeToParent() const;

    const VIRTUAL_DISK_TYPE& getType() const;
};