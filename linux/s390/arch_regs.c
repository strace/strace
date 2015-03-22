/* PTRACE_GETREGSET on S390 is available since linux v2.6.27. */
static struct user_regs_struct s390_regset;
unsigned long *const s390_frame_ptr = &s390_regset.gprs[15];
#define ARCH_REGS_FOR_GETREGSET s390_regset
