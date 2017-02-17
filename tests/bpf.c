/*
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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
#include <asm/unistd.h>

#if defined HAVE_UNION_BPF_ATTR_LOG_BUF && defined __NR_bpf
# include <stdio.h>
# include <stdint.h>
# include <unistd.h>
# include <linux/bpf.h>

static const struct bpf_insn insns[] = {
	{ .code = BPF_JMP | BPF_EXIT }
};

static const char *errstr;
static char log_buf[4096];

static long
sys_bpf(kernel_ulong_t cmd, kernel_ulong_t attr, kernel_ulong_t size)
{
	long rc = syscall(__NR_bpf, cmd, attr, size);
	errstr = sprintrc(rc);
	return rc;
}

static int
map_create(void)
{
	union bpf_attr attr = {
		.key_size = 4,
		.value_size = 8,
		.max_entries = 256
	};
	void *const t_attr = tail_memdup(&attr, sizeof(attr));
	return sys_bpf(BPF_MAP_CREATE, (unsigned long) t_attr, sizeof(attr));
}

static int
map_any(int cmd)
{
	union bpf_attr attr = {
		.map_fd = -1,
		.key = 0xdeadbeef,
		.value = 0xbadc0ded
	};
	void *const t_attr = tail_memdup(&attr, sizeof(attr));
	return sys_bpf(cmd, (unsigned long) t_attr, sizeof(attr));
}

static int
prog_load(void)
{
	union bpf_attr attr = {
		.insn_cnt = sizeof(insns) / sizeof(insns[0]),
		.insns = (unsigned long) insns,
		.license = (unsigned long) "GPL",
		.log_level = 42,
		.log_size = sizeof(log_buf),
		.log_buf = (unsigned long) log_buf
	};
	void *const t_attr = tail_memdup(&attr, sizeof(attr));
	return sys_bpf(BPF_PROG_LOAD, (unsigned long) t_attr, sizeof(attr));
}

/*
 * bpf() syscall and its first six commands were introduced in Linux kernel
 * 3.18. Some additional commands were added afterwards, so we need to take
 * precautions to make sure the tests compile.
 *
 * BPF_OBJ_PIN and BPF_OBJ_GET commands appear in kernel 4.4.
 */
# ifdef HAVE_UNION_BPF_ATTR_BPF_FD
static int
obj_manage(int cmd)
{
	union bpf_attr attr = {
		.pathname = (unsigned long) "/sys/fs/bpf/foo/bar",
		.bpf_fd = -1
	};
	void *const t_attr = tail_memdup(&attr, sizeof(attr));
	return sys_bpf(cmd, (unsigned long) t_attr, sizeof(attr));
}
# endif

/* BPF_PROG_ATTACH and BPF_PROG_DETACH commands appear in kernel 4.10. */
# ifdef HAVE_UNION_BPF_ATTR_ATTACH_FLAGS
static int
prog_cgroup(int cmd)
{
	union bpf_attr attr = {
		.target_fd = -1,
		.attach_bpf_fd = -1,
		.attach_type = 0,
		.attach_flags = 1
	};
	void *const t_attr = tail_memdup(&attr, sizeof(attr));
	return sys_bpf(cmd, (unsigned long) t_attr, sizeof(attr));
}
# endif

static unsigned long efault;

static void
bogus_bpf(int cmd, const char *name)
{
	const unsigned long bogus_size = 1024;
	const unsigned long bogus_addr = efault - bogus_size;

	sys_bpf(cmd, efault, 4);
	printf("bpf(%s, %#lx, %lu) = %s\n",
	       name, efault, 4UL, errstr);

	sys_bpf(cmd, efault, bogus_size);
	printf("bpf(%s, %#lx, %lu) = %s\n",
	       name, efault, bogus_size, errstr);

	sys_bpf(cmd, bogus_addr, 0);
	printf("bpf(%s, %#lx, %lu) = %s\n",
	       name, bogus_addr, 0UL, errstr);
}

#define BOGUS_BPF(cmd)	bogus_bpf(cmd, #cmd)

