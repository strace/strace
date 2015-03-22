#ifndef __X32_SYSCALL_BIT
# define __X32_SYSCALL_BIT	0x40000000
#endif

unsigned int currpers;

#if 1
/* GETREGSET of NT_PRSTATUS tells us regset size,
 * which unambiguously detects i386.
 *
 * Linux kernel distinguishes x86-64 and x32 processes
 * solely by looking at __X32_SYSCALL_BIT:
 * arch/x86/include/asm/compat.h::is_x32_task():
 * if (task_pt_regs(current)->orig_ax & __X32_SYSCALL_BIT)
 *         return true;
 */
if (x86_io.iov_len == sizeof(i386_regs)) {
	scno = i386_regs.orig_eax;
	currpers = 1;
} else {
	scno = x86_64_regs.orig_rax;
	currpers = 0;
	if (scno & __X32_SYSCALL_BIT) {
		/*
		 * Syscall number -1 requires special treatment:
		 * it might be a side effect of SECCOMP_RET_ERRNO
		 * filtering that sets orig_rax to -1
		 * in some versions of linux kernel.
		 * If that is the case, then
		 * __X32_SYSCALL_BIT logic does not apply.
		 */
		if ((long long) x86_64_regs.orig_rax != -1) {
			scno -= __X32_SYSCALL_BIT;
			currpers = 2;
		} else {
# ifdef X32
			currpers = 2;
# endif
		}
	}
}

#elif 0
/* cs = 0x33 for long mode (native 64 bit and x32)
 * cs = 0x23 for compatibility mode (32 bit)
 * ds = 0x2b for x32 mode (x86-64 in 32 bit)
 */
scno = x86_64_regs.orig_rax;
switch (x86_64_regs.cs) {
	case 0x23: currpers = 1; break;
	case 0x33:
		if (x86_64_regs.ds == 0x2b) {
			currpers = 2;
			scno &= ~__X32_SYSCALL_BIT;
		} else
			currpers = 0;
		break;
	default:
		fprintf(stderr, "Unknown value CS=0x%08X while "
			 "detecting personality of process "
			 "PID=%d\n", (int)x86_64_regs.cs, tcp->pid);
		currpers = current_personality;
		break;
}
#elif 0
/* This version analyzes the opcode of a syscall instruction.
 * (int 0x80 on i386 vs. syscall on x86-64)
 * It works, but is too complicated, and strictly speaking, unreliable.
 */
unsigned long call, rip = x86_64_regs.rip;
/* sizeof(syscall) == sizeof(int 0x80) == 2 */
rip -= 2;
errno = 0;
call = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)rip, (char *)0);
if (errno)
	fprintf(stderr, "ptrace_peektext failed: %s\n",
			strerror(errno));
switch (call & 0xffff) {
	/* x86-64: syscall = 0x0f 0x05 */
	case 0x050f: currpers = 0; break;
	/* i386: int 0x80 = 0xcd 0x80 */
	case 0x80cd: currpers = 1; break;
	default:
		currpers = current_personality;
		fprintf(stderr,
			"Unknown syscall opcode (0x%04X) while "
			"detecting personality of process "
			"PID=%d\n", (int)call, tcp->pid);
		break;
}
#endif

#ifdef X32
/* If we are built for a x32 system, then personality 0 is x32
 * (not x86_64), and stracing of x86_64 apps is not supported.
 * Stracing of i386 apps is still supported.
 */
if (currpers == 0) {
	fprintf(stderr, "syscall_%lu(...) in unsupported "
			"64-bit mode of process PID=%d\n",
		scno, tcp->pid);
	return 0;
}
currpers &= ~2; /* map 2,1 to 0,1 */
#endif /* X32 */

update_personality(tcp, currpers);
