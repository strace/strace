/*
 * Check verbose decoding of perf_event_open syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined(__NR_perf_event_open) && defined(HAVE_LINUX_PERF_EVENT_H)

# include <inttypes.h>
# include <limits.h>
# include <stddef.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>

# include <linux/perf_event.h>

# include "xlat.h"
# include "xlat/perf_event_open_flags.h"
# include "xlat/perf_attr_size.h"

# if ULONG_MAX > UINT_MAX /* Poor man's "whether long is 8 bytes?" */
#  define LONG_STR_PREFIX "ffffffff"
# else /* !(ULONG_MAX > UINT_MAX) */
#  define LONG_STR_PREFIX ""
# endif /* ULONG_MAX > UINT_MAX */

# ifndef PERF_TYPE_BREAKPOINT
#  define PERF_TYPE_BREAKPOINT 5
# endif

struct s32_val_str {
	int32_t val;
	const char *str;
};

struct u32_val_str {
	uint32_t val;
	const char *str;
};

struct u64_val_str {
	uint64_t val;
	const char *str;
};

/* In order to avoid endianness-specific hackery. */
struct pea_flags {
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
};

static const char *
printaddr(void *ptr)
{
	static char buf[sizeof("0x") + sizeof(void *) * 2];

	if (ptr == NULL)
		return "NULL";

	snprintf(buf, sizeof(buf), "%#lx", (unsigned long)ptr);

	return buf;
}

/*
 * Checklist:
 *
 * type - 8 IDs
 * config - 13 IDs (0..11 + random), depends on type
 * sample type - bitmask, up to 20 bits
 * read_format - 5 IDs
 * bp_type - 6, weird semantics (invalid/unknown)
 * branch_sample_type - bitmask, 16 bits
 * clockid - 13 values
 *
 * Unions:
 * sample_period/sample_freq
 * wakeup_event/wakeup_watermark
 * bp_addr/config1
 * bp_len/config2
 */

/*
 * The main idea behind all those numerous ifdefs is checking against version of
 * structure provided in kernel headers and not use one defined in strace
 * headers (assume the case when suddenly we add flag without proper update of
 * __reserved_1 field or something like this).
 */
