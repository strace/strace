static struct pt_regs arm_regs;
unsigned long *const arm_sp_ptr = (unsigned long *) &arm_regs.ARM_sp;

#define ARCH_REGS_FOR_GETREGS arm_regs
#define ARCH_PC_REG arm_regs.ARM_pc
