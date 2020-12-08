#pragma once

#include "VirtualDisk.h"

class DereferencingDisk : public VirtualDisk {
public:

    DereferencingDisk();

    void create(const std::wstring& virtualDiskPath, const std::wstring& parentPath);

    void open(const std::wstring& diskPath, const VIRTUAL_DISK_ACCESS_MASK& accessMask = VIRTUAL_DISK_ACCESS_NONE, const OPEN_VIRTUAL_DISK_FLAG& openFlag = OPEN_VIRTUAL_DISK_FLAG_NONE);

    void mergeToParent() const;

    ~DereferencingDisk();

    void addVirtualDiskParent(const std::wstring& parentPath);
};