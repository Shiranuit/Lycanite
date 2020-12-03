#include "DynamicDisk.h"

int main() {
	DynamicDisk disk(L"E:\fixed.vhdx", L"", 1073741824, 0, 0, 0);
	disk.create();
}