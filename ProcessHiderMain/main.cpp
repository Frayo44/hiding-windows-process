#include <Windows.h>
#include <stdio.h>
#include "..\ProcessHider\PorcessHideCommon.h"

int error(const char* message) {
	printf("%s (error=%d)\n", message, GetLastError());
	return 1;
}

size_t getPid() {
	size_t pid = 0;
	printf("Enter a pid you want to hide (0 for exit): \n");
	scanf_s("%zu", &pid);
	return pid;
}

int main() {
	size_t pid = 0;
	HANDLE device = CreateFile(L"\\\\.\\processhider", GENERIC_WRITE, 
		FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);

	if (device == INVALID_HANDLE_VALUE)
		return error("Failed to open device");

	while ((pid = getPid()) != 0) {
		DWORD returned;
		BOOL success = DeviceIoControl(device,
			IOCTL_PROCESS_HIDE_BY_PID, 
			&pid, sizeof(pid), 
			nullptr, 0, 
			&returned, nullptr);
		if (success) {
			printf("Hiding pid succeeded!\n");
		} else {
			error("Hiding pid failed!");
		}
	}
	CloseHandle(device);
}