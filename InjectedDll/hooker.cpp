#include "hooker.h"

Trampoline::Trampoline(void* address) : movabs_rax{ '\x48', '\xb8' }, address(address), jmp_rax{ '\xff', '\xe0' } {}

Hook::Hook() {}

Hook::Hook(void* func, void* hook) : func(func), hook(hook), active(false) {
    memcpy(originalBytes, func, sizeof(Trampoline));
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

/* void hookFunction(void* func, void* hook, void** originalFunc) {
    const char JMP_QWORD_PTR[]{ '\x48', '\xff', '\x25' };
    DWORD oldProtect;

    if (nullptr != originalFunc) {
        // Create new function start
        if (0 == memcmp(func, &JMP_QWORD_PTR, sizeof(JMP_QWORD_PTR))) {
            auto rel_jump = *reinterpret_cast<int*>(static_cast<char*>(func) + sizeof(JMP_QWORD_PTR));
            *originalFunc = *reinterpret_cast<void**>(static_cast<char*>(func) + rel_jump + sizeof(int) + sizeof(JMP_QWORD_PTR));
        }
        else {
            auto newFunc = VirtualAlloc(0, 16 + sizeof(Trampoline), MEM_COMMIT, PAGE_EXECUTE_READWRITE);
            memcpy(newFunc, func, 16);
            *reinterpret_cast<Trampoline*>(static_cast<char*>(newFunc) + 16) = Trampoline(static_cast<char*>(func) + 16);
            VirtualProtect(newFunc, 16 + sizeof(Trampoline), PAGE_EXECUTE_READ, &oldProtect);
            *originalFunc = newFunc;
        }
    }

    // Setup trampoline
    VirtualProtect(func, sizeof(Trampoline), PAGE_EXECUTE_READWRITE, &oldProtect);
    *static_cast<Trampoline*>(func) = Trampoline(hook);
    VirtualProtect(func, sizeof(Trampoline), oldProtect, &oldProtect);
} */
