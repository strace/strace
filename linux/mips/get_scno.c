scno = mips_REG_V0;

if (!SCNO_IN_RANGE(scno)) {
	if (mips_REG_A3 == 0 || mips_REG_A3 == (uint64_t) -1) {
		if (debug_flag)
			error_msg("stray syscall exit: v0 = %ld", scno);
		return 0;
	}
}
