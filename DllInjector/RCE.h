#pragma once
#include <Windows.h>

namespace RCE {
	struct FreeLibraryParams {
		BOOL(WINAPI* freeLibrary)(_In_ HMODULE hLibModule);
		HMODULE(WINAPI* getModuleHandleA)(_In_opt_ LPCSTR lpModuleName);
		const char* dllName;
		FreeLibraryParams(const char* dllName);
	};

#pragma runtime_checks("", off)
	static DWORD WINAPI freeLibrary(FreeLibraryParams* params) {
		params->freeLibrary(params->getModuleHandleA(params->dllName));
		return 0;
	}

	static void freeLibraryEnd() {}
#pragma runtime_checks("", restore)
}

