#pragma once
#include <Windows.h>
#include <winternl.h>

typedef DWORD(WINAPI* GetProcessIdFunc)(_In_ HANDLE Process);
typedef VOID(*RtlInitUnicodeStringFunc)(
    PUNICODE_STRING         DestinationString,
    __drv_aliasesMem PCWSTR SourceString
    );

typedef __kernel_entry NTSTATUS
(NTAPI* NtQuerySystemInformationFunc)(
    IN SYSTEM_INFORMATION_CLASS SystemInformationClass,
    OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG ReturnLength OPTIONAL
    );

typedef NTSYSAPI LONG(*RtlCompareUnicodeStringFunc)(
    PCUNICODE_STRING String1,
    PCUNICODE_STRING String2,
    BOOLEAN          CaseInSensitive
);

RtlCompareUnicodeStringFunc _RtlCompareUnicodeString;
const auto _NtQuerySystemInformation = reinterpret_cast<NtQuerySystemInformationFunc>(GetProcAddress(GetModuleHandle("ntdll"), "NtQuerySystemInformation"));
RtlInitUnicodeStringFunc _RtlInitUnicodeString;
