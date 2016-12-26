/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	kernel_ulong_t scno = 0;

	/*
	 * In the new syscall ABI, the system call number is in R3.
	 */
	if (upeek(tcp->pid, 4*(REG_REG0+3), &scno) < 0)
		return -1;

	if ((long) scno < 0) {
		/* Odd as it may seem, a glibc bug has been known to cause
		   glibc to issue bogus negative syscall numbers.  So for
		   our purposes, make strace print what it *should* have been */
		kernel_ulong_t correct_scno = (scno & 0xff);
		if (debug_flag)
			error_msg("Detected glibc bug: bogus system call"
				  " number = %ld, correcting to %ld",
				  scno, correct_scno);
		scno = correct_scno;
	}

	tcp->scno = scno;
	return 1;
}
