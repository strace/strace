/*
 * Check decoding of PERF_EVENT_IOC_* commands of ioctl syscall.
 *
 * Copyright (c) 2018-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>

#define STR16 "0123456789abcdef"

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
	printf("ioctl(-1, _IOC(%s_IOC_READ|_IOC_WRITE, 0x24, %#x, %#x), %#lx)"
	       RVAL_EBADF,
	       _IOC_DIR((unsigned int) unknown_perf_cmd) & _IOC_NONE ?
	       "_IOC_NONE|" : "",
	       _IOC_NR((unsigned int) unknown_perf_cmd),
	       _IOC_SIZE((unsigned int) unknown_perf_cmd),
	       (unsigned long) magic);

	sys_ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES + 1, magic);
	printf("ioctl(-1, _IOC(_IOC_WRITE, 0x24, %#x, %#x), %#lx)" RVAL_EBADF,
	       (unsigned int) _IOC_NR(PERF_EVENT_IOC_MODIFY_ATTRIBUTES + 1),
	       (unsigned int) _IOC_SIZE(PERF_EVENT_IOC_MODIFY_ATTRIBUTES + 1),
	       (unsigned long) magic);

	/* PERF_EVENT_IOC_{ENABLE,DISABLE,RESET} */
	for (unsigned i = 0; i < ARRAY_SIZE(flag_iocs); i++) {
		ioctl(-1, flag_iocs[i].cmd, 0);
		printf("ioctl(-1, %s, 0)" RVAL_EBADF, flag_iocs[i].str);

		ioctl(-1, flag_iocs[i].cmd, 1);
		printf("ioctl(-1, %s, PERF_IOC_FLAG_GROUP)" RVAL_EBADF,
		       flag_iocs[i].str);

		ioctl(-1, flag_iocs[i].cmd, 2);
		printf("ioctl(-1, %s, 0x2 /* PERF_IOC_FLAG_??? */)" RVAL_EBADF,
		       flag_iocs[i].str);

		sys_ioctl(-1, flag_iocs[i].cmd, magic);
		printf("ioctl(-1, %s, PERF_IOC_FLAG_GROUP|%#x)" RVAL_EBADF,
		       flag_iocs[i].str, (unsigned int) magic & ~1U);
	}

	/* PERF_EVENT_IOC_REFRESH */
	sys_ioctl(-1, PERF_EVENT_IOC_REFRESH, magic);
	printf("ioctl(-1, PERF_EVENT_IOC_REFRESH, %d)" RVAL_EBADF,
	       (int) magic);

	/* PERF_EVENT_IOC_PERIOD */
	ioctl(-1, PERF_EVENT_IOC_PERIOD, NULL);
	printf("ioctl(-1, PERF_EVENT_IOC_PERIOD, NULL)" RVAL_EBADF);

	ioctl(-1, PERF_EVENT_IOC_PERIOD, u64_efault);
	printf("ioctl(-1, PERF_EVENT_IOC_PERIOD, %p)" RVAL_EBADF,
	      u64_efault);

	ioctl(-1, PERF_EVENT_IOC_PERIOD, u64_ptr);
	printf("ioctl(-1, PERF_EVENT_IOC_PERIOD, [%" PRIu64 "])" RVAL_EBADF,
	       magic64);

	/* PERF_EVENT_IOC_SET_OUTPUT */
	sys_ioctl(-1, PERF_EVENT_IOC_SET_OUTPUT, magic);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_OUTPUT, %d)" RVAL_EBADF,
	       (int) magic);

	/* PERF_EVENT_IOC_SET_FILTER */
	ioctl(-1, PERF_EVENT_IOC_SET_FILTER, NULL);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_FILTER, NULL)" RVAL_EBADF);

	ioctl(-1, PERF_EVENT_IOC_SET_FILTER, str_efault);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_FILTER, %p)" RVAL_EBADF,
	       str_efault);

	ioctl(-1, PERF_EVENT_IOC_SET_FILTER, str_ptr);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_FILTER, \"%.32s\"...)" RVAL_EBADF,
	       str_ptr);

	ioctl(-1, PERF_EVENT_IOC_SET_FILTER, str_ptr + 40);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_FILTER, \"%.32s\")" RVAL_EBADF,
	       str_ptr + 40);

	str_ptr[sizeof(str) - 1] = '0';
	ioctl(-1, PERF_EVENT_IOC_SET_FILTER, str_ptr + 40);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_FILTER, %p)" RVAL_EBADF,
	       str_ptr + 40);

	/* PERF_EVENT_IOC_ID */
	ioctl(-1, PERF_EVENT_IOC_ID, NULL);
	printf("ioctl(-1, PERF_EVENT_IOC_ID, NULL)" RVAL_EBADF);

	ioctl(-1, PERF_EVENT_IOC_ID, u64_efault);
	printf("ioctl(-1, PERF_EVENT_IOC_ID, %p)" RVAL_EBADF,
	      u64_efault);

	ioctl(-1, PERF_EVENT_IOC_ID, u64_ptr);
	printf("ioctl(-1, PERF_EVENT_IOC_ID, %p)" RVAL_EBADF,
	       u64_ptr);

	/* PERF_EVENT_IOC_SET_BPF */
	sys_ioctl(-1, PERF_EVENT_IOC_SET_BPF, magic);
	printf("ioctl(-1, PERF_EVENT_IOC_SET_BPF, %d)" RVAL_EBADF,
	       (int) magic);

	/* PERF_EVENT_IOC_PAUSE_OUTPUT */
	sys_ioctl(-1, PERF_EVENT_IOC_PAUSE_OUTPUT, magic);
	printf("ioctl(-1, PERF_EVENT_IOC_PAUSE_OUTPUT, %lu)" RVAL_EBADF,
	       (unsigned long) magic);

	/* PERF_EVENT_IOC_QUERY_BPF */
	ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, NULL);
	printf("ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, NULL)" RVAL_EBADF);

	ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, u32_efault);
	printf("ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, %p)" RVAL_EBADF,
	       u32_efault);

	u32_arr[0] = 0xbadc0ded;
	ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, u32_arr);
	printf("ioctl(-1, PERF_EVENT_IOC_QUERY_BPF, {ids_len=3134983661, ...})"
	       RVAL_EBADF);

	/* PERF_EVENT_IOC_MODIFY_ATTRIBUTES */
	ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES, NULL);
	printf("ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES, NULL)" RVAL_EBADF);

	ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES, pea_ptr + 1);
	printf("ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES, %p)" RVAL_EBADF,
	       pea_ptr + 1);

	printf("ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES"
	       ", {type=%#x /* PERF_TYPE_??? */"
	       ", size=%#x /* PERF_ATTR_SIZE_??? */"
	       ", config=%#llx, sample_period=%llu%s, ...})" RVAL_EBADF,
	       (unsigned int) pea_ptr->type,
	       (unsigned int) pea_ptr->size,
	       (unsigned long long) pea_ptr->config,
	       (unsigned long long) pea_ptr->sample_period,
#ifdef WORDS_BIGENDIAN
	       ", sample_type=PERF_SAMPLE_IP|PERF_SAMPLE_ADDR|PERF_SAMPLE_ID|"
	       "PERF_SAMPLE_CPU|PERF_SAMPLE_BRANCH_STACK|PERF_SAMPLE_WEIGHT|"
	       "PERF_SAMPLE_DATA_SRC|PERF_SAMPLE_IDENTIFIER|"
	       "PERF_SAMPLE_TRANSACTION|PERF_SAMPLE_REGS_INTR|"
	       "PERF_SAMPLE_DATA_PAGE_SIZE|PERF_SAMPLE_CODE_PAGE_SIZE|"
	       "0xc2c3c4c5c6000000"
	       ", read_format=PERF_FORMAT_TOTAL_TIME_ENABLED|PERF_FORMAT_LOST|"
	       "0xcacbcccdcecfd0c0"
	       ", disabled=1, inherit=1, exclusive=1, exclude_hv=1, mmap=1"
	       ", comm=1, inherit_stat=1, watermark=1"
	       ", precise_ip=3 /* must have 0 skid */, mmap_data=1"
	       ", exclude_host=1, exclude_callchain_kernel=1, comm_exec=1"
	       ", use_clockid=1, write_backward=1, ksymbol=1, aux_output=1"
	       ", cgroup=1, text_poke=1, inherit_thread=1, sigtrap=1"
	       ", __reserved_1=0x2d7d8d9 /* Bits 63..38 */"
#else
	       ", sample_type=PERF_SAMPLE_TID|PERF_SAMPLE_ID|PERF_SAMPLE_CPU|"
	       "PERF_SAMPLE_PERIOD|PERF_SAMPLE_STREAM_ID|PERF_SAMPLE_WEIGHT|"
	       "PERF_SAMPLE_DATA_SRC|PERF_SAMPLE_REGS_INTR|"
	       "PERF_SAMPLE_DATA_PAGE_SIZE|PERF_SAMPLE_CODE_PAGE_SIZE|"
	       "PERF_SAMPLE_WEIGHT_STRUCT|0xc9c8c7c6c4000000"
	       ", read_format=PERF_FORMAT_TOTAL_TIME_RUNNING|PERF_FORMAT_GROUP|"
	       "0xd1d0cfcecdcccbc0"
	       ", inherit=1, exclude_user=1, exclude_hv=1, exclude_idle=1"
	       ", mmap=1, comm=1, enable_on_exec=1, watermark=1"
	       ", precise_ip=1 /* constant skid */, sample_id_all=1"
	       ", exclude_guest=1, exclude_callchain_user=1, mmap2=1"
	       ", comm_exec=1, context_switch=1, namespaces=1, bpf_event=1"
	       ", aux_output=1, text_poke=1, build_id=1, remove_on_exec=1"
	       ", __reserved_1=0x367635f /* Bits 63..38 */"
#endif
	       );
	ioctl(-1, PERF_EVENT_IOC_MODIFY_ATTRIBUTES, pea_ptr);

	puts("+++ exited with 0 +++");
	return 0;
}
