/*
 * Raw syscalls.
 *
 * Copyright (c) 2018 The strace developers.
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

#ifndef STRACE_RAW_SYSCALL_H
#define STRACE_RAW_SYSCALL_H

# include "kernel_types.h"

static inline kernel_ulong_t
raw_syscall_0(const kernel_ulong_t nr, kernel_ulong_t *err)
{
	register kernel_ulong_t r15 __asm__("r15") = nr;
	register kernel_ulong_t r8 __asm__("r8");
	register kernel_ulong_t r10 __asm__("r10");
	__asm__ __volatile__("break 0x100000"
			     : "=r"(r8), "=r"(r10), "+r"(r15)
			     :
			     : "memory", "out0", "out1", "out2",
			       "out3", "out4", "out5", "out6", "out7",
			       "r2", "r3", "r9", "r11", "r12", "r13",
			       "r14", "r16", "r17", "r18", "r19", "r20",
			       "r21", "r22", "r23", "r24", "r25", "r26",
			       "r27", "r28", "r29", "r30", "r31",
			       "p6", "p7", "p8", "p9", "p10",
			       "p11", "p12", "p13", "p14", "p15",
			       "f6", "f7", "f8", "f9", "f10",
			       "f11", "f12", "f13", "f14", "f15",
			       "f5", "f6", "f7", "f8", "f9", "f10", "f11",
			       "b6", "b7");
	*err = !!r10;
	return r8;
}
# define raw_syscall_0 raw_syscall_0

#endif /* !STRACE_RAW_SYSCALL_H */
