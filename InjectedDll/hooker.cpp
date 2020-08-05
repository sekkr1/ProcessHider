#include "hooker.h"

Trampoline::Trampoline(void* address) : movabs_rax{ '\x48', '\xb8' }, address(address), jmp_rax{ '\xff', '\xe0' } {}
