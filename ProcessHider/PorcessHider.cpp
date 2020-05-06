#include "pch.h"
#include "Consts.h"
#include "Routines.h"


extern "C" NTSTATUS DriverEntry(_In_ PDRIVER_OBJECT driverObject, _In_ PUNICODE_STRING registryPath) {
	UNREFERENCED_PARAMETER(registryPath);
	driverObject->DriverUnload = processHiderUnload;
	driverObject->MajorFunction[IRP_MJ_CREATE] = processHiderCreateClose;
	driverObject->MajorFunction[IRP_MJ_CLOSE] = processHiderCreateClose;
	driverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = processHiderDeviceControl;

	UNICODE_STRING devName = RTL_CONSTANT_STRING(L"\\Device\\processhider");
	
	PDEVICE_OBJECT deviceObject;
	NTSTATUS status = IoCreateDevice(driverObject,
		0,
		&devName,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&deviceObject);

	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create device object (0x%08X)\n", status));
		return status;
	}

	UNICODE_STRING symlink = RTL_CONSTANT_STRING(L"\\??\\processhider");
	status = IoCreateSymbolicLink(&symlink, &devName);
	if (!NT_SUCCESS(status)) {
		KdPrint(("Failed to create symbolic link (0x%08X)\n", status));
		IoDeleteDevice(deviceObject);
		return status;
	}

	return status;
}