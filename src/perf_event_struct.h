/*
 * Copyright (c) 2016-2018 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_LINUX_PERF_EVENT_STRUCT_H
# define STRACE_LINUX_PERF_EVENT_STRUCT_H

# include <stdint.h>

struct perf_event_attr {
	uint32_t type;
	uint32_t size;
	uint64_t config;
	union {
		uint64_t sample_period;
		uint64_t sample_freq;
	};
	uint64_t sample_type;
	uint64_t read_format;
	uint64_t disabled			:1,
		 inherit			:1,
		 pinned				:1,
		 exclusive			:1,
		 exclude_user			:1,
		 exclude_kernel			:1,
		 exclude_hv			:1,
		 exclude_idle			:1,
		 mmap				:1,
		 comm				:1,
		 freq				:1,
		 inherit_stat			:1,
		 enable_on_exec			:1,
		 task				:1,
		 watermark			:1,
		 precise_ip			:2,
		 mmap_data			:1,
		 sample_id_all			:1,
		 exclude_host			:1,
		 exclude_guest			:1,
		 exclude_callchain_kernel	:1,
		 exclude_callchain_user		:1,
		 mmap2				:1,
		 comm_exec			:1,
		 use_clockid			:1,
		 context_switch			:1,
		 write_backward			:1,
		 namespaces			:1,
		 __reserved_1			:35;
	union {
		uint32_t wakeup_events;
		uint32_t wakeup_watermark;
	};
	uint32_t bp_type;
	union {
		uint64_t bp_addr;
		uint64_t config1;
	};
	/* End of ver 0 - 64 bytes */
	union {
		uint64_t bp_len;
		uint64_t config2;
	};
	/* End of ver 1 - 72 bytes */
	uint64_t branch_sample_type;
	/* End of ver 2 - 80 bytes */
	uint64_t sample_regs_user;
	uint32_t sample_stack_user;
	int32_t  clockid;
	/* End of ver 3 - 96 bytes */
	uint64_t sample_regs_intr;
	/* End of ver 4 - 104 bytes */
	uint32_t aux_watermark;
	uint16_t sample_max_stack;
	uint16_t __reserved_2;
	/* End of ver 5 - 112 bytes */
	uint32_t aux_sample_size;
	uint32_t __reserved_3;
	/* End of ver 6 - 120 bytes */
};

struct perf_event_query_bpf {
        uint32_t ids_len;
        uint32_t prog_cnt;
        uint32_t ids[0];
};

#endif /* !STRACE_LINUX_PERF_EVENT_STRUCT_H */
