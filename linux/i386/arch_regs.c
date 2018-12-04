static struct user_regs_struct i386_regs;

#define ARCH_REGS_FOR_GETREGS i386_regs
#define ARCH_PC_REG i386_regs.eip
#define ARCH_SP_REG i386_regs.esp
