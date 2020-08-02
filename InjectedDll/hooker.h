#pragma once
#include <Windows.h>

// void hookFunction(void* func, void* hook, void** originalFunc);

#pragma pack(push,1)
class Trampoline {
    char movabs_rax[2];
    void* address;
    char jmp_rax[2];
public:
    Trampoline(void* address);
};
#pragma pack(pop)

class Hook {
    void* func;
    void* hook;
    char originalBytes[sizeof(Trampoline)];
    bool active;
public:
    Hook();
    Hook(void* func, void* hook);
    void apply();
    void revert();
};
