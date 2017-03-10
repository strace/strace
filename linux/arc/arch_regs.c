static struct user_regs_struct arc_regs;
unsigned long *const arc_sp_ptr = &arc_regs.sp;
#define ARCH_REGS_FOR_GETREGSET arc_regs
#define ARCH_PC_REG arc_regs.efa