static void
print_event_attr(struct perf_event_attr *attr_ptr, size_t size,
	const char *type, const char *config, const char *sample_type,
	const char *read_format, const char *precise_ip_desc,
	const char *bp_type, const char *branch_sample_type,
	const char *clockid, uint32_t available_size)
{
	/*
	 * Currently, strace supports version 5 of the structure, which is
	 * 112 bytes in size.
	 */
	enum {
		STRACE_PEA_ABBREV_SIZE =
			offsetof(struct perf_event_attr, config) +
			sizeof(attr_ptr->config),
		STRACE_PEA_SIZE = 120,
	};

	uint32_t read_size;
	struct perf_event_attr *attr;
# if VERBOSE
	uint32_t cutoff;
	uint64_t val;
	uint64_t use_clockid;
	union {
		struct pea_flags flags;
		uint64_t raw;
	} flags_data;
# endif

	read_size =
# if !VERBOSE
		STRACE_PEA_ABBREV_SIZE;
# else
		size < STRACE_PEA_SIZE ?
			(size ? size : PERF_ATTR_SIZE_VER0) : STRACE_PEA_SIZE;
# endif

	if (read_size > available_size) {
		printf("%s", printaddr(attr_ptr));
		return;
	}

	/*
	 * Replicate kernel's behaviour regarding copying structure from
	 * userspace.
	 */
	attr = calloc(1, STRACE_PEA_SIZE);

	if (!attr)
		error_msg_and_fail("calloc");


	memcpy(attr, attr_ptr, read_size);

	if (size && (size < PERF_ATTR_SIZE_VER0)) {
		printf("%s", printaddr(attr_ptr));
		free(attr);
		return;
	}

	printf("{type=%s, size=", type);
	if (size != attr->size) {
		printxval(perf_attr_size, size, "PERF_ATTR_SIZE_???");
		printf(" => ");
	}
	printxval(perf_attr_size, attr->size, "PERF_ATTR_SIZE_???");
	printf(", config=%s, ", config);

	if (!size)
		size = PERF_ATTR_SIZE_VER0;

# if !VERBOSE
	printf("...}");
# else /* !VERBOSE */
	printf("%s=%" PRI__u64", sample_type=%s, read_format=%s",
	       attr->freq ? "sample_freq" : "sample_period",
	       attr->freq ? attr->sample_freq : attr->sample_period,
	       sample_type, read_format);

	printf(", disabled=%u"
	       ", inherit=%u"
	       ", pinned=%u"
	       ", exclusive=%u"
	       ", exclusive_user=%u"
	       ", exclude_kernel=%u"
	       ", exclude_hv=%u"
	       ", exclude_idle=%u"
	       ", mmap=%u"
	       ", comm=%u"
	       ", freq=%u"
	       ", inherit_stat=%u"
	       ", enable_on_exec=%u"
	       ", task=%u"
	       ", watermark=%u",
	       attr->disabled,
	       attr->inherit,
	       attr->pinned,
	       attr->exclusive,
	       attr->exclude_user,
	       attr->exclude_kernel,
	       attr->exclude_hv,
	       attr->exclude_idle,
	       attr->mmap,
	       attr->comm,
	       attr->freq,
	       attr->inherit_stat,
	       attr->enable_on_exec,
	       attr->task,
	       attr->watermark);

	flags_data.raw = ((uint64_t *) attr)[5];

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_PRECISE_IP
		attr->precise_ip;
#  else
		flags_data.flags.precise_ip;
#  endif
	printf(", precise_ip=%" PRIu64 " /* %s */", val, precise_ip_desc);

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_MMAP_DATA
		attr->mmap_data;
#  else
		flags_data.flags.mmap_data;
#  endif
	printf(", mmap_data=%" PRIu64, val);

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_SAMPLE_ID_ALL
		attr->sample_id_all;
#  else
		flags_data.flags.sample_id_all;
#  endif
	printf(", sample_id_all=%" PRIu64, val);

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_EXCLUDE_HOST
		attr->exclude_host;
#  else
		flags_data.flags.exclude_host;
#  endif
	printf(", exclude_host=%" PRIu64, val);

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_EXCLUDE_GUEST
		attr->exclude_guest;
#  else
		flags_data.flags.exclude_guest;
#  endif
	printf(", exclude_guest=%" PRIu64, val);

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_EXCLUDE_CALLCHAIN_KERNEL
		attr->exclude_callchain_kernel;
#  else
		flags_data.flags.exclude_callchain_kernel;
#  endif
	printf(", exclude_callchain_kernel=%" PRIu64, val);

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_EXCLUDE_CALLCHAIN_USER
		attr->exclude_callchain_user;
#  else
		flags_data.flags.exclude_callchain_user;
#  endif
	printf(", exclude_callchain_user=%" PRIu64, val);

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_MMAP2
		attr->mmap2;
#  else
		flags_data.flags.mmap2;
#  endif
	printf(", mmap2=%" PRIu64, val);

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_COMM_EXEC
		attr->comm_exec;
#  else
		flags_data.flags.comm_exec;
#  endif
	printf(", comm_exec=%" PRIu64, val);

	use_clockid = val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_USE_CLOCKID
		attr->use_clockid;
#  else
		flags_data.flags.use_clockid;
#  endif
	printf(", use_clockid=%" PRIu64, val);

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_CONTEXT_SWITCH
		attr->context_switch;
#  else
		flags_data.flags.context_switch;
#  endif
	printf(", context_switch=%" PRIu64, val);

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_WRITE_BACKWARD
		attr->write_backward;
#  else
		flags_data.flags.write_backward;
#  endif
	printf(", write_backward=%" PRIu64, val);

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_NAMESPACES
		attr->namespaces;
#  else
		flags_data.flags.namespaces;
#  endif
	printf(", namespaces=%" PRIu64, val);

	val = flags_data.flags.__reserved_1;
	if (val)
		printf(", __reserved_1=%#" PRIx64 " /* Bits 63..29 */", val);

	printf(", %s=%u",
		attr->watermark ? "wakeup_watermark" : "wakeup_events",
		attr->watermark ? attr->wakeup_watermark : attr->wakeup_events);

	if (attr->type == PERF_TYPE_BREAKPOINT)
		printf(", bp_type=%s", bp_type);

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_CONFIG1
		attr->config1;
#  else
		((uint64_t *) attr)[56 / sizeof(uint64_t)];
#  endif
	printf(", %s=%#" PRIx64,
	       attr->type == PERF_TYPE_BREAKPOINT ? "bp_addr" : "config1",
	       val);

	/* End of version 0 of the structure */
	if (size <= 64) {
		cutoff = 64;
		goto end;
	}

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_CONFIG2
		attr->config2;
#  else
		((uint64_t *) attr)[64 / sizeof(uint64_t)];
#  endif
	if (attr->type == PERF_TYPE_BREAKPOINT)
		printf(", bp_len=%" PRIu64, val);
	else
		printf(", config2=%#" PRIx64, val);

	/* End of version 1 of the structure */
	if (size <= 72) {
		cutoff = 72;
		goto end;
	}

	/*
	 * Print branch sample type only in case  PERF_SAMPLE_BRANCH_STACK
	 * is set in the sample_type field.
	 */
	if (attr->sample_type & (1 << 11))
		printf(", branch_sample_type=%s", branch_sample_type);

	/* End of version 2 of the structure */
	if (size <= 80) {
		cutoff = 80;
		goto end;
	}

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_SAMPLE_REGS_USER
		attr->sample_regs_user;
#  else
		((uint64_t *) attr)[80 / sizeof(uint64_t)];
#  endif
	printf(", sample_regs_user=%#" PRIx64, val);

	if (size <= 88) {
		cutoff = 88;
		goto end;
	}

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_SAMPLE_STACK_USER
		attr->sample_stack_user;
#  else
		((uint32_t *) attr)[88 / sizeof(uint32_t)];
#  endif
	/*
	 * Print branch sample type only in case PERF_SAMPLE_STACK_USER
	 * is set in the sample_type field.
	 */
	if (attr->sample_type & (1 << 13))
		printf(", sample_stack_user=%#" PRIx32, (uint32_t) val);

	if (size <= 92) {
		cutoff = 92;
		goto end;
	}

	if (use_clockid)
		printf(", clockid=%s", clockid);

	/* End of version 3 of the structure */
	if (size <= 96) {
		cutoff = 96;
		goto end;
	}

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_SAMPLE_REGS_INTR
		attr->sample_regs_intr;
#  else
		((uint64_t *) attr)[96 / sizeof(uint64_t)];
#  endif
	printf(", sample_regs_intr=%#" PRIx64, val);

	/* End of version 4 of the structure */
	if (size <= 104) {
		cutoff = 104;
		goto end;
	}

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_AUX_WATERMARK
		attr->aux_watermark;
#  else
		((uint32_t *) attr)[104 / sizeof(uint32_t)];
#  endif
	printf(", aux_watermark=%" PRIu32, (uint32_t) val);

	if (size <= 108) {
		cutoff = 108;
		goto end;
	}

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_SAMPLE_MAX_STACK
		attr->sample_max_stack;
#  else
		((uint16_t *) attr)[108 / sizeof(uint16_t)];
#  endif
	printf(", sample_max_stack=%" PRIu16, (uint16_t) val);

	if (size <= 110) {
		cutoff = 110;
		goto end;
	}

	val = ((uint16_t *) attr)[110 / sizeof(uint16_t)];
	if (val)
		printf(" /* bytes 110..111: %#" PRIx16 " */", (uint16_t) val);

	if (size <= 112) {
		cutoff = 112;
		goto end;
	}

	val =
#  ifdef HAVE_STRUCT_PERF_EVENT_ATTR_AUX_SAMPLE_SIZE
		attr->aux_sample_size;
#  else
		((uint32_t *) attr)[112 / sizeof(uint32_t)];
#  endif
	printf(", aux_sample_size=%" PRIu32, (uint32_t) val);

	if (size <= 116) {
		cutoff = 116;
		goto end;
	}

	cutoff = STRACE_PEA_SIZE;

end:
	if (size > cutoff)
		printf(", ...");

	printf("}");
# endif /* !VERBOSE */

	free(attr);
}

