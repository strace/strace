static struct pt_regs arm_regs;
long *const arm_sp_ptr = &arm_regs.ARM_sp;

#define ARCH_REGS_FOR_GETREGS arm_regs
