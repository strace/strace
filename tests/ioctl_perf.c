/*
 * Check decoding of PERF_EVENT_IOC_* commands of ioctl syscall.
 *
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_LINUX_PERF_EVENT_H

# include <inttypes.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>
# include <sys/ioctl.h>
# include "scno.h"
# include <linux/perf_event.h>

/*
 * Workaround the bug in kernel UAPI that was fixed
 * in Linux commit v2.6.33-rc1~48^2~288^2~19.
 */
# ifndef u64
#  define u64 uint64_t
# endif

# define XLAT_MACROS_ONLY
# include "xlat/perf_ioctl_cmds.h"
# undef XLAT_MACROS_ONLY

# define STR16 "0123456789abcdef"

static long
sys_ioctl(kernel_long_t fd, kernel_ulong_t cmd, kernel_ulong_t arg)
{
	return syscall(__NR_ioctl, fd, cmd, arg);
}

int
main(void)
{
	static const kernel_ulong_t unknown_perf_cmd =
		(kernel_ulong_t) 0xbadc0dedfeed24edULL;
	static const kernel_ulong_t magic =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;
	static const uint64_t magic64 = 0xfacefeeddeadc0deULL;
	static const char str[] = STR16 STR16 STR16 STR16;

	static struct {
		unsigned int cmd;
		const char *str;
	} flag_iocs[] = {
		{ ARG_STR(PERF_EVENT_IOC_ENABLE) },
		{ ARG_STR(PERF_EVENT_IOC_DISABLE) },
		{ ARG_STR(PERF_EVENT_IOC_RESET) },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, u64_ptr);
	uint64_t *const u64_efault = u64_ptr + 1;
	uint32_t *const u32_arr = tail_alloc(sizeof(uint32_t) * 4);
	uint32_t *const u32_efault = u32_arr + 4;
	char *const str_ptr = tail_memdup(str, sizeof(str));
	char *const str_efault = str_ptr + sizeof(str);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct perf_event_attr, pea_ptr);

	*u64_ptr = magic64;
	fill_memory_ex(pea_ptr, sizeof(*pea_ptr), 0xaa, 0x55);

	/* Unknown perf commands */
	sys_ioctl(-1, unknown_perf_cmd, magic);
	printf("ioctl(-1, _IOC(%s_IOC_READ|_IOC_WRITE, 0x24, %#x, %#x), "
	       "%#lx) = -1 EBADF (%m)\n",
	       _IOC_DIR((unsigned int) unknown_perf_cmd) & _IOC_NONE ?
	       "_IOC_NONE|" : "",
	       _IOC_NR((unsigned int) unknown_perf_cmd),
	       _IOC_SIZE((unsigned int) unknown_perf_cmd),
	       (unsigned long) magic);

	sys_ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES + 1, magic);
	printf("ioctl(-1, _IOC(_IOC_WRITE, 0x24, %#x, %#x), %#lx)"
	       " = -1 EBADF (%m)\n",
	       (unsigned int) _IOC_NR(PERF_EVENT_IOC_MODIFY_ATTRIBUTES + 1),
	       (unsigned int) _IOC_SIZE(PERF_EVENT_IOC_MODIFY_ATTRIBUTES + 1),
	       (unsigned long) magic);

	/* PERF_EVENT_IOC_{ENABLE,DISABLE,RESET} */
	for (unsigned i = 0; i < ARRAY_SIZE(flag_iocs); i++) {
		ioctl(-1, flag_iocs[i].cmd, 0);
		printf("ioctl(-1, %s, 0) = -1 EBADF (%m)\n", flag_iocs[i].str);

		ioctl(-1, flag_iocs[i].cmd, 1);
		printf("ioctl(-1, %s, PERF_IOC_FLAG_GROUP) = -1 EBADF (%m)\n",
		       flag_iocs[i].str);

		ioctl(-1, flag_iocs[i].cmd, 2);
		printf("ioctl(-1, %s, 0x2 /* PERF_IOC_FLAG_??? */) "
		       "= -1 EBADF (%m)\n",
		       flag_iocs[i].str);

		sys_ioctl(-1, flag_iocs[i].cmd, magic);
		printf("ioctl(-1, %s, PERF_IOC_FLAG_GROUP|%#x) "
		       "= -1 EBADF (%m)\n",
		       flag_iocs[i].str, (unsigned int) magic & ~1U);
	}

	/* PERF_EVENT_IOC_REFRESH */
	sys_ioctl(-1, PERF_EVENT_IOC_REFRESH, magic);
	printf("ioctl(-1, PERF_EVENT_IOC_REFRESH, %d) = -1 EBADF (%m)\n",
	       (int) magic);

	/* PERF_EVENT_IOC_PERIOD */
	ioctl(-1, PERF_EVENT_IOC_PERIOD, NULL);
	printf("ioctl(-1, PERF_EVENT_IOC_PERIOD, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, PERF_EVENT_IOC_PERIOD, u64_efault);
	printf("ioctl(-1, PERF_EVENT_IOC_PERIOD, %p) = -1 EBADF (%m)\n",
	      u64_efault);

	ioctl(-1, PERF_EVENT_IOC_PERIOD, u64_ptr);
	printf("ioctl(-1, PERF_EVENT_IOC_PERIOD, [%" PRIu64 "])"
	       " = -1 EBADF (%m)\n",
	       magic64);

	/* PERF_EVENT_IOC_SET_OUTPUT */
	sys_ioctl(-1, PERF_EVENT_IOC_SET_OUTPUT, magic);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_OUTPUT, %d) = -1 EBADF (%m)\n",
	       (int) magic);

	/* PERF_EVENT_IOC_SET_FILTER */
	ioctl(-1, PERF_EVENT_IOC_SET_FILTER, NULL);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_FILTER, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, PERF_EVENT_IOC_SET_FILTER, str_efault);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_FILTER, %p) = -1 EBADF (%m)\n",
	       str_efault);

	ioctl(-1, PERF_EVENT_IOC_SET_FILTER, str_ptr);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_FILTER, \"%.32s\"...)"
	       " = -1 EBADF (%m)\n",
	       str_ptr);

	ioctl(-1, PERF_EVENT_IOC_SET_FILTER, str_ptr + 40);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_FILTER, \"%.32s\")"
	       " = -1 EBADF (%m)\n",
	       str_ptr + 40);

	str_ptr[sizeof(str) - 1] = '0';
	ioctl(-1, PERF_EVENT_IOC_SET_FILTER, str_ptr + 40);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_FILTER, %p)"
	       " = -1 EBADF (%m)\n",
	       str_ptr + 40);

	/* PERF_EVENT_IOC_ID */
	ioctl(-1, PERF_EVENT_IOC_ID, NULL);
	printf("ioctl(-1, PERF_EVENT_IOC_ID, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, PERF_EVENT_IOC_ID, u64_efault);
	printf("ioctl(-1, PERF_EVENT_IOC_ID, %p) = -1 EBADF (%m)\n",
	      u64_efault);

	ioctl(-1, PERF_EVENT_IOC_ID, u64_ptr);
	printf("ioctl(-1, PERF_EVENT_IOC_ID, %p) = -1 EBADF (%m)\n",
	       u64_ptr);

	/* PERF_EVENT_IOC_SET_BPF */
	sys_ioctl(-1, PERF_EVENT_IOC_SET_BPF, magic);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_BPF, %d) = -1 EBADF (%m)\n",
	       (int) magic);

	/* PERF_EVENT_IOC_PAUSE_OUTPUT */
	sys_ioctl(-1, PERF_EVENT_IOC_PAUSE_OUTPUT, magic);
	printf("ioctl(-1, PERF_EVENT_IOC_PAUSE_OUTPUT, %lu) = -1 EBADF (%m)\n",
	       (unsigned long) magic);

	/* PERF_EVENT_IOC_QUERY_BPF */
	ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, NULL);
	printf("ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, u32_efault);
	printf("ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, %p) = -1 EBADF (%m)\n",
	       u32_efault);

	u32_arr[0] = 0xbadc0ded;
	ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, u32_arr);
	printf("ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, {ids_len=3134983661, ...})"
	       " = -1 EBADF (%m)\n");

	/* PERF_EVENT_IOC_MODIFY_ATTRIBUTES */
	ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES, NULL);
	printf("ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES, NULL)"
	       " = -1 EBADF (%m)\n");

	ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES, pea_ptr + 1);
	printf("ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES, %p)"
	       " = -1 EBADF (%m)\n",
	       pea_ptr + 1);

	printf("ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES"
	       ", {type=%#x /* PERF_TYPE_??? */"
	       ", size=%#x /* PERF_ATTR_SIZE_??? */"
	       ", config=%#llx, ...}) = -1 EBADF (%m)\n",
	       (unsigned int) pea_ptr->type,
	       (unsigned int) pea_ptr->size,
	       (unsigned long long) pea_ptr->config);
	ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES, pea_ptr);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_LINUX_PERF_EVENT_H");

#endif
