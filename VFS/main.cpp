#include "DynamicDisk.h"

int main() {
    DynamicDisk disk;

    disk.create(L"D:\\OSfixed.vhdx", L"", 1073741824, 0, 0, 0);
    GET_VIRTUAL_DISK_INFO infos = disk.getDiskInfo();
    std::cout << infos.Version <<std::endl;
}