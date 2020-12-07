#include "DynamicDisk.h"

int main() {
    DynamicDisk disk(L"D:\\OSfixed.vhdx", L"", 1073741824, 0, 0, 0);
    disk.create();
    GET_VIRTUAL_DISK_INFO infos = disk.getDiskInfo();
    std::cout << infos.Version <<std::endl;
}