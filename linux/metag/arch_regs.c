static struct user_gp_regs metag_regs;
#define ARCH_REGS_FOR_GETREGSET metag_regs
#define ARCH_PC_REG metag_regs.pc
#define ARCH_SP_REG metag_regs.ax[0][0]
