#include "inject.h"
#include <iostream>

struct FreeLibraryRCEParams {
		BOOL
		(WINAPI* freeLibrary)(
			_In_ HMODULE hLibModule
		);
		HMODULE
			(WINAPI*
			GetModuleHandleA)(
				_In_opt_ LPCSTR lpModuleName
			);
	char* dllName;
};

#pragma runtime_checks("", off)
typedef struct {
	HMODULE (WINAPI *loadLibrary)(_In_ LPCSTR lpLibFileName);
	char param[256];
} InjectedFuncParameters;

static DWORD WINAPI injectedFunc(FreeLibraryRCEParams* params) {
	params->freeLibrary(params->GetModuleHandleA(params->dllName));
	return 0;
}

static void injectedFuncEnd() {}
#pragma runtime_checks("", restore)

BOOL injectDll(DWORD pid, const std::string& dllName) {
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (NULL == process)
		return FALSE;

	VOID *dllNameRegion = VirtualAllocEx(process, NULL, dllName.size() + 1, MEM_COMMIT, PAGE_READWRITE);

	if (NULL == dllNameRegion)
		return FALSE;

	SIZE_T bytesWritten;

	if (0 == WriteProcessMemory(process, dllNameRegion, dllName.c_str(), dllName.size() + 1, &bytesWritten))
		return FALSE;

	if (bytesWritten != dllName.size() + 1)
		return FALSE;

	DWORD tid;
	HANDLE thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, dllNameRegion, CREATE_SUSPENDED, &tid);
	if (NULL == thread)
		return FALSE;

	ResumeThread(thread);
	std::cout << "DLL injected..." << std::endl;
	WaitForSingleObject(thread, INFINITE);
	std::cout << "Hook applied! Press any key to unhook" << std::endl;

	system("Pause");

	VOID* paramsRegion = VirtualAllocEx(process, NULL, dllName.size() + 1, MEM_COMMIT, PAGE_READWRITE);

	if (NULL == paramsRegion)
		return FALSE;

	FreeLibraryRCEParams params{
		FreeLibrary,
		GetModuleHandle,
		(char *)dllNameRegion
	};

	if (0 == WriteProcessMemory(process, paramsRegion, &params, sizeof(params), &bytesWritten))
		return FALSE;

	if (bytesWritten != sizeof(params))
		return FALSE;

	size_t injectedFuncSize = (size_t)injectedFuncEnd - (size_t)injectedFunc;
	VOID* injectedFuncRegion = VirtualAllocEx(process, NULL, injectedFuncSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	if (NULL == injectedFuncRegion)
		return FALSE;

	if (0 == WriteProcessMemory(process, injectedFuncRegion, injectedFunc, injectedFuncSize, &bytesWritten))
		return FALSE;

	if (bytesWritten != injectedFuncSize)
		return FALSE;

	thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)injectedFuncRegion, (void*)(paramsRegion), CREATE_SUSPENDED, &tid);
	if (NULL == thread)
		return FALSE;

	ResumeThread(thread);
	WaitForSingleObject(thread, INFINITE);
	std::cout << "Unhooked..." << std::endl;
	system("pause");

	return TRUE;
}
