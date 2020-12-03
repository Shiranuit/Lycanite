#pragma once

#include <fltkernel.h>
#include <suppress.h>
#include <dontuse.h>
#include <wdm.h>
#include <ntddk.h>

/* ======================================================
*                       DEFINES & MACROS
*  ======================================================*/



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
*                       Callbacks
*  ======================================================*/

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
