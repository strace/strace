static struct pt_all_user_regs ia64_regs;
unsigned long *const ia64_frame_ptr = &ia64_regs.gr[12];

#define ARCH_REGS_FOR_GETREGS ia64_regs
#define ARCH_PC_REG ia64_regs.br[0]
