#include "inject.h"
#include "RCE.h"
#include <psapi.h>
#include <TlHelp32.h>

InjectableProcess::InjectableProcess(DWORD pid) {
	process = wil::unique_process_handle(OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid));
	if (NULL == process.get())
		throw std::exception("Couldn't open process");
}

std::vector<InjectableProcess> InjectableProcess::processesByName(const std::string& name) {
	wil::unique_handle snapshot(CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0));
	if (INVALID_HANDLE_VALUE == snapshot.get())
		throw std::exception("Couldn't create snapshot");

	std::vector<InjectableProcess> result;
	PROCESSENTRY32 process;
	process.dwSize = sizeof(PROCESSENTRY32);
	Process32First(snapshot.get(), &process);
	do {
		if (name == process.szExeFile)
			result.push_back(InjectableProcess(process.th32ProcessID));
	} while (Process32Next(snapshot.get(), &process));
	return result;
}

wil::unique_virtualalloc_ptr<> InjectableProcess::writeToNewRegion(const void* data, size_t size, DWORD protect = PAGE_EXECUTE_READWRITE) {
	wil::unique_virtualalloc_ptr<> region(VirtualAllocEx(process.get(), NULL, size, MEM_COMMIT, protect));

	if (NULL == region.get())
		throw std::exception("Couldn't allocate region");

	if (0 == WriteProcessMemory(process.get(), region.get(), data, size, NULL))
		throw std::exception("Couldn't write data to region");

	return region;
}

void InjectableProcess::injectDll(const std::string& dllName) {
	auto dllNameRegion = writeToNewRegion((void*)dllName.c_str(), dllName.size() + 1);

	wil::unique_handle thread(CreateRemoteThread(process.get(), NULL, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(LoadLibrary), dllNameRegion.get(), CREATE_SUSPENDED, NULL));
	if (NULL == thread.get())
		throw std::exception("Couldn't create remote thread");

	ResumeThread(thread.get());
	WaitForSingleObject(thread.get(), INFINITE);
}

void InjectableProcess::releaseDll(const std::string& dllName) {
	auto dllNameRegion = writeToNewRegion(dllName.c_str(), dllName.size() + 1);

	RCE::FreeLibraryParams params(static_cast<char*>(dllNameRegion.get()));
	auto paramsRegion = writeToNewRegion(&params, sizeof(RCE::FreeLibraryParams));

	auto injectedFuncSize = (size_t)RCE::freeLibraryEnd - (size_t)RCE::freeLibrary;
	auto injectedFuncRegion = writeToNewRegion(RCE::freeLibrary, injectedFuncSize);

	wil::unique_handle thread(CreateRemoteThread(process.get(), NULL, 0, (LPTHREAD_START_ROUTINE)injectedFuncRegion.get(), paramsRegion.get(), CREATE_SUSPENDED, NULL));
	if (NULL == thread.get())
		throw std::exception("Couldn't create remote thread");

	ResumeThread(thread.get());
	WaitForSingleObject(thread.get(), INFINITE);
}
