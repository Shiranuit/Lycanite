#include "Driver.h"
#include "Kashmap.h"


/* ======================================================
*                       Filter Structs
*  ======================================================*/

PFLT_FILTER gFilterInstance = NULL;
PFLT_PORT port = NULL;
PFLT_PORT clientPort = NULL;

const FLT_OPERATION_REGISTRATION Callbacks[] = {
    { IRP_MJ_CREATE, // Filter Action Name
      0,
      NULL,  // Pointer to the function to execute before the action is executed
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

    sizeof(FLT_REGISTRATION),           //  Size
    FLT_REGISTRATION_VERSION,           //  Version
    0,                                  //  Flags

    NULL,                               //  Context
    Callbacks,                          //  Operation callbacks

    CgUnload,                           //  MiniFilterUnload

    NULL,                               //  InstanceSetup
    NULL,                               //  InstanceQueryTeardown
    NULL,                               //  InstanceTeardownStart
    NULL,                               //  InstanceTeardownComplete

    NULL,                               //  GenerateFileName
    NULL,                               //  GenerateDestinationFileName
    NULL,                               //  NormalizeNameComponent
    NULL                                //  KTM notification callback

};

struct hashmap_s *ProcessInfos;
UINT64 LPID = 0; // Lycanite PID


/* ======================================================
*                       Utils Methods
*  ======================================================*/

UINT8 ulong2chr(UINT64 value, char* str) {
    UINT8 pos = 0;
    while (value != 0) {
        str[pos++] = (value % 10) + '0';
        value /= 10;
    }

    for (UINT8 i = 0; i < pos / 2; i++) {
        char tmp = str[i];
        UINT8 backIndex = pos - i - 1;
        str[i] = str[backIndex];
        str[backIndex] = tmp;
    }
    return pos;
}

UINT8 isRestricted(PFLT_CALLBACK_DATA Data, UINT64 permissions) {
    char PID[32] = { 0 };
    ULONG process = FltGetRequestorProcessId(Data);
    UINT8 len = ulong2chr(process, PID);
    ProcessPerm *perm = (ProcessPerm *)hashmap_get(ProcessInfos, PID, len);

    if (perm != NULL) {
        if (perm->leaf) {
            return PermFlag(perm->ptr->permissions, permissions);
        }
        else {
            return PermFlag(perm->permissions, permissions);
        }
    }

    return 0;
}

INT8 cleanupHashmap(PVOID const context, struct hashmap_element_s * const e) {
    UNREFERENCED_PARAMETER(context);

    free(e->key);
    free(e->data);
    return -1;
}

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
    PSECURITY_DESCRIPTOR securityDescriptor;
    OBJECT_ATTRIBUTES objectAttributes = { 0 };
    UNICODE_STRING name = RTL_CONSTANT_STRING(L"\\LycaniteFF");

    ProcessInfos = (struct hashmap_s *)calloc(1, sizeof(struct hashmap_s));
    if (ProcessInfos == NULL) {
        KdPrint(("%s", "Failed to alloc hashmap"));
        return STATUS_ABANDONED;
    }

    if (hashmap_create(8, ProcessInfos) != 0) {
        KdPrint(("%s", "Failed to initialize hashmap of processInfos"));
        return STATUS_ABANDONED;
    }

    status = PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)CreateProcessNotify, FALSE);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("Faild to PsSetCreateProcessNotifyRoutineEx .status : 0x%X\n", status));
        return STATUS_ABANDONED;
    }

    // Delete the warning for unused parameter
    UNREFERENCED_PARAMETER(RegistryPath);

    KdPrint(("%s", "Driver entered main function"));

    //  Register the filter with it's callbacks and context
    status = FltRegisterFilter(
        DriverObject,
        &FilterRegistration,
        &gFilterInstance
    );

    if (!NT_SUCCESS(status)) {
        return status;
    }


    status = FltBuildDefaultSecurityDescriptor(&securityDescriptor, FLT_PORT_ALL_ACCESS);

    // If Success
    if (NT_SUCCESS(status)) {

        InitializeObjectAttributes(&objectAttributes, &name, OBJ_KERNEL_HANDLE | OBJ_CASE_INSENSITIVE, NULL, securityDescriptor);

        status = FltCreateCommunicationPort(
            gFilterInstance,
            &port,
            &objectAttributes,
            NULL, 
            comConnectNotifyCallback,
            comDisconnectNotifyCallback,
            comMessageNotifyCallback,
            1
        );

        FltFreeSecurityDescriptor(securityDescriptor);
        
        if (NT_SUCCESS(status)) {
            status = FltStartFiltering(gFilterInstance);

            if (NT_SUCCESS(status)) {
                return status;
            }
            KdPrint(("[ERROR] FltStartFiltering FAILED. status = 0x%x\n", status));
        }
        KdPrint(("[ERROR] FltCreateCommunicationPort FAILED. status = 0x%x\n", status));

        FltCloseCommunicationPort(port);

    }

    FltUnregisterFilter(gFilterInstance);
    KdPrint(("[ERROR] FltBuildDefaultSecurityDescriptor FAILED. status = 0x%x\n", status));

    return status;
}