/* These require aligned access, so no byte-grain checks possible */
# if defined SPARC || defined SPARC64 || defined POWERPC || defined POWERPC64 || defined ARM
#  define ATTR_REC(sz) { tail_alloc((sz + 7) & ~7), sz }
# else
#  define ATTR_REC(sz) { tail_alloc(sz), sz }
# endif

# define BRANCH_TYPE_ALL \
	"PERF_SAMPLE_BRANCH_USER|" \
	"PERF_SAMPLE_BRANCH_KERNEL|" \
	"PERF_SAMPLE_BRANCH_HV|" \
	"PERF_SAMPLE_BRANCH_ANY|" \
	"PERF_SAMPLE_BRANCH_ANY_CALL|" \
	"PERF_SAMPLE_BRANCH_ANY_RETURN|" \
	"PERF_SAMPLE_BRANCH_IND_CALL|" \
	"PERF_SAMPLE_BRANCH_ABORT_TX|" \
	"PERF_SAMPLE_BRANCH_IN_TX|" \
	"PERF_SAMPLE_BRANCH_NO_TX|" \
	"PERF_SAMPLE_BRANCH_COND|" \
	"PERF_SAMPLE_BRANCH_CALL_STACK|" \
	"PERF_SAMPLE_BRANCH_IND_JUMP|" \
	"PERF_SAMPLE_BRANCH_CALL|" \
	"PERF_SAMPLE_BRANCH_NO_FLAGS|" \
	"PERF_SAMPLE_BRANCH_NO_CYCLES|" \
	"PERF_SAMPLE_BRANCH_TYPE_SAVE|" \
	"PERF_SAMPLE_BRANCH_HW_INDEX"

