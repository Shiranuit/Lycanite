#include "Driver.h"
#include "Permissions.h"


/* ======================================================
*                       Filter Structs
*  ======================================================*/

PFLT_FILTER gFilterInstance = NULL;
PFLT_PORT port = NULL;
PFLT_PORT clientPort = NULL;

typedef hashmap_map ihashmap;

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

ihashmap *ProcessesInfos = NULL;
ihashmap* Processes = NULL;

struct hashmap_s globalPerm;

UINT64 LPID = 0; // Lycanite PID
UUIDRecycler* uuidRecycler = NULL;



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

UINT8 isRestricted(PFLT_CALLBACK_DATA Data, PFLT_FILE_NAME_INFORMATION filenameInfo, UINT64 permissions) {
    
    UINT64 processId = FltGetRequestorProcessId(Data);

    processInfos* pinfo = NULL;
    if (ihashmap_get(Processes, processId, (any_t*)&pinfo) == MAP_OK) {
        UINT64 len = filenameInfo->Name.Length / sizeof(WCHAR);
        PWCHAR filename = (PWCHAR)calloc(len + 1, sizeof(WCHAR));
        if (filename != NULL) {
            filename[len] = 0;
            
            RtlCopyMemory(filename, filenameInfo->Name.Buffer, filenameInfo->Name.Length);

            UINT64 gperm = getFilePermission(filename, &globalPerm);
            if (gperm == 0) {
                UINT64 perm = getFilePermission(filename, &pinfo->permissions);
                KdPrint(("Check %llu: [%llu: %d] %ws", processId, permissions, PermFlag(perm, permissions), filename));
                free(filename);
                return PermFlag(perm, permissions);

            }
            KdPrint(("Check Global %llu: [%llu: %d] %ws", processId, permissions, PermFlag(gperm, permissions), filename));
            free(filename);
            return PermFlag(gperm, permissions);
        }
        return 1;
    }
    return 0;
}

INT8 cleanupHashmap(PVOID const context, struct hashmap_element_s * const e) {
    UNREFERENCED_PARAMETER(context);

    free(e->key);
    free(e->data);
    return -1;
}

INT cleanIHashmap(any_t item, any_t data) {
    processInfos* pinfo = (processInfos*)data;
    if (0 != hashmap_iterate_pairs(&pinfo->permissions, cleanupHashmap, NULL)) {
        KdPrint(("%s\n", "failed to deallocate hashmap entries\n"));
    }
    hashmap_destroy(&pinfo->permissions);
    free(data);
    return MAP_OK;
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

    uuidRecycler = UUIDRecycler_create(16);

    if (uuidRecycler == NULL) {
        KdPrint(("%s", "Failed to alloc UUIDRecycler"));
        return STATUS_ABANDONED;
    }

    ProcessesInfos = (ihashmap*)ihashmap_new();
    if (ProcessesInfos == NULL) {
        KdPrint(("%s", "Failed to alloc ProcessesInfos map"));
        return STATUS_ABANDONED;
    }

    Processes = (ihashmap*)ihashmap_new();
    if (Processes == NULL) {
        KdPrint(("%s", "Failed to alloc Processes map"));
        ihashmap_free(ProcessesInfos);
        return STATUS_ABANDONED;
    }

    if (hashmap_create(8, &globalPerm) != 0) {
        KdPrint(("%s", "Failed to alloc globalPerm map"));
        ihashmap_free(ProcessesInfos);
        ihashmap_free(Processes);
        return STATUS_ABANDONED;
    }

    status = PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)CreateProcessNotify, FALSE);
    if (!NT_SUCCESS(status))
    {
        KdPrint(("Faild to PsSetCreateProcessNotifyRoutineEx .status : 0x%X\n", status));
        ihashmap_free(ProcessesInfos);
        ihashmap_free(Processes);
        hashmap_destroy(&globalPerm);
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
        ihashmap_free(ProcessesInfos);
        ihashmap_free(Processes);
        hashmap_destroy(&globalPerm);
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

    ihashmap_free(ProcessesInfos);
    ihashmap_free(Processes);
    hashmap_destroy(&globalPerm);
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

    ihashmap_iterate(ProcessesInfos, cleanIHashmap, NULL);
    ihashmap_free(ProcessesInfos);
    ihashmap_free(Processes);

    if (0 != hashmap_iterate_pairs(&globalPerm, cleanupHashmap, NULL)) {
        KdPrint(("%s\n", "failed to deallocate hashmap entries\n"));
    }
    hashmap_destroy(&globalPerm);

    PsSetCreateProcessNotifyRoutine((PCREATE_PROCESS_NOTIFY_ROUTINE)CreateProcessNotify, TRUE);
    UUIDRecycler_destroy(uuidRecycler);
    return STATUS_SUCCESS;
}

