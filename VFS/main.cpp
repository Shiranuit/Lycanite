#include "DynamicDisk.h"

int main() {
    DynamicDisk disk;

    disk.create(L"C:\\Users\\tepia\\Desktop\\OSfixed.vhdx", L"", 1073741824, 0, 0, 0);

    disk.mirror(L"C:\\Users\\tepia\\Desktop\\mirror.vhdx");
}