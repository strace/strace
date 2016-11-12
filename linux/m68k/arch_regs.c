static struct user_regs_struct m68k_regs;
long *const m68k_usp_ptr = &m68k_regs.usp;
#define ARCH_PC_REG m68k_regs.pc
#define ARCH_REGS_FOR_GETREGS m68k_regs
