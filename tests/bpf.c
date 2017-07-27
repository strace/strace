/*
 * Check bpf syscall decoding.
 *
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

#if defined __NR_bpf				\
 && (defined HAVE_UNION_BPF_ATTR_ATTACH_FLAGS	\
  || defined HAVE_UNION_BPF_ATTR_BPF_FD		\
  || defined HAVE_UNION_BPF_ATTR_FLAGS		\
  || defined HAVE_UNION_BPF_ATTR_INNER_MAP_FD	\
  || defined HAVE_UNION_BPF_ATTR_PROG_FLAGS)

# include <stddef.h>
# include <stdio.h>
# include <stdint.h>
# include <string.h>
# include <unistd.h>
# include <linux/bpf.h>

static const kernel_ulong_t long_bits = (kernel_ulong_t) 0xfacefeed00000000ULL;
static const char *errstr;
static unsigned int sizeof_attr = sizeof(union bpf_attr);
static unsigned int page_size;
static unsigned long end_of_page;

static long
sys_bpf(kernel_ulong_t cmd, kernel_ulong_t attr, kernel_ulong_t size)
{
	long rc = syscall(__NR_bpf, cmd, attr, size);
	errstr = sprintrc(rc);
	return rc;
}

# if VERBOSE
#  define print_extra_data(addr_, size_) print_quoted_hex((addr_), (size_))
# else
#  define print_extra_data(addr_, size_) printf("...")
#endif

# define TEST_BPF_(cmd_, cmd_str_,					\
		  init_first_, print_first_,				\
		  init_attr_, print_attr_)				\
	do {								\
		/* zero addr */						\
		sys_bpf(cmd_, 0, long_bits | sizeof(union bpf_attr));	\
		printf("bpf(%s, NULL, %u) = %s\n",			\
		       cmd_str_, sizeof_attr, errstr);			\
									\
		/* zero size */						\
		unsigned long addr = end_of_page - sizeof_attr;		\
		sys_bpf(cmd_, addr, long_bits);				\
		printf("bpf(%s, %#lx, 0) = %s\n",			\
		       cmd_str_, addr, errstr);				\
									\
		/* the first field only */				\
		unsigned int offset = init_first_(end_of_page);		\
		addr = end_of_page - offset;				\
		sys_bpf(cmd_, addr, offset);				\
		printf("bpf(%s, {", cmd_str_);				\
		print_first_(addr);					\
		printf("}, %u) = %s\n", offset, errstr);		\
									\
		/* efault after the first field */			\
		sys_bpf(cmd_, addr, offset + 1);			\
		printf("bpf(%s, %#lx, %u) = %s\n",			\
		       cmd_str_, addr, offset + 1, errstr);		\
									\
		/* the relevant part of union bpf_attr */		\
		offset = init_attr_(end_of_page);			\
		addr = end_of_page - offset;				\
		sys_bpf(cmd_, addr, offset);				\
		printf("bpf(%s, {", cmd_str_);				\
		print_attr_(addr);					\
		printf("}, %u) = %s\n", offset, errstr);		\
									\
		/* short read of the relevant part of union bpf_attr */	\
		sys_bpf(cmd_, addr + 1, offset);			\
		printf("bpf(%s, %#lx, %u) = %s\n",			\
		       cmd_str_, addr + 1, offset, errstr);		\
									\
		if (offset < sizeof_attr) {				\
			/* short read of the whole union bpf_attr */	\
			memmove((void *) end_of_page - sizeof_attr + 1,	\
				(void *) addr, offset);			\
			addr = end_of_page - sizeof_attr + 1;		\
			memset((void *) addr + offset, 0,		\
			       sizeof_attr - offset - 1);		\
			sys_bpf(cmd_, addr, sizeof_attr);		\
			printf("bpf(%s, %#lx, %u) = %s\n",		\
			       cmd_str_, addr, sizeof_attr, errstr);	\
									\
			/* the whole union bpf_attr */			\
			memmove((void *) end_of_page - sizeof_attr,	\
				(void *) addr, offset);			\
			addr = end_of_page - sizeof_attr;		\
			memset((void *) addr + offset, 0,		\
			       sizeof_attr - offset);			\
			sys_bpf(cmd_, addr, sizeof_attr);		\
			printf("bpf(%s, {", cmd_str_);			\
			print_attr_(addr);				\
			printf("}, %u) = %s\n", sizeof_attr, errstr);	\
									\
			/* non-zero bytes after the relevant part */	\
			fill_memory_ex((void *) addr + offset,		\
				       sizeof_attr - offset, '0', 10);	\
			sys_bpf(cmd_, addr, sizeof_attr);		\
			printf("bpf(%s, {", cmd_str_);			\
			print_attr_(addr);				\
			printf(", ");					\
			print_extra_data((void *) addr + offset,	\
					 sizeof_attr - offset);		\
			printf("}, %u) = %s\n", sizeof_attr, errstr);	\
		}							\
									\
		/* short read of the whole page */			\
		memmove((void *) end_of_page - page_size + 1,		\
			(void *) addr, offset);				\
		addr = end_of_page - page_size + 1;			\
		memset((void *) addr + offset, 0,			\
		       page_size - offset - 1);				\
		sys_bpf(cmd_, addr, page_size);				\
		printf("bpf(%s, %#lx, %u) = %s\n",			\
		       cmd_str_, addr, page_size, errstr);		\
									\
		/* the whole page */					\
		memmove((void *) end_of_page - page_size,		\
			(void *) addr, offset);				\
		addr = end_of_page - page_size;				\
		memset((void *) addr + offset, 0, page_size - offset);	\
		sys_bpf(cmd_, addr, page_size);				\
		printf("bpf(%s, {", cmd_str_);				\
		print_attr_(addr);					\
		printf("}, %u) = %s\n", page_size, errstr);		\
									\
		/* non-zero bytes after the whole union bpf_attr */	\
		fill_memory_ex((void *) addr + offset,			\
			       page_size - offset, '0', 10);		\
		sys_bpf(cmd_, addr, page_size);				\
		printf("bpf(%s, {", cmd_str_);				\
		print_attr_(addr);					\
		printf(", ");						\
		print_extra_data((void *) addr + offset,		\
				 page_size - offset);			\
		printf("}, %u) = %s\n", page_size, errstr);		\
									\
		/* more than a page */					\
		sys_bpf(cmd_, addr, page_size + 1);			\
		printf("bpf(%s, %#lx, %u) = %s\n",			\
		       cmd_str_, addr, page_size + 1, errstr);		\
	} while (0)							\
	/* End of TEST_BPF_ definition. */

