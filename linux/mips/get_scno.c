scno = mips_REG_V0;

if (!SCNO_IN_RANGE(scno)) {
	if (mips_REG_A3 == 0 || mips_REG_A3 == (uint64_t) -1) {
		if (debug_flag)
			fprintf(stderr, "stray syscall exit: v0 = %ld\n", scno);
		return 0;
	}
}
