#include "x86_64/arch_regs.c"
#define ARCH_PC_REG (x86_io.iov_len == sizeof(i386_regs) ? i386_regs.eip : x86_64_regs.rip)