# define TEST_BPF(cmd_)							\
	TEST_BPF_((cmd_), #cmd_,					\
		  init_ ## cmd_ ## _first, print_ ## cmd_ ## _first,	\
		  init_ ## cmd_ ## _attr, print_ ## cmd_ ## _attr)	\
	/* End of TEST_BPF definition. */

# ifdef HAVE_UNION_BPF_ATTR_INNER_MAP_FD

static unsigned int
init_BPF_MAP_CREATE_first(const unsigned long eop)
{
	static const union bpf_attr attr = { .map_type = 2 };
	static const unsigned int offset = sizeof(attr.map_type);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr.map_type, offset);
	return offset;
}

static void
print_BPF_MAP_CREATE_first(const unsigned long addr)
{
	printf("map_type=BPF_MAP_TYPE_ARRAY, key_size=0, value_size=0"
	       ", max_entries=0, map_flags=0, inner_map_fd=0");
}

static unsigned int
init_BPF_MAP_CREATE_attr(const unsigned long eop)
{
	static const union bpf_attr attr = {
		.map_type = 1,
		.key_size = 4,
		.value_size = 8,
		.max_entries = 256,
		.map_flags = 1,
		.inner_map_fd = -1
	};
	static const unsigned int offset =
		offsetofend(union bpf_attr, inner_map_fd);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_MAP_CREATE_attr(const unsigned long addr)
{
	printf("map_type=BPF_MAP_TYPE_HASH, key_size=4"
	       ", value_size=8, max_entries=256"
	       ", map_flags=BPF_F_NO_PREALLOC, inner_map_fd=-1");
}

# endif /* HAVE_UNION_BPF_ATTR_INNER_MAP_FD */

# ifdef HAVE_UNION_BPF_ATTR_FLAGS

static unsigned int
init_BPF_MAP_LOOKUP_ELEM_first(const unsigned long eop)
{
	static const union bpf_attr attr = { .map_fd = -1 };
	static const unsigned int offset = sizeof(attr.map_fd);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr.map_fd, offset);
	return offset;
}

static void
print_BPF_MAP_LOOKUP_ELEM_first(const unsigned long addr)
{
	printf("map_fd=-1, key=0, value=0");
}

