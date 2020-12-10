#include "Driver.h"
#include "Permissions.h"

/* ======================================================
*                       Filter Structs
*  ======================================================*/

PFLT_FILTER gFilterInstance = NULL;

const FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE, // Filter Action Name
      0,
      AvPreCreate,  // Pointer to the function to execute before the action is executed
      NULL },       // Pointer to the function to execute after the action is executed

    { IRP_MJ_WRITE,
      0,
      AvPreWrite,
      NULL },

      { IRP_MJ_READ,
      0,
      AvPreRead,
      NULL },

      { IRP_MJ_SET_INFORMATION,
      0,
      AvPreSetInformation,
      NULL },

    { IRP_MJ_OPERATION_END }
};




const FLT_REGISTRATION FilterRegistration = {

    sizeof(FLT_REGISTRATION),         //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                //  Context
    Callbacks,                          //  Operation callbacks

    CgUnload,                           //  MiniFilterUnload

    NULL,                    //  InstanceSetup
    NULL,            //  InstanceQueryTeardown
    NULL,            //  InstanceTeardownStart
    NULL,         //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL,                               //  NormalizeNameComponent
    NULL           //  KTM notification callback

};

/* ======================================================
*                       Main
*  ======================================================*/


// Entry Point of the Driver
NTSTATUS
DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
) {
    NTSTATUS status;

    // Delete the warning for unused parameter
    UNREFERENCED_PARAMETER(RegistryPath);

    KdPrint(("%s", "Driver entered main function"));

    //  Register the filter with it's callbacks and context
    status = FltRegisterFilter(
        DriverObject,
        &FilterRegistration,
        &gFilterInstance
    );

    // If Success
    if (NT_SUCCESS(status)) {

        //  Start filtering
        status = FltStartFiltering(gFilterInstance);

        if (!NT_SUCCESS(status)) {
            FltUnregisterFilter(gFilterInstance);
        }
    }

    return status;
}

/* ======================================================
*                       Driver Methods
*  ======================================================*/

NTSTATUS CgUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {
    UNREFERENCED_PARAMETER(Flags);

    KdPrint(("%s", "Driver unload"));
    FltUnregisterFilter(gFilterInstance);
    return STATUS_SUCCESS;
}

