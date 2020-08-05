#include <iostream>
#include "inject.h"

int main(int argc, char* argv[])
{
	if (argc != 3) {
		std::cout << "Usage: DllInjector.exe [process name] [dll absolute path]" << std::endl;
		return 1;
	}

	std::string processName(argv[1]), dllPath(argv[2]);

	std::cout << "Searching processes..." << std::endl;
	auto procs = InjectableProcess::processesByName(processName);
	for (auto& proc : procs) {
		proc.injectDll(dllPath);
	}
	std::cout << "Injected DLL... press any key to free" << std::endl;
	system("pause");
	for (auto& proc : procs) {
		proc.releaseDll(dllPath);
	}
	std::cout << "Freed DLL" << std::endl;
	return 0;
}
