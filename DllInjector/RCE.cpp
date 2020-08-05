#include "RCE.h"

namespace RCE {
	FreeLibraryParams::FreeLibraryParams(const char* dllName) : dllName(dllName), freeLibrary(FreeLibrary), getModuleHandleA(GetModuleHandleA) {}
}

