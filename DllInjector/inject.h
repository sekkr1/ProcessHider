#pragma once
#include <string>
#include <Windows.h>
#include <vector>
#include <wil/resource.h>

class InjectableProcess {
	wil::unique_process_handle process;
public:
	InjectableProcess(DWORD pid);
	static std::vector<InjectableProcess> processesByName(const std::string& name);
	wil::unique_virtualalloc_ptr<> writeToNewRegion(const void* data, size_t size);
	void injectDll(const std::string& dllName);
	void releaseDll(const std::string& dllName);
};
