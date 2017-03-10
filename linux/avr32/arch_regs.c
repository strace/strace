static struct pt_regs avr32_regs;
unsigned long *const avr32_sp_ptr = &avr32_regs.sp;
#define ARCH_REGS_FOR_GETREGS avr32_regs
#define ARCH_PC_REG avr32_regs.pc