static unsigned int
init_BPF_MAP_LOOKUP_ELEM_attr(const unsigned long eop)
{
	static const union bpf_attr attr = {
		.map_fd = -1,
		.key = 0xdeadbeef,
		.value = 0xbadc0ded
	};
	static const unsigned int offset =
		offsetofend(union bpf_attr, value);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_MAP_LOOKUP_ELEM_attr(const unsigned long addr)
{
	printf("map_fd=-1, key=0xdeadbeef, value=0xbadc0ded");
}

#  define init_BPF_MAP_UPDATE_ELEM_first init_BPF_MAP_LOOKUP_ELEM_first

static void
print_BPF_MAP_UPDATE_ELEM_first(const unsigned long addr)
{
	printf("map_fd=-1, key=0, value=0, flags=BPF_ANY");
}

static unsigned int
init_BPF_MAP_UPDATE_ELEM_attr(const unsigned long eop)
{
	static const union bpf_attr attr = {
		.map_fd = -1,
		.key = 0xdeadbeef,
		.value = 0xbadc0ded,
		.flags = 2
	};
	static const unsigned int offset =
		offsetofend(union bpf_attr, flags);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_MAP_UPDATE_ELEM_attr(const unsigned long addr)
{
	printf("map_fd=-1, key=0xdeadbeef, value=0xbadc0ded, flags=BPF_EXIST");
}

#  define init_BPF_MAP_DELETE_ELEM_first init_BPF_MAP_LOOKUP_ELEM_first

static void
print_BPF_MAP_DELETE_ELEM_first(const unsigned long addr)
{
	printf("map_fd=-1, key=0");
}

static unsigned int
init_BPF_MAP_DELETE_ELEM_attr(const unsigned long eop)
{
	static const union bpf_attr attr = {
		.map_fd = -1,
		.key = 0xdeadbeef
	};
	static const unsigned int offset =
		offsetofend(union bpf_attr, key);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_MAP_DELETE_ELEM_attr(const unsigned long addr)
{
	printf("map_fd=-1, key=0xdeadbeef");
}

#  define init_BPF_MAP_GET_NEXT_KEY_first init_BPF_MAP_LOOKUP_ELEM_first

static void
print_BPF_MAP_GET_NEXT_KEY_first(const unsigned long addr)
{
	printf("map_fd=-1, key=0, next_key=0");
}

static unsigned int
init_BPF_MAP_GET_NEXT_KEY_attr(const unsigned long eop)
{
	static const union bpf_attr attr = {
		.map_fd = -1,
		.key = 0xdeadbeef,
		.next_key = 0xbadc0ded
	};
	static const unsigned int offset =
		offsetofend(union bpf_attr, next_key);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_MAP_GET_NEXT_KEY_attr(const unsigned long addr)
{
	printf("map_fd=-1, key=0xdeadbeef, next_key=0xbadc0ded");
}

# endif /* HAVE_UNION_BPF_ATTR_FLAGS */

# ifdef HAVE_UNION_BPF_ATTR_PROG_FLAGS

static unsigned int
init_BPF_PROG_LOAD_first(const unsigned long eop)
{
	static const union bpf_attr attr = { .prog_type = 1 };
	static const unsigned int offset = sizeof(attr.prog_type);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr.prog_type, offset);
	return offset;
}

static void
print_BPF_PROG_LOAD_first(const unsigned long addr)
{

	printf("prog_type=BPF_PROG_TYPE_SOCKET_FILTER, insn_cnt=0, insns=0"
	       ", license=NULL, log_level=0, log_size=0, log_buf=0"
	       ", kern_version=0, prog_flags=0");
}

static const struct bpf_insn insns[] = {
	{ .code = BPF_JMP | BPF_EXIT }
};
static char log_buf[4096];

static unsigned int
init_BPF_PROG_LOAD_attr(const unsigned long eop)
{
	const union bpf_attr attr = {
		.prog_type = 1,
		.insn_cnt = ARRAY_SIZE(insns),
		.insns = (uintptr_t) insns,
		.license = (uintptr_t) "GPL",
		.log_level = 42,
		.log_size = sizeof(log_buf),
		.log_buf = (uintptr_t) log_buf,
		.kern_version = 0xcafef00d,
		.prog_flags = 1
	};
	static const unsigned int offset =
		offsetofend(union bpf_attr, prog_flags);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_PROG_LOAD_attr(const unsigned long addr)
{
	printf("prog_type=BPF_PROG_TYPE_SOCKET_FILTER, insn_cnt=%u, insns=%p"
	       ", license=\"GPL\", log_level=42, log_size=4096, log_buf=%p"
	       ", kern_version=%u, prog_flags=BPF_F_STRICT_ALIGNMENT",
	       (unsigned int) ARRAY_SIZE(insns), insns,
	       log_buf, 0xcafef00d);
}

# endif /* HAVE_UNION_BPF_ATTR_PROG_FLAGS */

/*
 * bpf() syscall and its first six commands were introduced in Linux kernel
 * 3.18. Some additional commands were added afterwards, so we need to take
 * precautions to make sure the tests compile.
 *
 * BPF_OBJ_PIN and BPF_OBJ_GET commands appear in kernel 4.4.
 */
# ifdef HAVE_UNION_BPF_ATTR_BPF_FD

static unsigned int
init_BPF_OBJ_PIN_first(const unsigned long eop)
{
	static const union bpf_attr attr = {};
	static const unsigned int offset = sizeof(attr.pathname);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr.pathname, offset);
	return offset;
}

static void
print_BPF_OBJ_PIN_first(const unsigned long addr)
{

	printf("pathname=NULL, bpf_fd=0");
}

static unsigned int
init_BPF_OBJ_PIN_attr(const unsigned long eop)
{
	const union bpf_attr attr = {
		.pathname = (uintptr_t) "/sys/fs/bpf/foo/bar",
		.bpf_fd = -1
	};
	static const unsigned int offset =
		offsetofend(union bpf_attr, bpf_fd);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_OBJ_PIN_attr(const unsigned long addr)
{
	printf("pathname=\"/sys/fs/bpf/foo/bar\", bpf_fd=-1");
}

#  define init_BPF_OBJ_GET_first init_BPF_OBJ_PIN_first
#  define print_BPF_OBJ_GET_first print_BPF_OBJ_PIN_first
#  define init_BPF_OBJ_GET_attr init_BPF_OBJ_PIN_attr
#  define print_BPF_OBJ_GET_attr print_BPF_OBJ_PIN_attr

# endif /* HAVE_UNION_BPF_ATTR_BPF_FD */

/* BPF_PROG_ATTACH and BPF_PROG_DETACH commands appear in kernel 4.10. */
# ifdef HAVE_UNION_BPF_ATTR_ATTACH_FLAGS

static unsigned int
init_BPF_PROG_ATTACH_first(const unsigned long eop)
{
	static const union bpf_attr attr = { .target_fd = -1 };
	static const unsigned int offset = sizeof(attr.target_fd);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr.target_fd, offset);
	return offset;
}

static void
print_BPF_PROG_ATTACH_first(const unsigned long addr)
{
	printf("target_fd=-1, attach_bpf_fd=0"
	       ", attach_type=BPF_CGROUP_INET_INGRESS, attach_flags=0");
}

static unsigned int
init_BPF_PROG_ATTACH_attr(const unsigned long eop)
{
	static const union bpf_attr attr = {
		.target_fd = -1,
		.attach_bpf_fd = -2,
		.attach_type = 2,
		.attach_flags = 1
	};
	static const unsigned int offset =
		offsetofend(union bpf_attr, attach_flags);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}

static void
print_BPF_PROG_ATTACH_attr(const unsigned long addr)
{
	printf("target_fd=-1, attach_bpf_fd=-2"
	       ", attach_type=BPF_CGROUP_INET_SOCK_CREATE"
	       ", attach_flags=BPF_F_ALLOW_OVERRIDE");
}

#  define init_BPF_PROG_DETACH_first init_BPF_PROG_ATTACH_first

static unsigned int
init_BPF_PROG_DETACH_attr(const unsigned long eop)
{
	static const union bpf_attr attr = {
		.target_fd = -1,
		.attach_type = 2
	};
	static const unsigned int offset =
		offsetofend(union bpf_attr, attach_type);
	const unsigned long addr = eop - offset;

	memcpy((void *) addr, &attr, offset);
	return offset;
}


static void
print_BPF_PROG_DETACH_first(const unsigned long addr)
{
	printf("target_fd=-1, attach_type=BPF_CGROUP_INET_INGRESS");
}

static void
print_BPF_PROG_DETACH_attr(const unsigned long addr)
{
	printf("target_fd=-1, attach_type=BPF_CGROUP_INET_SOCK_CREATE");
}

# endif /* HAVE_UNION_BPF_ATTR_ATTACH_FLAGS */

int
main(void)
{
	page_size = get_page_size();
	end_of_page = (unsigned long) tail_alloc(1) + 1;

# ifdef HAVE_UNION_BPF_ATTR_INNER_MAP_FD
	TEST_BPF(BPF_MAP_CREATE);
# endif

# ifdef HAVE_UNION_BPF_ATTR_FLAGS
	TEST_BPF(BPF_MAP_LOOKUP_ELEM);
	TEST_BPF(BPF_MAP_UPDATE_ELEM);
	TEST_BPF(BPF_MAP_DELETE_ELEM);
	TEST_BPF(BPF_MAP_GET_NEXT_KEY);
# endif

# ifdef HAVE_UNION_BPF_ATTR_PROG_FLAGS
	TEST_BPF(BPF_PROG_LOAD);
# endif

# ifdef HAVE_UNION_BPF_ATTR_BPF_FD
	TEST_BPF(BPF_OBJ_PIN);
	TEST_BPF(BPF_OBJ_GET);
# endif

# ifdef HAVE_UNION_BPF_ATTR_ATTACH_FLAGS
	TEST_BPF(BPF_PROG_ATTACH);
	TEST_BPF(BPF_PROG_DETACH);
# endif

	sys_bpf(0xfacefeed, end_of_page, 40);
	printf("bpf(0xfacefeed /* BPF_??? */, %#lx, 40) = %s\n",
	       end_of_page, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_bpf && HAVE_UNION_BPF_ATTR_*")

#endif
