#include "DynamicDisk.h"
#include "DereferencingDisk.h"

int main() {
    DynamicDisk disk;

    disk.create(L"C:\\Users\\tepia\\Desktop\\OSfixed.vhdx", 1073741824, 0, 0, 0);

    DereferencingDisk diffDisk;

    diffDisk.create(L"C:\\Users\\tepia\\Desktop\\diff.vhdx", L"C:\\Users\\tepia\\Desktop\\OSfixed.vhdx");

    diffDisk.mergeToParent();
}