static struct pt_regs ppc_regs;

#define ARCH_REGS_FOR_GETREGS ppc_regs
#define ARCH_PC_REG ppc_regs.nip
#define ARCH_SP_REG ppc_regs.gpr[1]
