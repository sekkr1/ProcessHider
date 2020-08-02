#include "inject.h"

#pragma runtime_checks("", off)
typedef struct {
	HMODULE (WINAPI *loadLibrary)(_In_ LPCSTR lpLibFileName);
	char param[256];
} InjectedFuncParameters;

static DWORD WINAPI injectedFunc(InjectedFuncParameters *params) {
	params->loadLibrary(params->param);
	return 0;
}

static void injectedFuncEnd() {}
#pragma runtime_checks("", restore)

BOOL injectDll(DWORD pid, const std::string& dllName) {
	HANDLE process = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
	if (NULL == process)
		return FALSE;

	VOID *paramsRegion = VirtualAllocEx(process, NULL, dllName.size() + 1, MEM_COMMIT, PAGE_READWRITE);

	if (NULL == paramsRegion)
		return FALSE;

	SIZE_T bytesWritten;
	InjectedFuncParameters params;
	params.loadLibrary = LoadLibrary;
	strcpy_s(params.param, dllName.c_str());

	if (0 == WriteProcessMemory(process, paramsRegion, &params, sizeof(params), &bytesWritten))
		return FALSE;

	size_t injectedFuncSize = (size_t)injectedFuncEnd - (size_t)injectedFunc;
	VOID* injectedFuncRegion = VirtualAllocEx(process, NULL, injectedFuncSize, MEM_COMMIT,  PAGE_EXECUTE_READWRITE);

	if (NULL == injectedFuncRegion)
		return FALSE;

	if (0 == WriteProcessMemory(process, injectedFuncRegion, injectedFunc, injectedFuncSize, &bytesWritten))
		return FALSE;

	DWORD tid;
	HANDLE thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)LoadLibrary, (void *)((size_t)paramsRegion + 8), CREATE_SUSPENDED, &tid);
	if (NULL == thread)
		return FALSE;

	ResumeThread(thread);

	return TRUE;
}
