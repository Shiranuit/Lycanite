#include "vhd.h"

int VHD_destroy(VHD_Handle *handle) 
{
    CloseHandle(handle->handle);

    if (remove(handle->diskPath)) {
        printf("Disk deleted successfully");
        free(handle);
        return 0;
    } else {
        printf("Error: unable to delete disk");
        return 1;
    }
}