/*
 * Copyright (c) 2013 Ben Noordhuis <info@bnoordhuis.nl>
 * Copyright (c) 2013-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2015-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "perf_event_struct.h"

#include "xlat/hw_breakpoint_len.h"
#include "xlat/hw_breakpoint_type.h"
#include "xlat/perf_attr_size.h"
#include "xlat/perf_branch_sample_type.h"
#include "xlat/perf_event_open_flags.h"
#include "xlat/perf_event_read_format.h"
#include "xlat/perf_event_sample_format.h"
#include "xlat/perf_hw_cache_id.h"
#include "xlat/perf_hw_cache_op_id.h"
#include "xlat/perf_hw_cache_op_result_id.h"
#include "xlat/perf_hw_id.h"
#include "xlat/perf_sw_ids.h"
#include "xlat/perf_type_id.h"

struct pea_desc {
	struct perf_event_attr *attr;
	uint32_t size;
};

static void
free_pea_desc(void *pea_desc_ptr)
{
	struct pea_desc *desc = pea_desc_ptr;

	free(desc->attr);
	free(desc);
}

int
fetch_perf_event_attr(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct pea_desc *desc;
	struct perf_event_attr *attr;
	uint32_t size;

	if (umove(tcp, addr + offsetof(struct perf_event_attr, size), &size)) {
		printaddr(addr);
		return 1;
	}

	if (size > sizeof(*attr))
		size = sizeof(*attr);

	if (!size)
		size = PERF_ATTR_SIZE_VER0;

	/*
	 * Kernel (rightfully) deems invalid attribute structures with size less
	 * than first published format size, and we do the same.
	 */
	if (size < PERF_ATTR_SIZE_VER0) {
		printaddr(addr);
		return 1;
	}

	if (abbrev(tcp))
		size = offsetof(struct perf_event_attr, wakeup_events);

	/* Size should be multiple of 8, but kernel doesn't check for it */
	/* size &= ~7; */

	attr = xzalloc(sizeof(*attr));

	if (umoven_or_printaddr(tcp, addr, size, attr)) {
		free(attr);

		return 1;
	}

	desc = xmalloc(sizeof(*desc));

	desc->attr = attr;
	desc->size = size;

	set_tcb_priv_data(tcp, desc, free_pea_desc);

	return 0;
}

