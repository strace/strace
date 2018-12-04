static struct pt_regs tile_regs;
#define ARCH_REGS_FOR_GETREGS tile_regs
#define ARCH_PC_REG tile_regs.pc
#define ARCH_SP_REG tile_regs.sp
