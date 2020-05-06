#pragma once
#include "pch.h"

NTSTATUS processHiderCreateClose(PDEVICE_OBJECT deviceObject, PIRP irp);
void processHiderUnload(_In_ PDRIVER_OBJECT driverObject);
NTSTATUS processHiderDeviceControl(PDEVICE_OBJECT, PIRP irp);