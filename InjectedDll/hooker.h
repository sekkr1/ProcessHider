#pragma once
#include <Windows.h>
#include <wil/resource.h>

#pragma pack(push,1)
class Trampoline {
	char movabs_rax[2];
	void* address;
	char jmp_rax[2];
public:
	Trampoline(void* address);
};
#pragma pack(pop)

template<typename Fn>
class Hook {
	Fn func;
	Fn hook;
	char originalBytes[sizeof(Trampoline)];
	bool active = false;
public:
	Hook() = default;
	Hook(Fn func, Fn hook) : func(func), hook(hook) {
		memcpy(originalBytes, func, sizeof(Trampoline));
	}
	~Hook() {
		revert();
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
		if (!active)
			return;

		DWORD oldProtect, newProtect = PAGE_EXECUTE_READWRITE;
		VirtualProtect(func, sizeof(Trampoline), newProtect, &oldProtect);
		memcpy(func, originalBytes, sizeof(Trampoline));
		VirtualProtect(func, sizeof(Trampoline), oldProtect, &newProtect);
		active = false;
	}
	template<typename... Args>
	typename std::result_of<Fn(Args...)>::type callOriginal(Args&&... args) {
		revert();
		auto retval = func(std::forward<Args>(args)...);
		apply();
		return retval;
	}
};
