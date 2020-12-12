#pragma once

#ifndef DRIVER_H_INCLUDED
#define DRIVER_H_INCLUDED

#include <fltkernel.h>
#include <suppress.h>
#include <dontuse.h>
#include <wdm.h>
#include <ntddk.h>

#include "IKashmap.h"
#include "UUIDRecycler.h"
#include "Permissions.h"

/* ======================================================
*                       DEFINES & MACROS
*  ======================================================*/

#define PermFlag(x, y) ((x & y) != y)

typedef struct processInfos_s {
    UINT16 referenceCount;
    struct hashmap_s permissions;
    UINT64 uuid;
} processInfos;

enum Permission {
    LYCANITE_WRITE  = 0b001,
    LYCANITE_READ   = 0b010,
    LYCANITE_DELETE = 0b100
};

enum LycaniteAction {
    SET_LYCANITE_PID = 0,
    SET_AUTHORIZATION_PID = 1,
    SET_AUTHORIZATION_GLOBAL = 2,
    GET_PROCESS_STATS = 3,
    DELETE_AUTHORIZATION_PID = 4,
    DELETE_AUTHORIZATION_GLOBAL = 5
};

enum comError {
    INVALID_REQUEST_SIZE = 1,
    UNKNOWN_REQUEST = 2,
    BAD_ALLOC = 3
};

enum UserCallback {
    PROCESS_CREATE = 1,
    PROCESS_DESTROY = 2,
    PROCESS_REQPERM = 3,
};

/* ======================================================
*                       Driver
*  ======================================================*/

DRIVER_INITIALIZE DriverEntry;

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
);

NTSTATUS CgUnload(FLT_FILTER_UNLOAD_FLAGS Flags);



/* ======================================================
*               Communication Callbacks
*  ======================================================*/

NTSTATUS
comConnectNotifyCallback(
    _In_ PFLT_PORT ClientPort,
    _In_ PVOID ServerPortCookie,
    _In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Outptr_result_maybenull_ PVOID* ConnectionCookie
);

VOID
comDisconnectNotifyCallback(
    _In_opt_ PVOID ConnectionCookie
);

NTSTATUS
comMessageNotifyCallback(
    _In_ PVOID ConnectionCookie,
    _In_reads_bytes_opt_(InputBufferSize) PVOID InputBuffer,
    _In_ ULONG InputBufferSize,
    _Out_writes_bytes_to_opt_(OutputBufferSize, *ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferSize,
    _Out_ PULONG ReturnOutputBufferLength
);

/* ======================================================
*                    Communication
*  ======================================================*/

UINT8
comSetLycanitePid(
    _In_ unsigned char* Input,
    _In_ ULONG InputBufferSize
);

UINT8
comSetAuthorizationPid(
    _In_ unsigned char* Input,
    _In_ UINT64 InputBufferSize
);

UINT8
comSetAuthorizationGlobal(
    _In_ unsigned char* Input,
    _In_ UINT64 InputBufferSize
);

UINT8
comGetProcessStats(
    _Out_ unsigned char* Output,
    _In_ UINT64 OutputBufferSize,
    _Out_ PULONG ReturnOutputBufferLength
);

UINT8
comDeleteAuthorizationPid(
    _In_ unsigned char* Input,
    _In_ UINT64 InputBufferSize
);

UINT8
comDeleteAuthorizationGlobal(
    _In_ unsigned char* Input,
    _In_ UINT64 InputBufferSize
);

/* ======================================================
*                       Callbacks
*  ======================================================*/

VOID CreateProcessNotify(
    _In_ HANDLE ParentId,
    _In_ HANDLE ProcessId,
    _In_ BOOLEAN Create
);

FLT_PREOP_CALLBACK_STATUS
AvPreCreate(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

FLT_PREOP_CALLBACK_STATUS
AvPreWrite(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

FLT_PREOP_CALLBACK_STATUS
AvPreRead(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

FLT_PREOP_CALLBACK_STATUS
AvPreSetInformation(
    _Inout_ PFLT_CALLBACK_DATA Data,
    _In_ PCFLT_RELATED_OBJECTS FltObjects,
    _Flt_CompletionContext_Outptr_ PVOID* CompletionContext
);

/* ======================================================
*                       Context
*  ======================================================*/


/* ======================================================
*                       Text Section
*  ======================================================*/

#ifdef ALLOC_PRAGMA
#pragma alloc_text(INIT, DriverEntry)

#pragma alloc_text(PAGE, AvPreCreate)
#pragma alloc_text(PAGE, AvPreWrite)
#pragma alloc_text(PAGE, AvPreRead)
#pragma alloc_text(PAGE, AvPreSetInformation)
#endif

#endif