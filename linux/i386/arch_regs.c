static struct user_regs_struct i386_regs;
unsigned long *const i386_esp_ptr = (unsigned long *) &i386_regs.esp;

#define ARCH_REGS_FOR_GETREGS i386_regs
#define ARCH_PC_REG i386_regs.eip
