/* Note: we support only 32-bit CPUs, not 26-bit */

#if !defined(__ARM_EABI__) || ENABLE_ARM_OABI
if (arm_regs.ARM_cpsr & 0x20) {
	/* Thumb mode */
	goto scno_in_r7;
}
/* ARM mode */
/* Check EABI/OABI by examining SVC insn's low 24 bits */
errno = 0;
scno = ptrace(PTRACE_PEEKTEXT, tcp->pid, (void *)(arm_regs.ARM_pc - 4), NULL);
if (errno)
	return -1;
/* EABI syscall convention? */
if ((unsigned long) scno != 0xef000000) {
	/* No, it's OABI */
	if ((scno & 0x0ff00000) != 0x0f900000) {
		fprintf(stderr, "pid %d unknown syscall trap 0x%08lx\n",
			tcp->pid, scno);
		return -1;
	}
	/* Fixup the syscall number */
	scno &= 0x000fffff;
} else {
scno_in_r7:
	scno = arm_regs.ARM_r7;
}
#else /* __ARM_EABI__ || !ENABLE_ARM_OABI */

scno = arm_regs.ARM_r7;

#endif

scno = shuffle_scno(scno);

/*
 * Do some sanity checks to figure out
 * whether it's really a syscall entry.
 */
if (arm_regs.ARM_ip && !SCNO_IN_RANGE(scno)) {
	if (debug_flag)
		fprintf(stderr,
			"pid %d stray syscall exit: ARM_ip = %ld, scno = %ld\n",
			tcp->pid, arm_regs.ARM_ip, shuffle_scno(scno));
	return 0;
}
