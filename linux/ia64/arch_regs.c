static struct pt_all_user_regs ia64_regs;
unsigned long *const ia64_frame_ptr = &ia64_regs.gr[12];

#define IA64_PSR_IS	((long)1 << 34)
#define ia64_ia32mode	(ia64_regs.cr_ipsr & IA64_PSR_IS)

#define ARCH_REGS_FOR_GETREGS ia64_regs
