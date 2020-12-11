#pragma once

#include <ntddk.h>

#if defined(__cplusplus)
extern "C" {
#endif

	static UINT32 my_strlen(CONST PCHAR str);

    // override calloc for kernel
    static PVOID calloc(SIZE_T elementCount, SIZE_T elementSize);

    // override free for kernel
    static VOID free(PVOID mem);

    // override memcpy for kernel
    static PVOID Kmemcpy(PVOID destination, CONST PVOID source, SIZE_T size);

    // override memcmp
    static INT32 Kmemcmp(CONST PVOID pointer1, CONST PVOID pointer2, SIZE_T size);

    // override memset
    static PVOID Kmemset(PVOID pointer, INT8 value, SIZE_T count);

#if defined(__cplusplus)
}
#endif

UINT32 my_strlen(CONST PWCHAR str)
{
	UINT32 len = 0;

	for (SIZE_T i = 0; str[i] != '\0'; i++) {
		len += 1;
	}
	return len;
}

PVOID malloc(SIZE_T memory_size) {
    return ExAllocatePoolWithTag(NonPagedPoolNx, memory_size, 'aloc');
}

PVOID calloc(SIZE_T elementCount, SIZE_T elementSize) {
    SIZE_T total_size = elementCount * elementSize;
    PCHAR mem = (PCHAR)ExAllocatePoolWithTag(NonPagedPoolNx, elementCount * elementSize, 'aloc');

    // init data allocated with 0 like calloc
    for (SIZE_T i = 0; mem != NULL && i < total_size; i++) {
        mem[i] = 0;
    }

    return (PVOID)mem;
}

VOID free(PVOID mem) {
    if (mem != NULL) {
        ExFreePoolWithTag(mem, 'aloc');
    }
}

PVOID Kmemcpy(PVOID destination, CONST PVOID source, SIZE_T size) {
    RtlCopyMemory(destination, source, size);

    return destination;
}

// override memcmp
INT32 Kmemcmp(CONST PVOID pointer1, CONST PVOID pointer2, SIZE_T size) {
    if (pointer1 == pointer2) return 0;
    for (SIZE_T i = 0; i < size; i++) {
        if (((PCHAR)pointer1)[i] != ((PCHAR)pointer2)[i]) {
            return (((PCHAR)pointer1)[i] - ((PCHAR)pointer2)[i]);
        }
    }
    return 0;
}

// override memset
PVOID Kmemset(PVOID pointer, INT8 value, SIZE_T count) {
    for (SIZE_T i = 0; pointer != NULL && i < count; i++) {
        ((PCHAR)pointer)[i] = value;
    }
    return (pointer);
}