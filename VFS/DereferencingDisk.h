#pragma once

#include "VirtualDisk.h"

class DereferencingDisk : public VirtualDisk {
    public:

    DereferencingDisk(const std::wstring &virtualDiskPath, const std::wstring &parentPath);

    ~DereferencingDisk();
};

