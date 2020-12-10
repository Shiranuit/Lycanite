#pragma once

#include <fltkernel.h>
#include <suppress.h>
#include <dontuse.h>
#include <wdm.h>
#include <ntddk.h>

/* ======================================================
*                       Driver
*  ======================================================*/

DRIVER_INITIALIZE DriverEntry;

NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
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
*                       Filter
*  ======================================================*/

NTSTATUS CgUnload(FLT_FILTER_UNLOAD_FLAGS Flags);

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
