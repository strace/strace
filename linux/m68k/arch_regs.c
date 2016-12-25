static struct user_regs_struct m68k_regs;
unsigned long *const m68k_usp_ptr = (unsigned long *) &m68k_regs.usp;
#define ARCH_PC_REG m68k_regs.pc
#define ARCH_REGS_FOR_GETREGS m68k_regs