int
main(void)
{
	efault = (unsigned long) tail_alloc(1) + 1;

	map_create();
	printf("bpf(BPF_MAP_CREATE"
	       ", {map_type=BPF_MAP_TYPE_UNSPEC, key_size=4"
	       ", value_size=8, max_entries=256}, %u) = %s\n",
	       (unsigned) sizeof(union bpf_attr), errstr);
	BOGUS_BPF(BPF_MAP_CREATE);

	map_any(BPF_MAP_LOOKUP_ELEM);
	printf("bpf(BPF_MAP_LOOKUP_ELEM"
	       ", {map_fd=-1, key=0xdeadbeef}, %u) = %s\n",
	       (unsigned) sizeof(union bpf_attr), errstr);
	BOGUS_BPF(BPF_MAP_LOOKUP_ELEM);

	map_any(BPF_MAP_UPDATE_ELEM);
	printf("bpf(BPF_MAP_UPDATE_ELEM"
	       ", {map_fd=-1, key=0xdeadbeef"
	       ", value=0xbadc0ded, flags=BPF_ANY}, %u) = %s\n",
	       (unsigned) sizeof(union bpf_attr), errstr);
	BOGUS_BPF(BPF_MAP_UPDATE_ELEM);

	map_any(BPF_MAP_DELETE_ELEM);
	printf("bpf(BPF_MAP_DELETE_ELEM"
	       ", {map_fd=-1, key=0xdeadbeef}, %u) = %s\n",
	       (unsigned) sizeof(union bpf_attr), errstr);
	BOGUS_BPF(BPF_MAP_DELETE_ELEM);

	map_any(BPF_MAP_GET_NEXT_KEY);
	printf("bpf(BPF_MAP_GET_NEXT_KEY"
	       ", {map_fd=-1, key=0xdeadbeef}, %u) = %s\n",
	       (unsigned) sizeof(union bpf_attr), errstr);
	BOGUS_BPF(BPF_MAP_GET_NEXT_KEY);

	prog_load();
	printf("bpf(BPF_PROG_LOAD"
	       ", {prog_type=BPF_PROG_TYPE_UNSPEC, insn_cnt=1, insns=%p"
	       ", license=\"GPL\", log_level=42, log_size=4096, log_buf=%p"
	       ", kern_version=0}, %u) = %s\n",
	       insns, log_buf, (unsigned) sizeof(union bpf_attr), errstr);
	BOGUS_BPF(BPF_PROG_LOAD);

# ifdef HAVE_UNION_BPF_ATTR_BPF_FD
	obj_manage(BPF_OBJ_PIN);
	printf("bpf(BPF_OBJ_PIN"
	       ", {pathname=\"/sys/fs/bpf/foo/bar\", bpf_fd=-1}, %u) = %s\n",
	       (unsigned) sizeof(union bpf_attr), errstr);
	BOGUS_BPF(BPF_OBJ_PIN);

	obj_manage(BPF_OBJ_GET);
	printf("bpf(BPF_OBJ_GET"
	       ", {pathname=\"/sys/fs/bpf/foo/bar\", bpf_fd=-1}, %u) = %s\n",
	       (unsigned) sizeof(union bpf_attr), errstr);
	BOGUS_BPF(BPF_OBJ_GET);
# endif

# ifdef HAVE_UNION_BPF_ATTR_ATTACH_FLAGS
	prog_cgroup(BPF_PROG_ATTACH);
	printf("bpf(BPF_PROG_ATTACH"
	       ", {target_fd=-1, attach_bpf_fd=-1"
	       ", attach_type=BPF_CGROUP_INET_INGRESS"
	       ", attach_flags=BPF_F_ALLOW_OVERRIDE}, %u) = %s\n",
	       (unsigned) sizeof(union bpf_attr), errstr);
	BOGUS_BPF(BPF_PROG_ATTACH);

	prog_cgroup(BPF_PROG_DETACH);
	printf("bpf(BPF_PROG_DETACH"
	       ", {target_fd=-1, attach_type=BPF_CGROUP_INET_INGRESS}, %u)"
	       " = %s\n",
	       (unsigned) sizeof(union bpf_attr), errstr);
	BOGUS_BPF(BPF_PROG_DETACH);
# endif

	bogus_bpf(0xfacefeed, "0xfacefeed /* BPF_??? */");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_bpf")

#endif
