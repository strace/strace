static struct user_regs_struct i386_regs;
long *const i386_esp_ptr = &i386_regs.esp;

#define ARCH_REGS_FOR_GETREGS i386_regs
