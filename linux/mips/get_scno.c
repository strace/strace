/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	tcp->scno = mips_REG_V0;

	if (!scno_in_range(tcp->scno)) {
		if (mips_REG_A3 == 0 || mips_REG_A3 == (uint64_t) -1) {
			if (debug_flag)
				error_msg("stray syscall exit: v0 = %ld",
					  tcp->scno);
			return 0;
		}
	}

	return 1;
}
