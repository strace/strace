/* Disassemble the syscall trap. */
/* Retrieve the syscall trap instruction. */
unsigned long trap;

errno = 0;

#ifdef SPARC64
trap = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)sparc_regs.tpc, 0);
trap >>= 32;
#else
trap = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)sparc_regs.pc, 0);
#endif

if (errno)
	return -1;

/* Disassemble the trap to see what personality to use. */
switch (trap) {
case 0x91d02010:
	/* Linux/SPARC syscall trap. */
	update_personality(tcp, 0);
	break;
case 0x91d0206d:
	/* Linux/SPARC64 syscall trap. */
	update_personality(tcp, 2);
	break;
case 0x91d02000:
	/* SunOS syscall trap. (pers 1) */
	fprintf(stderr, "syscall: SunOS no support\n");
	return -1;
case 0x91d02008:
	/* Solaris 2.x syscall trap. (per 2) */
	update_personality(tcp, 1);
	break;
case 0x91d02009:
	/* NetBSD/FreeBSD syscall trap. */
	fprintf(stderr, "syscall: NetBSD/FreeBSD not supported\n");
	return -1;
case 0x91d02027:
	/* Solaris 2.x gettimeofday */
	update_personality(tcp, 1);
	break;
default:
#ifdef SPARC64
	fprintf(stderr, "syscall: unknown syscall trap %08lx %016lx\n", trap, sparc_regs.tpc);
#else
	fprintf(stderr, "syscall: unknown syscall trap %08lx %08lx\n", trap, sparc_regs.pc);
#endif
	return -1;
}

/* Extract the system call number from the registers. */
if (trap == 0x91d02027) {
	scno = 156;
} else {
	scno = sparc_regs.u_regs[U_REG_G1];
}

if (scno == 0) {
	scno = sparc_regs.u_regs[U_REG_O0];
	memmove(&sparc_regs.u_regs[U_REG_O0], &sparc_regs.u_regs[U_REG_O1], 7*sizeof(sparc_regs.u_regs[0]));
}
