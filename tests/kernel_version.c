/*
 * Check kernel version decoding.
 *
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

#include "tests.h"

#include <stddef.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <asm/unistd.h>
#include "scno.h"

#ifdef HAVE_LINUX_BPF_H
# include <linux/bpf.h>
#endif

#include "bpf_attr.h"
#include "print_fields.h"

#include "xlat.h"
#include "xlat/bpf_commands.h"

#define CMD_STR(x) #x
static const char *errstr;

static void
print_bpf_attr(void)
{
#if XLAT_RAW
	printf("{prog_type=0x15"
#else
	printf("{prog_type=0x15 /* BPF_PROG_TYPE_??? */"
#endif
		", insn_cnt=3134983661"
		", insns=NULL"
		", license=NULL"
		", log_level=24"
		", log_size=3141592653"
		", log_buf=NULL"
#if XLAT_RAW
		", kern_version=0xcafef00d"
#elif XLAT_VERBOSE
		", kern_version=0xcafef00d"
		" /* KERNEL_VERSION(51966, 240, 13) */"
#else
		", kern_version=KERNEL_VERSION(51966, 240, 13)"
#endif
		", prog_flags=0"
		", prog_name=\"\""
		", prog_ifindex=0"
		", expected_attach_type="
#if XLAT_RAW
		"0}"
#elif XLAT_VERBOSE
		"0 /* BPF_CGROUP_INET_INGRESS */}"
#else /* XLAT_ABBREV */
		"BPF_CGROUP_INET_INGRESS}"
#endif
		);
}

int
main(void)
{
	long ret;
	struct BPF_PROG_LOAD_struct prog = {
		.prog_type = 21,
		.insn_cnt = 0xbadc0ded,
		.insns = 0,
		.license = 0,
		.log_level = 24,
		.log_size = 3141592653U,
		.log_buf = 0,
		.kern_version = 0xcafef00d,
		.prog_flags = 0,
	};
	ret = syscall(__NR_bpf, BPF_PROG_LOAD, &prog, sizeof(prog));
	errstr = sprintrc(ret);
#if XLAT_RAW
	printf("bpf(%#x, ", BPF_PROG_LOAD);
#elif XLAT_VERBOSE
	printf("bpf(%#x /* %s */, ", BPF_PROG_LOAD, CMD_STR(BPF_PROG_LOAD));
#else
	printf("bpf(%s, ", CMD_STR(BPF_PROG_LOAD));
#endif
	print_bpf_attr();
	printf(", %u) = %s\n", (unsigned int) sizeof(prog), errstr);
	puts("+++ exited with 0 +++");
	return 0;
}