FLT_PREOP_CALLBACK_STATUS AvPreCreate(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION FileNameInfo;

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);
    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(FileNameInfo);
        if (NT_SUCCESS(status)) {
            ULONG Disposition = (Data->Iopb->Parameters.Create.Options >> 24) & 0xFF;
            if (Disposition == FILE_OPEN) {
                if (Data->Iopb->Parameters.Create.Options & FILE_DELETE_ON_CLOSE) {
                    if (isRestricted(Data, FileNameInfo, LYCANITE_DELETE)) {
                        FltReleaseFileNameInformation(FileNameInfo);
                        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                        Data->IoStatus.Information = 0;

                        return FLT_PREOP_COMPLETE;
                    }
                }
                else {
                    if (isRestricted(Data, FileNameInfo, LYCANITE_READ)) {
                        FltReleaseFileNameInformation(FileNameInfo);
                        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                        Data->IoStatus.Information = 0;

                        return FLT_PREOP_COMPLETE;
                    }
                }
            }
            else {
                if (isRestricted(Data, FileNameInfo, LYCANITE_WRITE)) {
                    FltReleaseFileNameInformation(FileNameInfo);
                    Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                    Data->IoStatus.Information = 0;

                    return FLT_PREOP_COMPLETE;
                }
            }
            FltReleaseFileNameInformation(FileNameInfo);
        }
    }
    

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS AvPreRead(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION FileNameInfo;

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(FileNameInfo);
        if (NT_SUCCESS(status)) {
            if (isRestricted(Data, FileNameInfo, LYCANITE_READ)) {
                FltReleaseFileNameInformation(FileNameInfo);
                Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                Data->IoStatus.Information = 0;

                return FLT_PREOP_COMPLETE;
            }
            FltReleaseFileNameInformation(FileNameInfo);
        }
    }


    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS AvPreWrite(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION FileNameInfo;

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(FileNameInfo);
        if (NT_SUCCESS(status)) {
            if (isRestricted(Data, FileNameInfo, LYCANITE_WRITE)) {
                FltReleaseFileNameInformation(FileNameInfo);
                Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                Data->IoStatus.Information = 0;

                return FLT_PREOP_COMPLETE;
            }
            FltReleaseFileNameInformation(FileNameInfo);
        }
    }

    return FLT_PREOP_SUCCESS_NO_CALLBACK;
}