FLT_PREOP_CALLBACK_STATUS AvPreCreate(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    WCHAR Name[256] = { 0 };
    WCHAR Ext[256] = { 0 };

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(FileNameInfo);

        if (NT_SUCCESS(status)) {

            RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength < 255 ? FileNameInfo->Name.MaximumLength : 255);
            RtlCopyMemory(Ext, FileNameInfo->Extension.Buffer, FileNameInfo->Extension.MaximumLength < 255 ? FileNameInfo->Extension.MaximumLength : 255);

            ULONG Disposition = (Data->Iopb->Parameters.Create.Options >> 24) & 0xFF;

            if (wcsstr(Ext, L"txt") != NULL) {
                if (Disposition == FILE_OPEN) {
                    if (Data->Iopb->Parameters.Create.Options & FILE_DELETE_ON_CLOSE) {
                        KdPrint(("[DELETE] file: %ws\n", Name));
                    }
                    else {
                        KdPrint(("Open file: %ws\n", Name));
                    }
                    
                }
                else if (Disposition == FILE_CREATE) {
                    KdPrint(("Create file: %ws\n", Name));
                }
                else if (Disposition == FILE_SUPERSEDE) {
                    KdPrint(("Replace or Create file: %ws\n", Name));
                }
                else if (Disposition == FILE_OPEN_IF) {
                    KdPrint(("Open or (Create + Open) file: %ws\n", Name));
                }
                else if (Disposition == FILE_OVERWRITE) {
                    KdPrint(("Overwrite file: %ws\n", Name));
                }
                else if (Disposition == FILE_OVERWRITE_IF) {
                    KdPrint(("Overwrite or Create file: %ws\n", Name));
                }

            }
        }

        FltReleaseFileNameInformation(FileNameInfo);
        
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS AvPreRead(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    WCHAR Name[256] = { 0 };
    WCHAR Ext[256] = { 0 };

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(FileNameInfo);

        if (NT_SUCCESS(status)) {

            RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength < 255 ? FileNameInfo->Name.MaximumLength : 255);
            RtlCopyMemory(Ext, FileNameInfo->Extension.Buffer, FileNameInfo->Extension.MaximumLength < 255 ? FileNameInfo->Extension.MaximumLength : 255);


            ULONG process = FltGetRequestorProcessId(Data);

            
            //if (wcsstr(FileNameInfo->Extension.Buffer, L"txt") == NULL) {

            if (wcsstr(Ext, L"txt") != NULL) {
                if (process != 0) {
                    KdPrint(("[%lu] Read file: %ws\n", process, Name));
                }
                else {
                    KdPrint(("Read file: %ws", Name));
                }
            }

            //}
            //else {
            //    Data->IoStatus.Status = STATUS_INVALID_PARAMETER;
            //    Data->IoStatus.Information = 0;
            //    FltReleaseFileNameInformation(FileNameInfo);

            //    return FLT_PREOP_COMPLETE;
            //}

            
        }

        FltReleaseFileNameInformation(FileNameInfo);
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS AvPreWrite(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    WCHAR Name[256] = { 0 };
    WCHAR Ext[256] = { 0 };

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(FileNameInfo);
        
        if (NT_SUCCESS(status)) {

            RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength < 255 ? FileNameInfo->Name.MaximumLength : 255);
            RtlCopyMemory(Ext, FileNameInfo->Extension.Buffer, FileNameInfo->Extension.MaximumLength < 255 ? FileNameInfo->Extension.MaximumLength : 255);
            

            ULONG process = FltGetRequestorProcessId(Data);

            if (wcsstr(Ext, L"txt") != NULL) {
                if (process != 0) {
                    KdPrint(("[%lu] Write file: %ws\n", process, Name));
                }
                else {
                    KdPrint(("Write file: %ws", Name));
                }
            }

            /*Data->IoStatus.Status = STATUS_INVALID_PARAMETER;
            Data->IoStatus.Information = NULL;
            FltReleaseFileNameInformation(FileNameInfo);
            
            return FLT_PREOP_COMPLETE;*/
        }

        FltReleaseFileNameInformation(FileNameInfo);
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS AvPreSetInformation(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION FileNameInfo;
    WCHAR Name[256] = { 0 };
    WCHAR Ext[256] = { 0 };

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(FileNameInfo);

        if (NT_SUCCESS(status)) {

            RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength < 255 ? FileNameInfo->Name.MaximumLength : 255);
            RtlCopyMemory(Ext, FileNameInfo->Extension.Buffer, FileNameInfo->Extension.MaximumLength < 255 ? FileNameInfo->Extension.MaximumLength : 255);


            if ((Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformation) ||
                (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformationEx)) {

                ULONG flags = ((PFILE_DISPOSITION_INFORMATION_EX) Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->Flags;

                if (FlagOn(flags, FILE_DISPOSITION_DELETE)) {
                    ULONG process = FltGetRequestorProcessId(Data);

                    if (wcsstr(Ext, L"txt") != NULL) {
                        if (process != 0) {
                            KdPrint(("[%lu] Delete file: %ws\n", process, Name));
                        }
                        else {
                            KdPrint(("Delete file: %ws", Name));
                        }
                    }
                }
            }
           



            /*Data->IoStatus.Status = STATUS_INVALID_PARAMETER;
            Data->IoStatus.Information = NULL;
            FltReleaseFileNameInformation(FileNameInfo);

            return FLT_PREOP_COMPLETE;*/
        }

        FltReleaseFileNameInformation(FileNameInfo);
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}
