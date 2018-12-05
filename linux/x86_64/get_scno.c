/*
 * Copyright (c) 2002 Andi Kleen <ak@suse.de>
 * Copyright (c) 2002 Michal Ludvig <mludvig@suse.cz>
 * Copyright (c) 2002 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2008-2013 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2012 H.J. Lu <hongjiu.lu@intel.com>
 * Copyright (c) 2010-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2015-2018 The strace developers.
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
	unsigned int currpers;

#ifndef __X32_SYSCALL_BIT
# define __X32_SYSCALL_BIT	0x40000000
#endif

	/*
	 * GETREGSET of NT_PRSTATUS tells us regset size,
	 * which unambiguously detects i386.
	 *
	 * Linux kernel distinguishes x86-64 and x32 processes
	 * solely by looking at __X32_SYSCALL_BIT:
	 * arch/x86/include/asm/compat.h::is_x32_task():
	 * if (task_pt_regs(current)->orig_ax & __X32_SYSCALL_BIT)
	 *	return true;
	 */
	if (x86_io.iov_len == sizeof(i386_regs)) {
		scno = i386_regs.orig_eax;
		currpers = 1;
	} else {
		scno = x86_64_regs.orig_rax;
		currpers = 0;
		if (scno & __X32_SYSCALL_BIT) {
			/*
			 * Syscall number -1 requires special treatment:
			 * it might be a side effect of SECCOMP_RET_ERRNO
			 * filtering that sets orig_rax to -1
			 * in some versions of linux kernel.
			 * If that is the case, then
			 * __X32_SYSCALL_BIT logic does not apply.
			 */
			if ((long long) x86_64_regs.orig_rax != -1) {
				currpers = 2;
			} else {
# ifdef X32
				currpers = 2;
# endif
			}
		}
	}

#ifdef X32
	/*
	 * If we are built for a x32 system, then personality 0 is x32
	 * (not x86_64), and stracing of x86_64 apps is not supported.
	 * Stracing of i386 apps is still supported.
	 */
	if (currpers == 0) {
		error_msg("syscall_%" PRI_klu "(...) in unsupported "
			  "64-bit mode of process PID=%d", scno, tcp->pid);
		return 0;
	}
	currpers &= ~2; /* map 2,1 to 0,1 */
#endif /* X32 */

	update_personality(tcp, currpers);
	tcp->scno = scno;
	return 1;
}
