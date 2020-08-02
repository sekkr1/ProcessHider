#pragma once
#include <string>
#include <Windows.h>

BOOL injectDll(DWORD pid, const std::string& dllName);