int
main(void)
{
	static const size_t attr_small_size = PERF_ATTR_SIZE_VER0 - 8;
	static const size_t attr_v0_size = PERF_ATTR_SIZE_VER0;
	static const size_t attr_v1_size = PERF_ATTR_SIZE_VER1;
	static const size_t attr_v2_size = PERF_ATTR_SIZE_VER2;
	static const size_t attr_v2_5_size = PERF_ATTR_SIZE_VER2 + 8;
	static const size_t attr_v2_75_size = PERF_ATTR_SIZE_VER2 + 12;
	static const size_t attr_v3_size = PERF_ATTR_SIZE_VER3;
	static const size_t attr_v4_size = PERF_ATTR_SIZE_VER4;
	static const size_t attr_v4_5_size = PERF_ATTR_SIZE_VER4 + 4;
	static const size_t attr_v4_625_size = PERF_ATTR_SIZE_VER4 + 5;
	static const size_t attr_v4_875_size = PERF_ATTR_SIZE_VER4 + 7;
	static const size_t attr_v5_size = PERF_ATTR_SIZE_VER5;
	static const size_t attr_v5_25_size = PERF_ATTR_SIZE_VER5 + 2;
	static const size_t attr_v5_5_size = PERF_ATTR_SIZE_VER5 + 4;
	static const size_t attr_v5_75_size = PERF_ATTR_SIZE_VER5 + 6;
	static const size_t attr_v6_size = PERF_ATTR_SIZE_VER6;
	static const size_t attr_big_size = PERF_ATTR_SIZE_VER6 + 32;

	static const struct u64_val_str attr_types[] = {
		{ ARG_STR(PERF_TYPE_HARDWARE) },
		{ ARG_STR(PERF_TYPE_SOFTWARE) },
		{ ARG_STR(PERF_TYPE_TRACEPOINT) },
		{ ARG_STR(PERF_TYPE_HW_CACHE) },
		{ ARG_STR(PERF_TYPE_RAW) },
		{ ARG_STR(PERF_TYPE_BREAKPOINT) },
		{ ARG_STR(0x6) " /* PERF_TYPE_??? */" },
		{ ARG_STR(0xdeadc0de) " /* PERF_TYPE_??? */" },
	};
	static const struct u64_val_str
	    attr_configs[ARRAY_SIZE(attr_types)][3] = {
		/* PERF_TYPE_HARDWARE */ {
			{ 9, "PERF_COUNT_HW_REF_CPU_CYCLES" },
			{ 10, "0xa /* PERF_COUNT_HW_??? */" },
			{ ARG_ULL_STR(0xfaceca75deadb0d4)
				" /* PERF_COUNT_HW_??? */" },
		},
		/* PERF_TYPE_SOFTWARE */ {
			{ 10, "PERF_COUNT_SW_BPF_OUTPUT" },
			{ 11, "0xb /* PERF_COUNT_SW_??? */" },
			{ ARG_ULL_STR(0xdec0ded1dec0ded2)
				" /* PERF_COUNT_SW_??? */" },
		},
		/* PERF_TYPE_TRACEPOINT */ {
			{ ARG_STR(0) },
			{ 4207856245U, "4207856245" },
			{ ARG_ULL_STR(16051074073505095380) },
		},
		/* PERF_TYPE_HW_CACHE */ {
			{ 0, "PERF_COUNT_HW_CACHE_L1D|"
				"PERF_COUNT_HW_CACHE_OP_READ<<8|"
				"PERF_COUNT_HW_CACHE_RESULT_ACCESS<<16" },
			{ 0x020207, "0x7 /* PERF_COUNT_HW_CACHE_??? */|"
				"PERF_COUNT_HW_CACHE_OP_PREFETCH<<8|"
				"0x2 /* PERF_COUNT_HW_CACHE_RESULT_??? */<<16" },
			{ 0xdeadf157ed010306ULL, "PERF_COUNT_HW_CACHE_NODE|"
				"0x3 /* PERF_COUNT_HW_CACHE_OP_??? */<<8|"
				"PERF_COUNT_HW_CACHE_RESULT_MISS<<16|"
				"0xdeadf157ed<<24 "
				"/* PERF_COUNT_HW_CACHE_??? */" },
		},
		/* PERF_TYPE_RAW */ {
			{ ARG_STR(0) },
			{ ARG_STR(0xda7a1057) },
			{ ARG_ULL_STR(0xdec0ded7dec0ded8) },
		},
		/* PERF_TYPE_BREAKPOINT */ {
			{ ARG_STR(0) },
			{ ARG_STR(0xbadc0ded) },
			{ ARG_ULL_STR(0xdec0ded9dec0deda) },
		},
		/* invalid 1 */ {
			{ ARG_STR(0) },
			{ ARG_STR(0xbeeff00d) },
			{ ARG_ULL_STR(0xdec0dedbdec0dedc) },
		},
		/* invalid 2 */ {
			{ ARG_STR(0) },
			{ ARG_STR(0xca75dead) },
			{ ARG_ULL_STR(0xdec0dedddec0dede) },
		},
	};
	static const struct u64_val_str sample_types[] = {
		{ ARG_STR(0) },
		{ 0x800, "PERF_SAMPLE_BRANCH_STACK" },
		{ ARG_ULL_STR(0xdeadc0deda600000) " /* PERF_SAMPLE_??? */" },
		{ 0xffffffffffffffffULL,
			"PERF_SAMPLE_IP|PERF_SAMPLE_TID|PERF_SAMPLE_TIME|"
			"PERF_SAMPLE_ADDR|PERF_SAMPLE_READ|"
			"PERF_SAMPLE_CALLCHAIN|PERF_SAMPLE_ID|PERF_SAMPLE_CPU|"
			"PERF_SAMPLE_PERIOD|PERF_SAMPLE_STREAM_ID|"
			"PERF_SAMPLE_RAW|PERF_SAMPLE_BRANCH_STACK|"
			"PERF_SAMPLE_REGS_USER|PERF_SAMPLE_STACK_USER|"
			"PERF_SAMPLE_WEIGHT|PERF_SAMPLE_DATA_SRC|"
			"PERF_SAMPLE_IDENTIFIER|PERF_SAMPLE_TRANSACTION|"
			"PERF_SAMPLE_REGS_INTR|PERF_SAMPLE_PHYS_ADDR|"
			"PERF_SAMPLE_AUX|"
			"0xffffffffffe00000" },
	};
	static const struct u64_val_str read_formats[] = {
		{ ARG_STR(0) },
		{ ARG_STR(PERF_FORMAT_TOTAL_TIME_ENABLED) },
		{ 0xf, "PERF_FORMAT_TOTAL_TIME_ENABLED|"
			"PERF_FORMAT_TOTAL_TIME_RUNNING|"
			"PERF_FORMAT_ID|PERF_FORMAT_GROUP" },
		{ ARG_ULL_STR(0xdeadf157dec0ded0) " /* PERF_FORMAT_??? */" },
		{ 0xffffffffffffffffULL,
			"PERF_FORMAT_TOTAL_TIME_ENABLED|"
			"PERF_FORMAT_TOTAL_TIME_RUNNING|"
			"PERF_FORMAT_ID|PERF_FORMAT_GROUP|"
			"0xfffffffffffffff0" },
	};
	static const char *precise_ip_descs[] = {
		"arbitrary skid",
		"constant skid",
		"requested to have 0 skid",
		"must have 0 skid",
	};
	static const struct u32_val_str bp_types[] = {
		{ 0, "HW_BREAKPOINT_EMPTY" },
		{ 1, "HW_BREAKPOINT_R" },
		{ 3, "HW_BREAKPOINT_RW" },
		{ 5, "0x5 /* HW_BREAKPOINT_INVALID */" },
		{ 8, "0x8 /* HW_BREAKPOINT_??? */" },
		{ ARG_STR(0xface1e55) " /* HW_BREAKPOINT_??? */" },
	};
	static const struct u64_val_str branch_sample_types[] = {
		{ ARG_STR(0) },
		{ 0x80, "PERF_SAMPLE_BRANCH_ABORT_TX" },
		{ 0x3ffff, BRANCH_TYPE_ALL },
		{ ARG_ULL_STR(0xdeadcaffeeec0000)
			" /* PERF_SAMPLE_BRANCH_??? */" },
		{ 0xffffffffffffffffULL,
			BRANCH_TYPE_ALL "|0xfffffffffffc0000" }
	};
	static const struct s32_val_str clockids[] = {
		{ 11, "CLOCK_TAI" },
		{ ARG_STR(0xc) " /* CLOCK_??? */" },
		{ ARG_STR(0xbeeffeed) " /* CLOCK_??? */" },
	};


	struct {
		struct perf_event_attr *ptr;
		size_t size;
	} attrs[] = {
		ATTR_REC(sizeof(struct perf_event_attr)),
		ATTR_REC(attr_v0_size),
		ATTR_REC(attr_v1_size),
		ATTR_REC(attr_v2_size),
		ATTR_REC(attr_v2_5_size),
		ATTR_REC(attr_v2_75_size),
		ATTR_REC(attr_v3_size),
		ATTR_REC(attr_v4_size),
		ATTR_REC(attr_v4_5_size),
		ATTR_REC(attr_v4_625_size),
		ATTR_REC(attr_v4_875_size),
		ATTR_REC(attr_v5_size),
		ATTR_REC(attr_v5_25_size),
		ATTR_REC(attr_v5_5_size),
		ATTR_REC(attr_v5_75_size),
		ATTR_REC(attr_v6_size),
		ATTR_REC(attr_big_size),
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(struct perf_event_attr, small_attr);

	struct {
		struct perf_event_attr *attr;
		pid_t pid;
		int cpu;
		int group_fd;
		unsigned long flags;
		const char *flags_str;
	} args[] = {
		{ NULL,           0xfacef00d, 0xbadabba7, -1,
			(unsigned long) 0xFFFFFFFFFFFFFFFFLLU,
			"PERF_FLAG_FD_NO_GROUP|PERF_FLAG_FD_OUTPUT|"
			"PERF_FLAG_PID_CGROUP|PERF_FLAG_FD_CLOEXEC|"
			"0x" LONG_STR_PREFIX "fffffff0"
			},
		{ small_attr + 1, 0,          0,          0,
			0, "0" },
		{ small_attr,     -1,         -1,         1,
			PERF_FLAG_FD_NO_GROUP | PERF_FLAG_FD_OUTPUT |
			PERF_FLAG_PID_CGROUP | PERF_FLAG_FD_CLOEXEC,
			"PERF_FLAG_FD_NO_GROUP|PERF_FLAG_FD_OUTPUT|"
			"PERF_FLAG_PID_CGROUP|PERF_FLAG_FD_CLOEXEC" },
		{ (struct perf_event_attr *) (uintptr_t) 0xfffffacefffffeedULL,
			          -100,       100,        0xface1e55,
			PERF_FLAG_FD_CLOEXEC, "PERF_FLAG_FD_CLOEXEC" },
	};

	size_t i;
	int rc;

	fill_memory(small_attr, sizeof(*small_attr));
	small_attr->size = attr_small_size;

	for (i = 0; i < ARRAY_SIZE(args); i++) {
		rc = syscall(__NR_perf_event_open, args[i].attr, args[i].pid,
			     args[i].cpu, args[i].group_fd, args[i].flags);
		printf("perf_event_open(%s, %d, %d, %d, %s) = %s\n",
		       printaddr(args[i].attr), args[i].pid, args[i].cpu,
		       args[i].group_fd, args[i].flags_str, sprintrc(rc));
	}

	for (i = 0; i < ARRAY_SIZE(attrs) * ARRAY_SIZE(attr_types) *
	    ARRAY_SIZE(attr_configs[0]) + 1; i++) {
		struct perf_event_attr *attr = attrs[i % ARRAY_SIZE(attrs)].ptr;
		uint32_t size = attrs[i % ARRAY_SIZE(attrs)].size;
		unsigned char fill_start = 0x80 + i;
		size_t type_idx = i % ARRAY_SIZE(attr_types);
		size_t config_idx = i % ARRAY_SIZE(attr_configs[0]);
		size_t sample_type_idx = i % ARRAY_SIZE(sample_types);
		size_t read_format_idx = i % ARRAY_SIZE(read_formats);
		size_t bp_type_idx = (i / ARRAY_SIZE(attr_configs[0])) %
			ARRAY_SIZE(bp_types);
		size_t branch_sample_type_idx = (i / ARRAY_SIZE(sample_types)) %
			ARRAY_SIZE(branch_sample_types);
		size_t clockid_idx = i % ARRAY_SIZE(clockids);
		size_t args_idx = i % ARRAY_SIZE(args);
		const char *ip_desc_str;

		fill_memory_ex(attr, size, fill_start, 0xff);

		attr->type = attr_types[type_idx].val;
		attr->size = size;
		attr->config = attr_configs[type_idx][config_idx].val;
		attr->sample_type = sample_types[sample_type_idx].val;
		attr->read_format = read_formats[read_format_idx].val;

		if ((i % 11) == 5)
			attr->__reserved_1 = 0;

# ifdef HAVE_STRUCT_PERF_EVENT_ATTR_BP_TYPE
		attr->bp_type =
# else
		((uint32_t *) attr)[52 / sizeof(uint32_t)] =
# endif
			bp_types[bp_type_idx].val;

		if (size >= 80)
# ifdef HAVE_STRUCT_PERF_EVENT_ATTR_BRANCH_SAMPLE_TYPE
			attr->branch_sample_type =
# else
			((uint64_t *) attr)[72 / sizeof(uint64_t)] =
# endif
				branch_sample_types[branch_sample_type_idx].val;

		if (size >= 96)
# ifdef HAVE_STRUCT_PERF_EVENT_ATTR_CLOCKID
			attr->clockid =
# else
			((uint32_t *) attr)[92 / sizeof(uint32_t)] =
# endif
				clockids[clockid_idx].val;

# ifdef HAVE_STRUCT_PERF_EVENT_ATTR_PRECISE_IP
		ip_desc_str = precise_ip_descs[attr->precise_ip];
# else
		union {
			struct pea_flags flags;
			uint64_t raw;
		} flags_data = { .raw = ((uint64_t *) attr)[5] };

		ip_desc_str = precise_ip_descs[flags_data.flags.precise_ip];
# endif

		if (((i % 17) == 3) && (size >= 112))
			((uint16_t *) attr)[110 / sizeof(uint16_t)] = 0;


		if (i == 0)
			attr->size = size + 8;

		if (i == 1)
			attr->size = 0;

		rc = syscall(__NR_perf_event_open, attr, args[args_idx].pid,
			     args[args_idx].cpu, args[args_idx].group_fd,
			     args[args_idx].flags);

		printf("perf_event_open(");
		print_event_attr(attr, i ? ((i == 1) ? 0 : size) : size + 8,
				 attr_types[type_idx].str,
				 attr_configs[type_idx][config_idx].str,
				 sample_types[sample_type_idx].str,
				 read_formats[read_format_idx].str,
				 ip_desc_str,
				 bp_types[bp_type_idx].str,
				 branch_sample_types[branch_sample_type_idx].str,
				 clockids[clockid_idx].str, size);
		printf(", %d, %d, %d, %s) = %s\n", args[args_idx].pid,
		       args[args_idx].cpu, args[args_idx].group_fd,
		       args[args_idx].flags_str, sprintrc(rc));
	}

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_perf_event_open && HAVE_LINUX_PERF_EVENT_H");

#endif