FLT_PREOP_CALLBACK_STATUS AvPreSetInformation(_Inout_ PFLT_CALLBACK_DATA Data, _In_ PCFLT_RELATED_OBJECTS FltObjects, _Flt_CompletionContext_Outptr_ PVOID* CompletionContext) {
    UNREFERENCED_PARAMETER(FltObjects);
    UNREFERENCED_PARAMETER(CompletionContext);

    NTSTATUS status;
    PFLT_FILE_NAME_INFORMATION FileNameInfo;

    status = FltGetFileNameInformation(Data, FLT_FILE_NAME_NORMALIZED | FLT_FILE_NAME_QUERY_DEFAULT, &FileNameInfo);

    if (NT_SUCCESS(status)) {
        status = FltParseFileNameInformation(FileNameInfo);
        if (NT_SUCCESS(status)) {

            if ((Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformation) ||
                (Data->Iopb->Parameters.SetFileInformation.FileInformationClass == FileDispositionInformationEx)) {

                ULONG flags = ((PFILE_DISPOSITION_INFORMATION_EX)Data->Iopb->Parameters.SetFileInformation.InfoBuffer)->Flags;

                if (FlagOn(flags, FILE_DISPOSITION_DELETE)) {
                    if (isRestricted(Data, FileNameInfo, LYCANITE_DELETE)) {
                        FltReleaseFileNameInformation(FileNameInfo);
                        Data->IoStatus.Status = STATUS_ACCESS_DENIED;
                        Data->IoStatus.Information = 0;

                        return FLT_PREOP_COMPLETE;
                    }
                }
            }

            FltReleaseFileNameInformation(FileNameInfo);
        }
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

UINT8
comSetLycanitePid(
    _In_ unsigned char* Input,
    _In_ ULONG InputBufferSize
) {
    if (InputBufferSize != 9) {
        return INVALID_REQUEST_SIZE;
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

    LPID = pid;
    return STATUS_SUCCESS;
}

UINT8
comSetAuthorizationPid(
    _In_ unsigned char* Input,
    _In_ UINT64 InputBufferSize
) {
    if (InputBufferSize <= 18) {
        return INVALID_REQUEST_SIZE;
    }

    UINT16 len = 0;
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

    len |= (((UINT16)Input[17]) & 0xFF);
    len |= (((UINT16)Input[18]) & 0xFF) << 8L;


    char* Message = (char*)calloc(len+1, sizeof(char));

    if (Message == NULL) {
        return BAD_ALLOC;
    }

    Message[len] = 0;
    RtlCopyMemory(Message, (PCHAR)Input + 19, len);

    processInfos* pinfo = NULL;
    if (ihashmap_get(ProcessesInfos, pid, &pinfo) == MAP_OK) {
        PWCHAR file = wcharFromChar(Message, len, &len);
        free(Message);

        if (file == NULL) {
            return BAD_ALLOC;
        }

        UINT64 *perm_ptr = (UINT64 *)hashmap_get(&pinfo->permissions, file, len);

        if (perm_ptr != NULL) {
            KdPrint(("Set PID Auth [%llu] %ws\n", len, file));
            free(file);
            *perm_ptr = perms;
        } else {
            perm_ptr = (UINT64*)calloc(1, sizeof(UINT64));
            if (perm_ptr == NULL) {
                free(file);
                return BAD_ALLOC;
            }

            *perm_ptr = perms;
            hashmap_put(&pinfo->permissions, file, len, perm_ptr);
            KdPrint(("Set PID Auth [%llu] %ws\n", len, file));
        }

        
        return STATUS_SUCCESS;
    }
    free(Message);

    return STATUS_SUCCESS;
}

UINT8
comSetAuthorizationGlobal(
    _In_ unsigned char* Input,
    _In_ UINT64 InputBufferSize
) {
    if (InputBufferSize <= 10) {
        return INVALID_REQUEST_SIZE;
    }

    UINT16 len = 0;
    UINT64 perms = 0;

    perms |= (((UINT64)Input[1]) & 0xFF);
    perms |= (((UINT64)Input[2]) & 0xFF) << 8L;
    perms |= (((UINT64)Input[3]) & 0xFF) << 16L;
    perms |= (((UINT64)Input[4]) & 0xFF) << 24L;
    perms |= (((UINT64)Input[5]) & 0xFF) << 32L;
    perms |= (((UINT64)Input[6]) & 0xFF) << 40L;
    perms |= (((UINT64)Input[7]) & 0xFF) << 48L;
    perms |= (((UINT64)Input[8]) & 0xFF) << 56L;

    len |= (((UINT16)Input[9]) & 0xFF);
    len |= (((UINT16)Input[10]) & 0xFF) << 8L;

    char* Message = (char*)calloc(len + 1, sizeof(char));

    if (Message == NULL) {
        return BAD_ALLOC;
    }

    Message[len] = 0;
    RtlCopyMemory(Message, (PCHAR)Input + 11, len);

    PWCHAR file = wcharFromChar(Message, len, &len);
    free(Message);

    if (file == NULL) {
        return BAD_ALLOC;
    }

    UINT64* perm_ptr = (UINT64*)hashmap_get(&globalPerm, file, len);
    if (perm_ptr != NULL) {
        KdPrint(("Set Global Auth [%llu] %ws\n", len, file));
        free(file);
        *perm_ptr = perms;
    } else {
        perm_ptr = (UINT64*)calloc(1, sizeof(UINT64));
        if (perm_ptr == NULL) {
            free(file);
            return BAD_ALLOC;
        }

        *perm_ptr = perms;
        hashmap_put(&globalPerm, file, len, perm_ptr);
        KdPrint(("Set Global Auth [%llu] %ws\n", len, file));
    }

    return STATUS_SUCCESS;
}

UINT8
comGetProcessStats(
    _Out_ unsigned char* Output,
    _In_ UINT64 OutputBufferSize,
    _Out_ PULONG ReturnOutputBufferLength
) {
    //TODO : fill Output with data
}

UINT8
comDeleteAuthorizationPid(
    _In_ unsigned char* Input,
    _In_ UINT64 InputBufferSize
) {
    if (InputBufferSize <= 10) {
        return INVALID_REQUEST_SIZE;
    }

    UINT16 len = 0;
    UINT64 pid = 0;

    pid |= (((UINT64)Input[1]) & 0xFF);
    pid |= (((UINT64)Input[2]) & 0xFF) << 8L;
    pid |= (((UINT64)Input[3]) & 0xFF) << 16L;
    pid |= (((UINT64)Input[4]) & 0xFF) << 24L;
    pid |= (((UINT64)Input[5]) & 0xFF) << 32L;
    pid |= (((UINT64)Input[6]) & 0xFF) << 40L;
    pid |= (((UINT64)Input[7]) & 0xFF) << 48L;
    pid |= (((UINT64)Input[8]) & 0xFF) << 56L;

    len |= (((UINT16)Input[9]) & 0xFF);
    len |= (((UINT16)Input[10]) & 0xFF) << 8L;

    char* Message = (char*)calloc(len + 1, sizeof(char));

    if (Message == NULL) {
        return BAD_ALLOC;
    }
    Message[len] = 0;

    RtlCopyMemory(Message, (PCHAR)Input + 11, len);

    processInfos* pinfo = NULL;
    if (ihashmap_get(ProcessesInfos, pid, &pinfo) == MAP_OK) {

        PWCHAR file = wcharFromChar(Message, len, &len);
        free(Message);

        if (file == NULL) {
            return BAD_ALLOC;
        }

        PWCHAR* key = NULL;
        UINT64* perms = NULL;

        if (hashmap_remove(&pinfo->permissions, file, len, &perms, &key) == 0) {
            free(perms);
            free(key);
        }

        KdPrint(("Delete PID Auth [%d] %ws\n", len, file));
        free(file);
        return STATUS_SUCCESS;
    }

    free(Message);

    return STATUS_SUCCESS;
}

UINT8
comDeleteAuthorizationGlobal(
    _In_ unsigned char* Input,
    _In_ UINT64 InputBufferSize
) {
    if (InputBufferSize <= 2) {
        return INVALID_REQUEST_SIZE;
    }

    UINT16 len = 0;

    len |= (((UINT16)Input[1]) & 0xFF);
    len |= (((UINT16)Input[2]) & 0xFF) << 8L;

    char* Message = (char*)calloc(len + 1, sizeof(char));

    if (Message == NULL) {
        return BAD_ALLOC;
    }

    Message[len] = 0;
    RtlCopyMemory(Message, (PCHAR)Input + 19, len);

    PWCHAR file = wcharFromChar(Message, len, &len);
    free(Message);

    if (file == NULL) {
        return BAD_ALLOC;
    }

    PWCHAR* key = NULL;
    UINT64* perms = NULL;

    if (hashmap_remove(&globalPerm, file, len, &perms, &key) == 0) {
        free(perms);
        free(key);
    }

    KdPrint(("Delete Global Auth [%d] %ws\n", len, file));
    free(file);

    return STATUS_SUCCESS;
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
        case SET_LYCANITE_PID:
            status = comSetLycanitePid(Input, InputBufferSize);
            break;
        case SET_AUTHORIZATION_PID:
            status = comSetAuthorizationPid(Input, InputBufferSize);
            break;
        case SET_AUTHORIZATION_GLOBAL:
            status = comSetAuthorizationGlobal(Input, InputBufferSize);
            break;
        case GET_PROCESS_STATS:
            status = comGetProcessStats(Output, OutputBufferSize, ReturnOutputBufferLength);
            break;
        case DELETE_AUTHORIZATION_PID:
            status = comDeleteAuthorizationPid(Input, InputBufferSize);
            break;
        case DELETE_AUTHORIZATION_GLOBAL:
            status = comDeleteAuthorizationGlobal(Input, InputBufferSize);
            break;
        }

        if (status == INVALID_REQUEST_SIZE) {
            DbgPrint(("request size invalid\n"));
        }
        else if (status == UNKNOWN_REQUEST) {
            DbgPrint(("request unknow\n"));
        }
        else if (status == BAD_ALLOC) {
            DbgPrint(("bad allocation\n"));
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
                UUID uuid = UUIDRecycler_getUUID(uuidRecycler);

                if (uuid == 0) { // Fail
                    return;
                }

                processInfos* pinfo = (processInfos *)calloc(1, sizeof(processInfos));

                if (pinfo == NULL) {
                    return;
                }

                pinfo->referenceCount = 1;
                pinfo->uuid = uuid;
                if (hashmap_create(8, &pinfo->permissions)) { // Failed to alloc hashmap
                    free(pinfo);
                    return;
                }

                ihashmap_put(ProcessesInfos, uuid, pinfo);
                ihashmap_put(Processes, ProcessId, pinfo);

                char* sendBuff = (char *)calloc(17, sizeof(char));

                if (sendBuff != NULL) {

                    sendBuff[0] = PROCESS_CREATE;

                    sendBuff[1] = (char)(ProcessId & 0xFF);
                    sendBuff[2] = (char)((ProcessId >> 8) & 0xFF);
                    sendBuff[3] = (char)((ProcessId >> 16) & 0xFF);
                    sendBuff[4] = (char)((ProcessId >> 24) & 0xFF);
                    sendBuff[5] = (char)((ProcessId >> 32) & 0xFF);
                    sendBuff[6] = (char)((ProcessId >> 40) & 0xFF);
                    sendBuff[7] = (char)((ProcessId >> 48) & 0xFF);
                    sendBuff[8] = (char)((ProcessId >> 56) & 0xFF);

                    sendBuff[9] = (char)(uuid & 0xFF);
                    sendBuff[10] = (char)((uuid >> 8) & 0xFF);
                    sendBuff[11] = (char)((uuid >> 16) & 0xFF);
                    sendBuff[12] = (char)((uuid >> 24) & 0xFF);
                    sendBuff[13] = (char)((uuid >> 32) & 0xFF);
                    sendBuff[14] = (char)((uuid >> 40) & 0xFF);
                    sendBuff[15] = (char)((uuid >> 48) & 0xFF);
                    sendBuff[16] = (char)((uuid >> 56) & 0xFF);

                    LARGE_INTEGER time;
                    time.QuadPart = -10000000;
                    ULONG byterec = 0;
                    NTSTATUS status = FltSendMessage(gFilterInstance, &clientPort, sendBuff, 17, NULL, &byterec, &time);

                    if (status == STATUS_TIMEOUT) {
                        KdPrint(("Failed to deliver message: Timedout"));
                    }
                    else if (!NT_SUCCESS(status)) {

                    }

                    free(sendBuff);
                }
                KdPrint(("Create Parent Process [%llu] -- %llu\n", ProcessId, uuid));
            }
            else {
                processInfos *pinfo;
                if (ihashmap_get(Processes, ParentId, (any_t*)&pinfo) == MAP_OK) {
                    pinfo->referenceCount += 1;
                    ihashmap_put(Processes, ProcessId, pinfo);
                    KdPrint(("Create Sub Process [%llu] %llu -- %llu\n", ParentId, ProcessId, pinfo->uuid));
                }
            }
        } else { // on process killed
            processInfos* pinfo = NULL;
            if (ihashmap_get(Processes, ProcessId, (any_t*)&pinfo) == MAP_OK) {
                pinfo->referenceCount -= 1;

                KdPrint(("Kill Process %llu -- %llu\n", ProcessId, pinfo->uuid));
                if (pinfo->referenceCount == 0) {
                    char* sendBuff = (char*)calloc(9, sizeof(char));

                    if (sendBuff != NULL) {

                        sendBuff[0] = PROCESS_DESTROY;

                        sendBuff[1] = (char)(pinfo->uuid & 0xFF);
                        sendBuff[2] = (char)((pinfo->uuid >> 8) & 0xFF);
                        sendBuff[3] = (char)((pinfo->uuid >> 16) & 0xFF);
                        sendBuff[4] = (char)((pinfo->uuid >> 24) & 0xFF);
                        sendBuff[5] = (char)((pinfo->uuid >> 32) & 0xFF);
                        sendBuff[6] = (char)((pinfo->uuid >> 40) & 0xFF);
                        sendBuff[7] = (char)((pinfo->uuid >> 48) & 0xFF);
                        sendBuff[8] = (char)((pinfo->uuid >> 56) & 0xFF);

                        LARGE_INTEGER time;
                        time.QuadPart = -10000000;
                        ULONG byterec = 0;
                        NTSTATUS status = FltSendMessage(gFilterInstance, &clientPort, sendBuff, 9, NULL, &byterec, &time);

                        if (status == STATUS_TIMEOUT) {
                            KdPrint(("Failed to deliver message: Timedout"));
                        }
                        else if (!NT_SUCCESS(status)) {

                        }

                        free(sendBuff);
                    }

                    ihashmap_remove(ProcessesInfos, pinfo->uuid, NULL);

                    if (0 != hashmap_iterate_pairs(&pinfo->permissions, cleanupHashmap, NULL)) {
                        KdPrint(("%s\n", "failed to deallocate hashmap entries\n"));
                    }
                    hashmap_destroy(&pinfo->permissions);

                    UUIDRecycler_recycleUUID(uuidRecycler, pinfo->uuid);
                    KdPrint(("Remove Reference %llu\n", pinfo->uuid));
                    free(pinfo);
                }

                ihashmap_remove(Processes, ProcessId, NULL);
            }
        }
    }
}