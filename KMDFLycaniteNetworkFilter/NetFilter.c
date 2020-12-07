#include <ndis.h>
#include <ntddk.h>
#include <wdf.h>
#include <fwpsk.h>

DRIVER_INITIALIZE DriverEntry;
EVT_WDF_DRIVER_DEVICE_ADD KmdfLycaniteNetworkFilterEvtDeviceAdd;

// Variable for the run-time callout identifier
UINT32 CalloutId;

// Injection handle
HANDLE injectionHandle;

WDFDEVICE wdfDevice;

VOID
Unload(
    IN WDFDRIVER Driver
);

NTSTATUS
DriverEntry(
    IN PDRIVER_OBJECT DriverObject,
    IN PUNICODE_STRING RegistryPath
)
{
    NTSTATUS status = STATUS_SUCCESS;
    WDF_DRIVER_CONFIG config;
    PDEVICE_OBJECT deviceObject;

    // Initialize the driver configuration object to register the
    // entry point for the EvtDeviceAdd callback, KmdfHelloWorldEvtDeviceAdd
    WDF_DRIVER_CONFIG_INIT(&config,
        KmdfLycaniteNetworkFilterEvtDeviceAdd
    );


    //config.DriverInitFlags = WdfDriverInitNonPnpDriver;

    //config.EvtDriverUnload = Unload;

    // Finally, create the driver object
    status = WdfDriverCreate(DriverObject,
        RegistryPath,
        WDF_NO_OBJECT_ATTRIBUTES,
        &config,
        WDF_NO_HANDLE
    );

    // Check status
    if (status == STATUS_SUCCESS) {

        // Initialization of the framework device object is complete
        WdfControlFinishInitializing(
            wdfDevice
        );

        // Get the associated WDM device object
        deviceObject = WdfDeviceWdmGetDeviceObject(wdfDevice);
    }
    else {
        DbgPrintEx(0, 0, "This driver isn't really working.");
    }

    DbgPrintEx(0, 0, "This driver is working.");

    return status;
}

// Unload function
VOID
Unload(
    IN WDFDRIVER Driver
)
{
    UNREFERENCED_PARAMETER(Driver);
    NTSTATUS status;

    // Unregister the callout
    status =
        FwpsCalloutUnregisterById0(
            CalloutId
        );

    // Check result
    if (status == STATUS_DEVICE_BUSY)
    {
        // For each data flow that is being processed by the
        // callout that has an associated context, clean up
        // the context and then call FwpsFlowRemoveContext0
        // to remove the context from the data flow.

        // Finish unregistering the callout
        status =
        FwpsCalloutUnregisterById0(
            CalloutId
        );
    }

    // Check status
    if (status != STATUS_SUCCESS)
    {
        // Handle error
    }

    // Delete the framework device object
    WdfObjectDelete(
        wdfDevice
    );

    // Destroy the injection handle
    status =
        FwpsInjectionHandleDestroy0(
            injectionHandle
        );
        
    // Check status
    if (status != STATUS_SUCCESS)
    {
        DbgPrintEx(0, 0, "Error in Unload.");
    }
}

NTSTATUS
KmdfLycaniteNetworkFilterEvtDeviceAdd(
    _In_    WDFDRIVER       Driver,
    _Inout_ PWDFDEVICE_INIT DeviceInit
)
{
    // We're not using the driver object,
    // so we need to mark it as unreferenced
    UNREFERENCED_PARAMETER(Driver);

    NTSTATUS status;

    // Create the device object
    status = WdfDeviceCreate(&DeviceInit,
        WDF_NO_OBJECT_ATTRIBUTES,
        &wdfDevice
    );
    return status;
}