/* ======================================================
*                       Driver Methods
*  ======================================================*/

NTSTATUS CgUnload(FLT_FILTER_UNLOAD_FLAGS Flags) {
    UNREFERENCED_PARAMETER(Flags);

    KdPrint(("%s", "Driver unload"));
    FltCloseCommunicationPort(port);
    FltUnregisterFilter(gFilterInstance);
    if (0 != hashmap_iterate_pairs(ProcessInfos, cleanupHashmap, NULL)) {
        KdPrint(("%s\n", "failed to deallocate hashmap entries\n"));
    }
    hashmap_destroy(ProcessInfos);
    free(ProcessInfos);

    PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)CreateProcessNotify, TRUE);

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

    if (isRestricted(Data, LYCANITE_WRITE)) {
        if (NT_SUCCESS(status)) {
            status = FltParseFileNameInformation(FileNameInfo);

            if (NT_SUCCESS(status)) {

                RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength < 255 ? FileNameInfo->Name.MaximumLength : 255);
                RtlCopyMemory(Ext, FileNameInfo->Extension.Buffer, FileNameInfo->Extension.MaximumLength < 255 ? FileNameInfo->Extension.MaximumLength : 255);

                ULONG Disposition = (Data->Iopb->Parameters.Create.Options >> 24) & 0xFF;

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

            FltReleaseFileNameInformation(FileNameInfo);
            }
        }

        // Reject Operation
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        Data->IoStatus.Information = 0;

        return FLT_PREOP_COMPLETE;
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

    if (isRestricted(Data, LYCANITE_READ)) {

        if (NT_SUCCESS(status)) {
            status = FltParseFileNameInformation(FileNameInfo);

            if (NT_SUCCESS(status)) {

                RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength < 255 ? FileNameInfo->Name.MaximumLength : 255);
                RtlCopyMemory(Ext, FileNameInfo->Extension.Buffer, FileNameInfo->Extension.MaximumLength < 255 ? FileNameInfo->Extension.MaximumLength : 255);


                ULONG process = FltGetRequestorProcessId(Data);


                //if (wcsstr(FileNameInfo->Extension.Buffer, L"txt") == NULL) {


                if (process != 0) {
                    KdPrint(("[%lu] Read file: %ws\n", process, Name));
                }
                else {
                    KdPrint(("Read file: %ws", Name));
                }

                FltReleaseFileNameInformation(FileNameInfo);
            }

        }

        // Reject Operation
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        Data->IoStatus.Information = 0;

        return FLT_PREOP_COMPLETE;
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

    if (isRestricted(Data, LYCANITE_WRITE)) {
        status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

        if (NT_SUCCESS(status)) {
            status = FltParseFileNameInformation(FileNameInfo);

            if (NT_SUCCESS(status)) {

                RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength < 255 ? FileNameInfo->Name.MaximumLength : 255);
                RtlCopyMemory(Ext, FileNameInfo->Extension.Buffer, FileNameInfo->Extension.MaximumLength < 255 ? FileNameInfo->Extension.MaximumLength : 255);


                ULONG process = FltGetRequestorProcessId(Data);

                if (process != 0) {
                    KdPrint(("[%lu] Write file: %ws\n", process, Name));
                }
                else {
                    KdPrint(("Write file: %ws", Name));
                }

                FltReleaseFileNameInformation(FileNameInfo);
            }

        }

        // Reject Operation
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        Data->IoStatus.Information = 0;

        return FLT_PREOP_COMPLETE;
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

    if (isRestricted(Data, LYCANITE_WRITE)) {

        status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

        if (NT_SUCCESS(status)) {
            status = FltParseFileNameInformation(FileNameInfo);

            if (NT_SUCCESS(status)) {

                RtlCopyMemory(Name, FileNameInfo->Name.Buffer, FileNameInfo->Name.MaximumLength < 255 ? FileNameInfo->Name.MaximumLength : 255);
                RtlCopyMemory(Ext, FileNameInfo->Extension.Buffer, FileNameInfo->Extension.MaximumLength < 255 ? FileNameInfo->Extension.MaximumLength : 255);


                if ((Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformation) ||
                    (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformationEx)) {

                    ULONG flags = ((PFILE_DISPOSITION_INFORMATION_EX)Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->Flags;

                    if (FlagOn(flags, FILE_DISPOSITION_DELETE)) {
                        ULONG process = FltGetRequestorProcessId(Data);
                        if (process != 0) {
                            KdPrint(("[%lu] Delete file: %ws\n", process, Name));
                        }
                        else {
                            KdPrint(("Delete file: %ws", Name));
                        }
                    }
                }

                FltReleaseFileNameInformation(FileNameInfo);
            }

        }

        // Reject Operation
        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
        Data->IoStatus.Information = 0;

        return FLT_PREOP_COMPLETE;
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}


/* ======================================================
*                 Communication Callbacks
*  ======================================================*/

NTSTATUS
comConnectNotifyCallback(
    _In_ PFLT_PORT ClientPort,
    _In_ PVOID ServerPortCookie,
    _In_reads_bytes_(SizeOfContext) PVOID ConnectionContext,
    _In_ ULONG SizeOfContext,
    _Outptr_result_maybenull_ PVOID* ConnectionCookie
) {

    UNREFERENCED_PARAMETER(ServerPortCookie);
    UNREFERENCED_PARAMETER(ConnectionContext);
    UNREFERENCED_PARAMETER(SizeOfContext);
    
    *ConnectionCookie = NULL;

    if (clientPort == NULL) {
        clientPort = ClientPort;
        KdPrint(("Com Connect\n"));
        return STATUS_SUCCESS;
    }
    KdPrint(("Com Connection refused\n"));
    return STATUS_ABANDONED;
}

VOID
comDisconnectNotifyCallback(
    _In_opt_ PVOID ConnectionCookie
) {
    UNREFERENCED_PARAMETER(ConnectionCookie);

    if (clientPort != NULL) {
        KdPrint(("Com Disconnect\n"));
        FltCloseClientPort(gFilterInstance, &clientPort);
    }
}

UINT16
comSetLycanitePid(
    _In_ unsigned char* Input,
    _In_ ULONG InputBufferSize,
    _In_ UINT64* LPID
) {
    if (InputBufferSize != 9) {
        return (INVALID_REQUEST_SIZE);
    }

    UINT64 pid = 0;

    pid |= (((UINT64)Input[1]) & 0xFF);
    pid |= (((UINT64)Input[2]) & 0xFF) << 8L;
    pid |= (((UINT64)Input[3]) & 0xFF) << 16L;
    pid |= (((UINT64)Input[4]) & 0xFF) << 24L;
    pid |= (((UINT64)Input[5]) & 0xFF) << 32L;
    pid |= (((UINT64)Input[6]) & 0xFF) << 40L;
    pid |= (((UINT64)Input[7]) & 0xFF) << 48L;
    pid |= (((UINT64)Input[8]) & 0xFF) << 56L;

    KdPrint(("Set Lycanite PID [%llu]\n", pid));

    *LPID = pid;
    return (STATUS_SUCCESS);
}

UINT16
comSetAuthorizationPid(
    _In_ unsigned char* Input,
    _In_ UINT64 InputBufferSize
) {
    if (InputBufferSize <= 20) {
        return (INVALID_REQUEST_SIZE);
    }

    UINT32 len = 0;
    UINT64 pid = 0;
    UINT64 perms = 0;

    pid |= (((UINT64)Input[1]) & 0xFF);
    pid |= (((UINT64)Input[2]) & 0xFF) << 8L;
    pid |= (((UINT64)Input[3]) & 0xFF) << 16L;
    pid |= (((UINT64)Input[4]) & 0xFF) << 24L;
    pid |= (((UINT64)Input[5]) & 0xFF) << 32L;
    pid |= (((UINT64)Input[6]) & 0xFF) << 40L;
    pid |= (((UINT64)Input[7]) & 0xFF) << 48L;
    pid |= (((UINT64)Input[8]) & 0xFF) << 56L;

    perms |= (((UINT64)Input[9]) & 0xFF);
    perms |= (((UINT64)Input[10]) & 0xFF) << 8L;
    perms |= (((UINT64)Input[11]) & 0xFF) << 16L;
    perms |= (((UINT64)Input[12]) & 0xFF) << 24L;
    perms |= (((UINT64)Input[13]) & 0xFF) << 32L;
    perms |= (((UINT64)Input[14]) & 0xFF) << 40L;
    perms |= (((UINT64)Input[15]) & 0xFF) << 48L;
    perms |= (((UINT64)Input[16]) & 0xFF) << 56L;

    len |= (((UINT32)Input[17]) & 0xFF);
    len |= (((UINT32)Input[18]) & 0xFF) << 8L;
    len |= (((UINT32)Input[19]) & 0xFF) << 16L;
    len |= (((UINT32)Input[20]) & 0xFF) << 24L;

    char* Message = (char*)calloc(len, sizeof(char));
    RtlCopyMemory(Message, (PCHAR)Input+21, len);

    KdPrint(("[%d] %s\n", len, Message));

    return (STATUS_SUCCESS);
}

UINT16
comSetAuthorizationGlobal(
    _In_ unsigned char* Input,
    _In_ UINT64 InputBufferSize
) {
    if (InputBufferSize <= 12) {
        return (INVALID_REQUEST_SIZE);
    }

    UINT32 len = 0;
    UINT64 perms = 0;

    perms |= (((UINT64)Input[1]) & 0xFF);
    perms |= (((UINT64)Input[2]) & 0xFF) << 8L;
    perms |= (((UINT64)Input[3]) & 0xFF) << 16L;
    perms |= (((UINT64)Input[4]) & 0xFF) << 24L;
    perms |= (((UINT64)Input[5]) & 0xFF) << 32L;
    perms |= (((UINT64)Input[6]) & 0xFF) << 40L;
    perms |= (((UINT64)Input[7]) & 0xFF) << 48L;
    perms |= (((UINT64)Input[8]) & 0xFF) << 56L;

    len |= (((UINT32)Input[9]) & 0xFF);
    len |= (((UINT32)Input[10]) & 0xFF) << 8L;
    len |= (((UINT32)Input[11]) & 0xFF) << 16L;
    len |= (((UINT32)Input[12]) & 0xFF) << 24L;

    char* Message = (char*)calloc(len, sizeof(char));
    RtlCopyMemory(Message, (PCHAR)Input + 13, len);

    KdPrint(("[%d] %s\n", len, Message));

    return (STATUS_SUCCESS);
}

UINT16
comGetProcessStats(
    _In_ unsigned char* Output,
    _In_ UINT64 OutputBufferSize
) {

}

UINT16
comDeleteAuthorizationPid(
    _In_ unsigned char* Input,
    _In_ UINT64 InputBufferSize
) {
    if (InputBufferSize <= 12) {
        return (INVALID_REQUEST_SIZE);
    }

    UINT32 len = 0;
    UINT64 pid = 0;

    pid |= (((UINT64)Input[1]) & 0xFF);
    pid |= (((UINT64)Input[2]) & 0xFF) << 8L;
    pid |= (((UINT64)Input[3]) & 0xFF) << 16L;
    pid |= (((UINT64)Input[4]) & 0xFF) << 24L;
    pid |= (((UINT64)Input[5]) & 0xFF) << 32L;
    pid |= (((UINT64)Input[6]) & 0xFF) << 40L;
    pid |= (((UINT64)Input[7]) & 0xFF) << 48L;
    pid |= (((UINT64)Input[8]) & 0xFF) << 56L;

    len |= (((UINT32)Input[9]) & 0xFF);
    len |= (((UINT32)Input[10]) & 0xFF) << 8L;
    len |= (((UINT32)Input[11]) & 0xFF) << 16L;
    len |= (((UINT32)Input[12]) & 0xFF) << 24L;

    char* Message = (char*)calloc(len, sizeof(char));
    RtlCopyMemory(Message, (PCHAR)Input + 13, len);

    KdPrint(("[%d] %s\n", len, Message));

    return (STATUS_SUCCESS);
}

UINT16
comDeleteAuthorizationGlobal(
    _In_ unsigned char* Input,
    _In_ UINT64 InputBufferSize
) {
    if (InputBufferSize <= 4) {
        return (INVALID_REQUEST_SIZE);
    }

    UINT32 len = 0;

    len |= (((UINT32)Input[1]) & 0xFF);
    len |= (((UINT32)Input[2]) & 0xFF) << 8L;
    len |= (((UINT32)Input[3]) & 0xFF) << 16L;
    len |= (((UINT32)Input[4]) & 0xFF) << 24L;

    char* Message = (char*)calloc(len, sizeof(char));
    RtlCopyMemory(Message, (PCHAR)Input + 5, len);

    KdPrint(("[%d] %s\n", len, Message));

    return (STATUS_SUCCESS);
}


NTSTATUS
comMessageNotifyCallback(
    _In_ PVOID ConnectionCookie,
    _In_reads_bytes_opt_(InputBufferSize) PVOID InputBuffer,
    _In_ ULONG InputBufferSize,
    _Out_writes_bytes_to_opt_(OutputBufferSize, *ReturnOutputBufferLength) PVOID OutputBuffer,
    _In_ ULONG OutputBufferSize,
    _Out_ PULONG ReturnOutputBufferLength
) {

    UNREFERENCED_PARAMETER(ConnectionCookie);
    UNREFERENCED_PARAMETER(InputBuffer);
    UNREFERENCED_PARAMETER(InputBufferSize);
    UNREFERENCED_PARAMETER(OutputBuffer);
    UNREFERENCED_PARAMETER(OutputBufferSize);
    
    *ReturnOutputBufferLength = 0;

    UINT16 status = UNKNOWN_REQUEST;

    if (InputBuffer != NULL && InputBufferSize > 0) {
        unsigned char* Input = (unsigned char*)InputBuffer;
        unsigned char* Output = (unsigned char*)OutputBuffer;

        switch (Input[0]) {
            case SET_LYCANITE_PID :
                status = comSetLycanitePid(Input, InputBufferSize, &LPID);
                break;
            case SET_AUTHORIZATION_PID:
                status = comSetAuthorizationPid(Input, InputBufferSize, ProcessInfos);
                break;
            case SET_AUTHORIZATION_GLOBAL:
                status = comSetAuthorizationPid(Input, InputBufferSize, ProcessInfos);
                break;
            case GET_PROCESS_STATS:
                status = comSetAuthorizationPid(Input, InputBufferSize, ProcessInfos);
                break;
            case DELETE_AUTHORIZATION_PID:
                status = comSetAuthorizationPid(Input, InputBufferSize, ProcessInfos);
                break;
            case DELETE_AUTHORIZATION_GLOBAL:
                status = comSetAuthorizationPid(Input, InputBufferSize, ProcessInfos);
                break;
        }
        if (status == INVALID_REQUEST_SIZE) {
            // TODO: Error gestion when the request size is bad
        }
        else if (status == UNKNOWN_REQUEST) {
            // TODO: Error gestion when the request size is bad
        }
    }

    return STATUS_SUCCESS;
}

VOID CreateProcessNotify(
    _In_ HANDLE PParentId,
    _In_ HANDLE PProcessId,
    _In_ BOOLEAN Create
) {
    UINT64 ParentId = (UINT64)PParentId;
    UINT64 ProcessId = (UINT64)PProcessId;
    if (LPID != 0) {
        if (Create) {
            if (LPID == ParentId) {
                char* ID = (char*)calloc(24, sizeof(char));
                UINT8 idlen = ulong2chr(ProcessId, ID);
                ProcessPerm* newperm = (ProcessPerm*)calloc(1, sizeof(ProcessPerm));

                newperm->leaf = 0;
                newperm->permissions = 0;

                hashmap_put(ProcessInfos, ID, idlen, newperm);

                KdPrint(("Create Parent Process [%llu]\n", ProcessId));
            }
            else {
                char PID[32] = { 0 };
                UINT8 len = ulong2chr(ParentId, PID);
                ProcessPerm* perm = (ProcessPerm*)hashmap_get(ProcessInfos, PID, len);

                if (perm != NULL) {
                    char* ID = (char*)calloc(24, sizeof(char));
                    UINT8 idlen = ulong2chr(ProcessId, ID);
                    ProcessPerm* newperm = (ProcessPerm*)calloc(1, sizeof(ProcessPerm));
                    newperm->leaf = 1;
                    if (perm->leaf) {
                        newperm->ptr = perm->ptr;
                    }
                    else {
                        newperm->ptr = perm;
                    }
                    hashmap_put(ProcessInfos, ID, idlen, newperm);
                    KdPrint(("Create Sub Process [%llu] %llu\n", ParentId, ProcessId));
                }
            }
        }
        else {
            char PID[32] = { 0 };
            UINT8 len = ulong2chr(ProcessId, PID);
            ProcessPerm* perm = (ProcessPerm*)hashmap_get(ProcessInfos, PID, len);
            if (perm != NULL) {
                if (perm->leaf) {
                    KdPrint(("Destroy Sub Process [%llu] %llu\n", ParentId, ProcessId));
                }
                else {
                    KdPrint(("Destroy Parent Process %llu\n", ProcessId));
                }
            }
        }
    }
}