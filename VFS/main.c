#include "vhd.h"

int main(int argc, char **argv)
{
    _Out_opt_ DWORD opStatus;
    VHD_Handle *vhd_handle = VHD_create_dynamic(L"E:\\fixed.vhdx", NULL, 1073741824, 0, 0, 0, &opStatus);
    return 0;
}