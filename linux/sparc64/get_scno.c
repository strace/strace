/* Retrieve the syscall trap instruction. */
unsigned long trap;
errno = 0;
trap = ptrace(PTRACE_PEEKTEXT, tcp->pid, (char *)sparc_regs.tpc, 0);
if (errno)
	return -1;
trap >>= 32;
switch (trap) {
case 0x91d02010:
	/* Linux/SPARC syscall trap. */
	update_personality(tcp, 0);
	break;
case 0x91d0206d:
	/* Linux/SPARC64 syscall trap. */
	update_personality(tcp, 1);
	break;
}

scno = sparc_regs.u_regs[U_REG_G1];
