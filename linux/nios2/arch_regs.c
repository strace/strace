static struct user_pt_regs nios2_regs;
unsigned int *const nios2_sp_ptr = &nios2_regs.regs[PTR_SP];
# define ARCH_REGS_FOR_GETREGSET nios2_regs
#define ARCH_PC_REG nios2_regs.regs[PTR_EA]
