#pragma once
#include <Windows.h>
#include <winternl.h>
#include <wil/resource.h>

typedef VOID(*RtlInitUnicodeStringFunc)(
	PUNICODE_STRING         DestinationString,
	__drv_aliasesMem PCWSTR SourceString
	);

typedef NTSYSAPI BOOLEAN(*RtlEqualUnicodeStringFunc)(
	PCUNICODE_STRING String1,
	PCUNICODE_STRING String2,
	BOOLEAN          CaseInSensitive
	);

wil::unique_hmodule ntosKrnl(LoadLibrary("NtosKrnl.exe"));

auto _NtQuerySystemInformation = reinterpret_cast<decltype(&NtQuerySystemInformation)>(GetProcAddress(GetModuleHandle("ntdll"), "NtQuerySystemInformation"));
auto _RtlEqualUnicodeString = reinterpret_cast<RtlEqualUnicodeStringFunc>(GetProcAddress(ntosKrnl.get(), "RtlEqualUnicodeString"));
auto _RtlInitUnicodeString = reinterpret_cast<RtlInitUnicodeStringFunc>(GetProcAddress(ntosKrnl.get(), "RtlInitUnicodeString"));
