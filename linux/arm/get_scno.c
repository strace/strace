/*
 * Copyright (c) 2003 Russell King <rmk@arm.linux.org.uk>
 * Copyright (c) 2011-2013 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2011-2015 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Return codes: 1 - ok, 0 - ignore, other - error. */
static int
arch_get_scno(struct tcb *tcp)
{
	kernel_ulong_t scno = 0;

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
	if (scno != 0xef000000) {
		/* No, it's OABI */
		if ((scno & 0x0ff00000) != 0x0f900000) {
			error_msg("pid %d unknown syscall trap 0x%08lx",
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
	if (arm_regs.ARM_ip && !scno_in_range(scno)) {
		if (debug_flag)
			error_msg("pid %d stray syscall exit:"
				  " ARM_ip = %ld, scno = %ld",
				  tcp->pid, arm_regs.ARM_ip,
				  shuffle_scno(scno));
		return 0;
	}

	tcp->scno = scno;
	return 1;
}
