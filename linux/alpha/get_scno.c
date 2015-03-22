if (upeek(tcp->pid, REG_A3, &alpha_a3) < 0)
	return -1;
if (upeek(tcp->pid, REG_R0, &scno) < 0)
	return -1;

/*
 * Do some sanity checks to figure out if it's
 * really a syscall entry
 */
if (!SCNO_IN_RANGE(scno)) {
	if (alpha_a3 == 0 || alpha_a3 == -1) {
		if (debug_flag)
			fprintf(stderr, "stray syscall exit: r0 = %ld\n", scno);
		return 0;
	}
}
