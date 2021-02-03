/*
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/* Return -1 on error or 1 on success (never 0!). */
static int
arch_get_syscall_args(struct tcb *tcp)
{
#if defined LINUX_MIPSN64 || defined LINUX_MIPSN32
	tcp->u_arg[0] = mips_REG_A0;
	tcp->u_arg[1] = mips_REG_A1;
	tcp->u_arg[2] = mips_REG_A2;
	tcp->u_arg[3] = mips_REG_A3;
	tcp->u_arg[4] = mips_REG_A4;
	tcp->u_arg[5] = mips_REG_A5;
#elif defined LINUX_MIPSO32
	tcp->u_arg[0] = mips_REG_A0;
	tcp->u_arg[1] = mips_REG_A1;
	tcp->u_arg[2] = mips_REG_A2;
	tcp->u_arg[3] = mips_REG_A3;
	if (n_args(tcp) > 4
	    && umoven(tcp, mips_REG_SP + 4 * sizeof(tcp->u_arg[0]),
		      (n_args(tcp) - 4) * sizeof(tcp->u_arg[0]),
		      &tcp->u_arg[4]) < 0) {
		error_msg("pid %d: cannot fetch 5th and 6th syscall arguments"
			  " from tracee's memory", tcp->pid);

		/*
		 * Let's proceed with the first 4 arguments
		 * instead of reporting the failure.
		 */
		memset(&tcp->u_arg[4], 0,
		       (n_args(tcp) - 4) * sizeof(tcp->u_arg[0]));
	}
#else
# error unsupported mips abi
#endif
	return 1;
}

#ifdef LINUX_MIPSO32
static void
arch_get_syscall_args_extra(struct tcb *tcp, const unsigned int n)
{
	/* This assumes n >= 4. */
	if (n_args(tcp) > n
	    && umoven(tcp, mips_REG_SP + n * sizeof(tcp->u_arg[0]),
		      (n_args(tcp) - n) * sizeof(tcp->u_arg[0]),
		      &tcp->u_arg[n]) < 0) {
		/*
		 * Let's proceed with the first n arguments
		 * instead of reporting the failure.
		 */
		memset(&tcp->u_arg[n], 0,
		       (n_args(tcp) - n) * sizeof(tcp->u_arg[0]));
	}
}
#endif

#ifdef SYS_syscall_subcall
static void
decode_syscall_subcall(struct tcb *tcp)
{
	if (!scno_is_valid(tcp->u_arg[0]))
		return;
	tcp->true_scno = tcp->scno = tcp->u_arg[0];
	tcp->qual_flg = qual_flags(tcp->scno);
	tcp->s_ent = &sysent[tcp->scno];
	memmove(&tcp->u_arg[0], &tcp->u_arg[1],
		sizeof(tcp->u_arg) - sizeof(tcp->u_arg[0]));
	/*
	 * Fetching the last arg of 7-arg syscalls (fadvise64_64
	 * and sync_file_range) requires additional code,
	 * see arch_get_syscall_args() above.
	 */
	if (n_args(tcp) == MAX_ARGS) {
		if (umoven(tcp,
			   mips_REG_SP + MAX_ARGS * sizeof(tcp->u_arg[0]),
			   sizeof(tcp->u_arg[0]),
			   &tcp->u_arg[MAX_ARGS - 1]) < 0)
		tcp->u_arg[MAX_ARGS - 1] = 0;
	}
}
#endif /* SYS_syscall_subcall */
