static struct user_regs_struct riscv_regs;
unsigned long *const riscv_sp_ptr = &riscv_regs.sp;
#define ARCH_REGS_FOR_GETREGSET riscv_regs
#define ARCH_PC_REG riscv_regs.pc
