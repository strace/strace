struct mips_regs mips_regs; /* not static */
/* PTRACE_GETREGS on MIPS is available since linux v2.6.15. */
#define ARCH_REGS_FOR_GETREGS mips_regs
