#include "hooker.h"

Trampoline::Trampoline(void* address) : movabs_rax{ '\x48', '\xb8' }, address(address), jmp_rax{ '\xff', '\xe0' } {}

Hook::Hook(void* func, void* hook) : func(func), hook(hook), active(false) {
	memcpy(originalBytes, func, sizeof(Trampoline));
}

Hook::~Hook() {
	revert();
}

void Hook::apply() {
	DWORD oldProtect, newProtect = PAGE_EXECUTE_READWRITE;

	// Setup trampoline
	VirtualProtect(func, sizeof(Trampoline), newProtect, &oldProtect);
	*static_cast<Trampoline*>(func) = Trampoline(hook);
	VirtualProtect(func, sizeof(Trampoline), oldProtect, &newProtect);
	active = true;
}

void Hook::revert() {
	if (!active)
		return;

	DWORD oldProtect, newProtect = PAGE_EXECUTE_READWRITE;
	VirtualProtect(func, sizeof(Trampoline), newProtect, &oldProtect);
	memcpy(func, originalBytes, sizeof(Trampoline));
	VirtualProtect(func, sizeof(Trampoline), oldProtect, &newProtect);
	active = false;
}
