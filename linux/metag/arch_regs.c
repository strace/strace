static struct user_gp_regs metag_regs;
unsigned long *const metag_sp_ptr = &metag_regs.ax[0][0];
#define ARCH_REGS_FOR_GETREGSET metag_regs
#define ARCH_PC_REG metag_regs.pc
