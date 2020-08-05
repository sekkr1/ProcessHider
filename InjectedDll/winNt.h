#pragma once
#include <Windows.h>
#include <winternl.h>
#include <wil/resource.h>

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

typedef NTSYSAPI BOOLEAN(*RtlEqualUnicodeStringFunc)(
	PCUNICODE_STRING String1,
	PCUNICODE_STRING String2,
	BOOLEAN          CaseInSensitive
	);

wil::unique_hmodule ntosKrnl(LoadLibrary("NtosKrnl.exe"));
auto _NtQuerySystemInformation = reinterpret_cast<NtQuerySystemInformationFunc>(GetProcAddress(GetModuleHandle("ntdll"), "NtQuerySystemInformation"));
auto _RtlEqualUnicodeString = reinterpret_cast<RtlEqualUnicodeStringFunc>(GetProcAddress(static_cast<HMODULE>(ntosKrnl.get()), "RtlEqualUnicodeString"));
auto _RtlInitUnicodeString = reinterpret_cast<RtlInitUnicodeStringFunc>(GetProcAddress(static_cast<HMODULE>(ntosKrnl.get()), "RtlInitUnicodeString"));
