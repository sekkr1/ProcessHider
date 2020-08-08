#pragma once
#include <Windows.h>
#include <wil/resource.h>
#include <string>
#include <TlHelp32.h>

namespace Hooking {
#pragma pack(push,1)
#ifdef _WIN64
	class Trampoline {
		char movabs_rax[2] = { '\x48', '\xb8' };
		void* address;
		char jmp_rax[2] = { '\xff', '\xe0' };
	public:
		Trampoline(void* address) : address(address) {}
	};
#else
	class Trampoline {
		char push[1] = { '\x68' };
		void* address;
		char ret[1] = { '\xc3' };
	public:
		Trampoline(void* address) : address(address) {}
	};
#endif
#pragma pack(pop)


	template<typename Fn>
	class IATHook {
		Fn func, hook;
		Fn* iatAddress;
	public:
		IATHook() = default;
		IATHook(Fn hook, const std::string& importName) : hook(hook) {
			auto baseAddr = reinterpret_cast<char*>(GetModuleHandle(NULL));
			auto dosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(baseAddr);
			auto ntHeader = reinterpret_cast<IMAGE_NT_HEADERS64*>(baseAddr + dosHeader->e_lfanew);
			auto optionalHeader = &ntHeader->OptionalHeader;
			auto dataDirectory = optionalHeader->DataDirectory;
			auto importDirectory = &dataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
			auto importDirectoryDataEntry = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(baseAddr + importDirectory->VirtualAddress);
			while (NULL != importDirectoryDataEntry->Name) {
				auto importDirectoryDataEntryName = reinterpret_cast<char*>(baseAddr + importDirectoryDataEntry->Name);
				auto importDirectoryDataEntryOriginalThunk = reinterpret_cast<IMAGE_THUNK_DATA*>(baseAddr + importDirectoryDataEntry->OriginalFirstThunk);
				auto i = 0;
				auto found = false;
				while (NULL != importDirectoryDataEntryOriginalThunk->u1.Ordinal) {
					if (importDirectoryDataEntryOriginalThunk->u1.Ordinal & 0x8000000000000000) {
						auto importDirectoryDataEntryOriginalThunkOrdinal = static_cast<WORD>(importDirectoryDataEntryOriginalThunk->u1.Ordinal);
					}
					else {
						auto importDirectoryDataEntryOriginalThunkHNTable = reinterpret_cast<_IMAGE_IMPORT_BY_NAME*>(baseAddr + (static_cast<DWORD>(importDirectoryDataEntryOriginalThunk->u1.AddressOfData) & 0x7FFFFFFF));
						if (importName == importDirectoryDataEntryOriginalThunkHNTable->Name) {
							found = true;
							break;
						}
					}
					importDirectoryDataEntryOriginalThunk++;
					i++;
				}
				if (found) {
					auto importDirectoryDataEntryThunk = reinterpret_cast<IMAGE_THUNK_DATA*>(baseAddr + importDirectoryDataEntry->FirstThunk) + i;
					iatAddress = reinterpret_cast<Fn*>(&importDirectoryDataEntryThunk->u1.Function);
					func = *iatAddress;
					return;
				}
				importDirectoryDataEntry++;
			}
			throw std::exception("Imported function couldn't be found");
		}
		void apply() {
			DWORD oldProtect, newProtect = PAGE_EXECUTE_READWRITE;
			VirtualProtect(iatAddress, sizeof(void*), newProtect, &oldProtect);
			*iatAddress = hook;
			VirtualProtect(iatAddress, sizeof(void*), oldProtect, &newProtect);
		}
		void revert() {
			DWORD oldProtect, newProtect = PAGE_EXECUTE_READWRITE;
			VirtualProtect(iatAddress, sizeof(void*), newProtect, &oldProtect);
			*iatAddress = func;
			VirtualProtect(iatAddress, sizeof(void*), oldProtect, &newProtect);
		}
		template<typename... Args>
		typename std::result_of<Fn(Args...)>::type callOriginal(Args&&... args) {
			return func(std::forward<Args>(args)...);
		}
		~IATHook() {
			revert();
		}
	};

	class TrampolineHook {
		void* func;
		void* hook;
		char originalBytes[sizeof(Trampoline)];
		bool active = false;
	public:
		TrampolineHook() = default;
		TrampolineHook(void* func, void* hook) : func(func), hook(hook) {
			memcpy(originalBytes, func, sizeof(Trampoline));
		}
		void apply() {
			DWORD oldProtect, newProtect = PAGE_EXECUTE_READWRITE;

			// Setup trampoline
			VirtualProtect(func, sizeof(Trampoline), newProtect, &oldProtect);
			*reinterpret_cast<Trampoline*>(func) = Trampoline(hook);
			VirtualProtect(func, sizeof(Trampoline), oldProtect, &newProtect);
			active = true;
		}
		void revert() {
			DWORD oldProtect, newProtect = PAGE_EXECUTE_READWRITE;
			VirtualProtect(func, sizeof(Trampoline), newProtect, &oldProtect);
			memcpy(func, originalBytes, sizeof(Trampoline));
			VirtualProtect(func, sizeof(Trampoline), oldProtect, &newProtect);
			active = false;
		}
		~TrampolineHook() {
			revert();
		}
	};
}
