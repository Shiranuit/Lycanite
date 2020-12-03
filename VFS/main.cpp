#include "DynamicDisk.h"

int main() {
    DynamicDisk disk(L"C:\\Users\\tepia\\Desktop\\OSfixed.vhdx", L"", 1073741824, 0, 0, 0);
    disk.create();
}