void
print_perf_event_attr(struct tcb *const tcp, const kernel_ulong_t addr)
{
	static const char *precise_ip_desc[] = {
		"arbitrary skid",
		"constant skid",
		"requested to have 0 skid",
		"must have 0 skid",
	};

	struct pea_desc *desc;
	struct perf_event_attr *attr;
	uint32_t size;
	uint32_t new_size;
	int use_new_size = 0;

	/*
	 * Amusingly, the kernel accepts structures with only part of the field
	 * present, so we perform the check like this (instead of checking
	 * offsetofend against size) in order to print fields as kernel sees
	 * them.  This also should work great on big endian architectures.
	 */
#define STRACE_PERF_CHECK_FIELD(field_) \
		do { \
			if (offsetof(struct perf_event_attr, field_) >= size) \
				goto print_perf_event_attr_out; \
		} while (0)

	desc = get_tcb_priv_data(tcp);

	attr = desc->attr;
	size = desc->size;

	/* The only error which expected to change size field currently */
	if (tcp->u_error == E2BIG) {
		if (umove(tcp, addr + offsetof(struct perf_event_attr, size),
		    &new_size))
			use_new_size = -1;
		else
			use_new_size = 1;
	}

	tprint_struct_begin();
	PRINT_FIELD_XVAL(*attr, type, perf_type_id, "PERF_TYPE_???");
	tprint_struct_next();
	PRINT_FIELD_XVAL(*attr, size, perf_attr_size, "PERF_ATTR_SIZE_???");

	if (use_new_size) {
		tprint_value_changed();

		if (use_new_size > 0)
			printxval(perf_attr_size, new_size,
				  "PERF_ATTR_SIZE_???");
		else
			tprint_unavailable();
	}

	switch (attr->type) {
	case PERF_TYPE_HARDWARE:
		/*
		 * EEEEEEEE000000AA
		 * EEEEEEEE - PMU type ID
		 * AA - perf_hw_id
		 */
		tprint_struct_next();
		tprints_field_name("config");
		tprint_flags_begin();
		if (attr->config >> 32) {
			tprint_shift_begin();
			PRINT_VAL_X(attr->config >> 32);
			tprint_shift();
			PRINT_VAL_U(32);
			tprint_shift_end();
			tprint_flags_or();
		}
		printxval(perf_hw_id, attr->config & PERF_HW_EVENT_MASK,
			   "PERF_COUNT_HW_???");
		tprint_flags_end();
		break;
	case PERF_TYPE_SOFTWARE:
		tprint_struct_next();
		PRINT_FIELD_XVAL(*attr, config, perf_sw_ids,
				 "PERF_COUNT_SW_???");
		break;
	case PERF_TYPE_TRACEPOINT:
		/*
		 * "The value to use in config can be obtained from under
		 * debugfs tracing/events/../../id if ftrace is enabled
		 * in the kernel."
		 */
		tprint_struct_next();
		PRINT_FIELD_U(*attr, config);
		break;
	case PERF_TYPE_HW_CACHE:
		/*
		 * EEEEEEEE00DDCCBB
		 * EEEEEEEE - PMU type ID
		 * BB - perf_hw_cache_id
		 * CC - perf_hw_cache_op_id
		 * DD - perf_hw_cache_op_result_id
		 */
		tprint_struct_next();
		tprints_field_name("config");
		tprint_flags_begin();
		if (attr->config >> 32){
			tprint_shift_begin();
			PRINT_VAL_X(attr->config >> 32);
			tprint_shift();
			PRINT_VAL_U(32);
			tprint_shift_end();
			tprint_flags_or();
		}
		if ((attr->config & PERF_HW_EVENT_MASK) >> 24) {
			tprint_shift_begin();
			PRINT_VAL_X((attr->config & PERF_HW_EVENT_MASK) >> 24);
			tprint_shift();
			PRINT_VAL_U(24);
			tprint_shift_end();
			tprint_flags_or();
		}
		tprint_shift_begin();
		printxval(perf_hw_cache_op_result_id,
			  (attr->config >> 16) & 0xFF,
			  "PERF_COUNT_HW_CACHE_RESULT_???");
		tprint_shift();
		PRINT_VAL_U(16);
		tprint_shift_end();

		tprint_flags_or();
		tprint_shift_begin();
		printxval(perf_hw_cache_op_id, (attr->config >> 8) & 0xFF,
			   "PERF_COUNT_HW_CACHE_OP_???");
		tprint_shift();
		PRINT_VAL_U(8);
		tprint_shift_end();

		tprint_flags_or();
		printxval(perf_hw_cache_id, attr->config & 0xFF,
			  "PERF_COUNT_HW_CACHE_???");
		tprint_flags_end();
		break;
	case PERF_TYPE_RAW:
		/*
		 * "If type is PERF_TYPE_RAW, then a custom "raw" config
		 * value is needed. Most CPUs support events that are not
		 * covered by the "generalized" events. These are
		 * implementation defined; see your CPU manual (for example the
		 * Intel Volume 3B documentation or the AMD BIOS and Kernel
		 * Developer Guide). The libpfm4 library can be used to
		 * translate from the name in the architectural manuals
		 * to the raw hex value perf_event_open() expects in this
		 * field."
		 */
	case PERF_TYPE_BREAKPOINT:
		/*
		 * "If type is PERF_TYPE_BREAKPOINT, then leave config set
		 * to zero. Its parameters are set in other places."
		 */
	default:
		tprint_struct_next();
		PRINT_FIELD_X(*attr, config);
		break;
	}

	if (attr->freq) {
		tprint_struct_next();
		PRINT_FIELD_U(*attr, sample_freq);
	} else {
		tprint_struct_next();
		PRINT_FIELD_U(*attr, sample_period);
	}

	tprint_struct_next();
	PRINT_FIELD_FLAGS(*attr, sample_type, perf_event_sample_format,
			  "PERF_SAMPLE_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(*attr, read_format, perf_event_read_format,
			  "PERF_FORMAT_???");

	/*** A shorthand for printing struct perf_event_attr bit flags */
#define STRACE_PERF_PRINT_FLAG(flag_) \
	do { \
		if (!abbrev(tcp) || attr->flag_) { \
			tprint_struct_next(); \
			PRINT_FIELD_U_CAST(*attr, flag_, unsigned int); \
		}  \
	} while (0)

	STRACE_PERF_PRINT_FLAG(disabled);
	STRACE_PERF_PRINT_FLAG(inherit);
	STRACE_PERF_PRINT_FLAG(pinned);
	STRACE_PERF_PRINT_FLAG(exclusive);
	STRACE_PERF_PRINT_FLAG(exclude_user);
	STRACE_PERF_PRINT_FLAG(exclude_kernel);
	STRACE_PERF_PRINT_FLAG(exclude_hv);
	STRACE_PERF_PRINT_FLAG(exclude_idle);
	STRACE_PERF_PRINT_FLAG(mmap);
	STRACE_PERF_PRINT_FLAG(comm);
	STRACE_PERF_PRINT_FLAG(freq);
	STRACE_PERF_PRINT_FLAG(inherit_stat);
	STRACE_PERF_PRINT_FLAG(enable_on_exec);
	STRACE_PERF_PRINT_FLAG(task);
	STRACE_PERF_PRINT_FLAG(watermark);
	tprint_struct_next();
	PRINT_FIELD_U_CAST(*attr, precise_ip, unsigned int);
	tprints_comment(precise_ip_desc[attr->precise_ip]);
	STRACE_PERF_PRINT_FLAG(mmap_data);
	STRACE_PERF_PRINT_FLAG(sample_id_all);
	STRACE_PERF_PRINT_FLAG(exclude_host);
	STRACE_PERF_PRINT_FLAG(exclude_guest);
	STRACE_PERF_PRINT_FLAG(exclude_callchain_kernel);
	STRACE_PERF_PRINT_FLAG(exclude_callchain_user);
	STRACE_PERF_PRINT_FLAG(mmap2);
	STRACE_PERF_PRINT_FLAG(comm_exec);
	STRACE_PERF_PRINT_FLAG(use_clockid);
	STRACE_PERF_PRINT_FLAG(context_switch);
	STRACE_PERF_PRINT_FLAG(write_backward);
	STRACE_PERF_PRINT_FLAG(namespaces);
	STRACE_PERF_PRINT_FLAG(ksymbol);
	STRACE_PERF_PRINT_FLAG(bpf_event);
	STRACE_PERF_PRINT_FLAG(aux_output);
	STRACE_PERF_PRINT_FLAG(cgroup);
	STRACE_PERF_PRINT_FLAG(text_poke);
	STRACE_PERF_PRINT_FLAG(build_id);
	STRACE_PERF_PRINT_FLAG(inherit_thread);
	STRACE_PERF_PRINT_FLAG(remove_on_exec);
	STRACE_PERF_PRINT_FLAG(sigtrap);

	/*
	 * Print it only in case it is non-zero, since it may contain flags we
	 * are not aware about.
	 */
	if (attr->__reserved_1) {
		tprint_struct_next();
		PRINT_FIELD_X_CAST(*attr, __reserved_1, uint64_t);
		tprints_comment("Bits 63..38");
	}

	if (abbrev(tcp))
		goto print_perf_event_attr_out;

	if (attr->watermark) {
		tprint_struct_next();
		PRINT_FIELD_U(*attr, wakeup_watermark);
	} else {
		tprint_struct_next();
		PRINT_FIELD_U(*attr, wakeup_events);
	}

	if (attr->type == PERF_TYPE_BREAKPOINT) {
		/* Any combination of R/W with X is deemed invalid */
		tprint_struct_next();
		PRINT_FIELD_XVAL(*attr, bp_type, hw_breakpoint_type,
				 (attr->bp_type <=
					(HW_BREAKPOINT_X | HW_BREAKPOINT_RW))
						? "HW_BREAKPOINT_INVALID"
						: "HW_BREAKPOINT_???");
	}

	if (attr->type == PERF_TYPE_BREAKPOINT) {
		tprint_struct_next();
		PRINT_FIELD_X(*attr, bp_addr);
	} else {
		tprint_struct_next();
		PRINT_FIELD_X(*attr, config1);
	}

	/*
	 * Fields after bp_addr/config1 are optional and may not present; check
	 * against size is needed.
	 */

	STRACE_PERF_CHECK_FIELD(bp_len);
	if (attr->type == PERF_TYPE_BREAKPOINT) {
		tprint_struct_next();
		PRINT_FIELD_U(*attr, bp_len);
	} else {
		tprint_struct_next();
		PRINT_FIELD_X(*attr, config2);
	}

	STRACE_PERF_CHECK_FIELD(branch_sample_type);
	if (attr->sample_type & PERF_SAMPLE_BRANCH_STACK) {
		tprint_struct_next();
		PRINT_FIELD_FLAGS(*attr, branch_sample_type,
				  perf_branch_sample_type,
				  "PERF_SAMPLE_BRANCH_???");
	}

	STRACE_PERF_CHECK_FIELD(sample_regs_user);
	/*
	 * "This bit mask defines the set of user CPU registers to dump on
	 * samples. The layout of the register mask is architecture-specific and
	 * described in the kernel header
	 * arch/ARCH/include/uapi/asm/perf_regs.h."
	 */
	tprint_struct_next();
	PRINT_FIELD_X(*attr, sample_regs_user);

	STRACE_PERF_CHECK_FIELD(sample_stack_user);
	/*
	 * "size of the user stack to dump if PERF_SAMPLE_STACK_USER is
	 * specified."
	 */
	if (attr->sample_type & PERF_SAMPLE_STACK_USER) {
		tprint_struct_next();
		PRINT_FIELD_X(*attr, sample_stack_user);
	}

	if (attr->use_clockid) {
		STRACE_PERF_CHECK_FIELD(clockid);
		tprint_struct_next();
		PRINT_FIELD_XVAL(*attr, clockid, clocknames, "CLOCK_???");
	}

	STRACE_PERF_CHECK_FIELD(sample_regs_intr);
	tprint_struct_next();
	PRINT_FIELD_X(*attr, sample_regs_intr);

	STRACE_PERF_CHECK_FIELD(aux_watermark);
	tprint_struct_next();
	PRINT_FIELD_U(*attr, aux_watermark);

	STRACE_PERF_CHECK_FIELD(sample_max_stack);
	tprint_struct_next();
	PRINT_FIELD_U(*attr, sample_max_stack);

	STRACE_PERF_CHECK_FIELD(__reserved_2);
	if (attr->__reserved_2)
		tprintf_comment("bytes 110..111: %#hx", attr->__reserved_2);

	STRACE_PERF_CHECK_FIELD(aux_sample_size);
	tprint_struct_next();
	PRINT_FIELD_U(*attr, aux_sample_size);

	STRACE_PERF_CHECK_FIELD(__reserved_3);
	if (attr->__reserved_3)
		tprintf_comment("bytes 116..119: %#x", attr->__reserved_3);

	STRACE_PERF_CHECK_FIELD(sig_data);
	tprint_struct_next();
	PRINT_FIELD_X(*attr, sig_data);

	STRACE_PERF_CHECK_FIELD(config3);
	tprint_struct_next();
	PRINT_FIELD_X(*attr, config3);

print_perf_event_attr_out:
	if ((attr->size && (attr->size > size)) ||
	    (!attr->size && (size < PERF_ATTR_SIZE_VER0))) {
		tprint_struct_next();
		tprint_more_data_follows();
	}

	tprint_struct_end();
}

SYS_FUNC(perf_event_open)
{
	/*
	 * We try to copy out the whole structure on entering in order to check
	 * size value on exiting. We do not check the rest of the fields because
	 * they shouldn't be changed, but copy the whole structure instead
	 * of just size field because they could.
	 */
	if (entering(tcp)) {
		/* attr */
		if (!fetch_perf_event_attr(tcp, tcp->u_arg[0]))
			return 0;
	} else {
		/* attr */
		print_perf_event_attr(tcp, tcp->u_arg[0]);
	}
	tprint_arg_next();

	/* pid */
	PRINT_VAL_D((int) tcp->u_arg[1]);
	tprint_arg_next();

	/* cpu */
	PRINT_VAL_D((int) tcp->u_arg[2]);
	tprint_arg_next();

	/* group_fd */
	printfd(tcp, tcp->u_arg[3]);
	tprint_arg_next();

	/* flags */
	printflags64(perf_event_open_flags, tcp->u_arg[4], "PERF_FLAG_???");

	return RVAL_DECODED | RVAL_FD;
}
