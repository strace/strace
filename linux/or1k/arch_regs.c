static struct user_regs_struct or1k_regs;
unsigned long *const or1k_sp_ptr = &or1k_regs.gpr[1];
#define ARCH_REGS_FOR_GETREGSET or1k_regs
#define ARCH_PC_REG or1k_regs.pc
