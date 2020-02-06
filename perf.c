/*
 * Copyright (c) 2013 Ben Noordhuis <info@bnoordhuis.nl>
 * Copyright (c) 2013-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2015-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "perf_event_struct.h"
#include "print_fields.h"

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
		size = offsetofend(struct perf_event_attr, config);

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
	 * Amusingly, kernel accepts structures with only part of the field
	 * present, so we making check like this (instead of checking
	 * offsetofend against size) in order to print fields as kernel sees
	 * them. This also should work great on big endian architectures.
	 */
#define _PERF_CHECK_FIELD(_field) \
		do { \
			if (offsetof(struct perf_event_attr, _field) >= size) \
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

	PRINT_FIELD_XVAL("{", *attr, type, perf_type_id, "PERF_TYPE_???");
	PRINT_FIELD_XVAL(", ", *attr, size, perf_attr_size,
			 "PERF_ATTR_SIZE_???");

	if (use_new_size) {
		tprints(" => ");

		if (use_new_size > 0)
			printxval(perf_attr_size, new_size,
				  "PERF_ATTR_SIZE_???");
		else
			tprints("???");
	}

	switch (attr->type) {
	case PERF_TYPE_HARDWARE:
		PRINT_FIELD_XVAL(", ", *attr, config, perf_hw_id,
				 "PERF_COUNT_HW_???");
		break;
	case PERF_TYPE_SOFTWARE:
		PRINT_FIELD_XVAL(", ", *attr, config, perf_sw_ids,
				 "PERF_COUNT_SW_???");
		break;
	case PERF_TYPE_TRACEPOINT:
		/*
		 * "The value to use in config can be obtained from under
		 * debugfs tracing/events/../../id if ftrace is enabled
		 * in the kernel."
		 */
		PRINT_FIELD_U(", ", *attr, config);
		break;
	case PERF_TYPE_HW_CACHE:
		/*
		 * (perf_hw_cache_id) | (perf_hw_cache_op_id << 8) |
		 * (perf_hw_cache_op_result_id << 16)
		 */
		tprints(", config=");
		printxval(perf_hw_cache_id, attr->config & 0xFF,
			  "PERF_COUNT_HW_CACHE_???");
		tprints("|");
		printxval(perf_hw_cache_op_id, (attr->config >> 8) & 0xFF,
			   "PERF_COUNT_HW_CACHE_OP_???");
		tprints("<<8|");
		/*
		 * Current code (see set_ext_hw_attr in arch/x86/events/core.c,
		 * tile_map_cache_event in arch/tile/kernel/perf_event.c,
		 * arc_pmu_cache_event in arch/arc/kernel/perf_event.c,
		 * hw_perf_cache_event in arch/blackfin/kernel/perf_event.c,
		 * _hw_perf_cache_event in arch/metag/kernel/perf/perf_event.c,
		 * mipspmu_map_cache_event in arch/mips/kernel/perf_event_mipsxx.c,
		 * hw_perf_cache_event in arch/powerpc/perf/core-book3s.c,
		 * hw_perf_cache_event in arch/powerpc/perf/core-fsl-emb.c,
		 * hw_perf_cache_event in arch/sh/kernel/perf_event.c,
		 * sparc_map_cache_event in arch/sparc/kernel/perf_event.c,
		 * xtensa_pmu_cache_event in arch/xtensa/kernel/perf_event.c,
		 * armpmu_map_cache_event in drivers/perf/arm_pmu.c) assumes
		 * that cache result is 8 bits in size.
		 */
		printxval(perf_hw_cache_op_result_id,
			  (attr->config >> 16) & 0xFF,
			  "PERF_COUNT_HW_CACHE_RESULT_???");
		tprints("<<16");
		if (attr->config >> 24) {
			tprintf("|%#" PRIx64 "<<24", attr->config >> 24);
			tprints_comment("PERF_COUNT_HW_CACHE_???");
		}
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
		PRINT_FIELD_X(", ", *attr, config);
		break;
	}

	if (abbrev(tcp))
		goto print_perf_event_attr_out;

	if (attr->freq)
		PRINT_FIELD_U(", ", *attr, sample_freq);
	else
		PRINT_FIELD_U(", ", *attr, sample_period);

	PRINT_FIELD_FLAGS(", ", *attr, sample_type, perf_event_sample_format,
			  "PERF_SAMPLE_???");
	PRINT_FIELD_FLAGS(", ", *attr, read_format, perf_event_read_format,
			  "PERF_FORMAT_???");

	tprintf(", disabled=%u"
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
		", watermark=%u"
		", precise_ip=%u",
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
		attr->watermark,
		attr->precise_ip);
	tprints_comment(precise_ip_desc[attr->precise_ip]);
	tprintf(", mmap_data=%u"
		", sample_id_all=%u"
		", exclude_host=%u"
		", exclude_guest=%u"
		", exclude_callchain_kernel=%u"
		", exclude_callchain_user=%u"
		", mmap2=%u"
		", comm_exec=%u"
		", use_clockid=%u"
		", context_switch=%u"
		", write_backward=%u"
		", namespaces=%u",
		attr->mmap_data,
		attr->sample_id_all,
		attr->exclude_host,
		attr->exclude_guest,
		attr->exclude_callchain_kernel,
		attr->exclude_callchain_user,
		attr->mmap2,
		attr->comm_exec,
		attr->use_clockid,
		attr->context_switch,
		attr->write_backward,
		attr->namespaces);

	/*
	 * Print it only in case it is non-zero, since it may contain flags we
	 * are not aware about.
	 */
	if (attr->__reserved_1) {
		tprintf(", __reserved_1=%#" PRIx64,
			(uint64_t) attr->__reserved_1);
		tprints_comment("Bits 63..29");
	}

	if (attr->watermark)
		PRINT_FIELD_U(", ", *attr, wakeup_watermark);
	else
		PRINT_FIELD_U(", ", *attr, wakeup_events);

	if (attr->type == PERF_TYPE_BREAKPOINT)
		/* Any combination of R/W with X is deemed invalid */
		PRINT_FIELD_XVAL(", ", *attr, bp_type, hw_breakpoint_type,
				 (attr->bp_type <=
					(HW_BREAKPOINT_X | HW_BREAKPOINT_RW))
						? "HW_BREAKPOINT_INVALID"
						: "HW_BREAKPOINT_???");

	if (attr->type == PERF_TYPE_BREAKPOINT)
		PRINT_FIELD_X(", ", *attr, bp_addr);
	else
		PRINT_FIELD_X(", ", *attr, config1);

	/*
	 * Fields after bp_addr/config1 are optional and may not present; check
	 * against size is needed.
	 */

	_PERF_CHECK_FIELD(bp_len);
	if (attr->type == PERF_TYPE_BREAKPOINT)
		PRINT_FIELD_U(", ", *attr, bp_len);
	else
		PRINT_FIELD_X(", ", *attr, config2);

	_PERF_CHECK_FIELD(branch_sample_type);
	if (attr->sample_type & PERF_SAMPLE_BRANCH_STACK) {
		PRINT_FIELD_FLAGS(", ", *attr, branch_sample_type,
				  perf_branch_sample_type,
				  "PERF_SAMPLE_BRANCH_???");
	}

	_PERF_CHECK_FIELD(sample_regs_user);
	/*
	 * "This bit mask defines the set of user CPU registers to dump on
	 * samples. The layout of the register mask is architecture-specific and
	 * described in the kernel header
	 * arch/ARCH/include/uapi/asm/perf_regs.h."
	 */
	PRINT_FIELD_X(", ", *attr, sample_regs_user);

	_PERF_CHECK_FIELD(sample_stack_user);
	/*
	 * "size of the user stack to dump if PERF_SAMPLE_STACK_USER is
	 * specified."
	 */
	if (attr->sample_type & PERF_SAMPLE_STACK_USER)
		PRINT_FIELD_X(", ", *attr, sample_stack_user);

	if (attr->use_clockid) {
		_PERF_CHECK_FIELD(clockid);
		PRINT_FIELD_XVAL(", ", *attr, clockid, clocknames, "CLOCK_???");
	}

	_PERF_CHECK_FIELD(sample_regs_intr);
	PRINT_FIELD_X(", ", *attr, sample_regs_intr);

	_PERF_CHECK_FIELD(aux_watermark);
	PRINT_FIELD_U(", ", *attr, aux_watermark);

	_PERF_CHECK_FIELD(sample_max_stack);
	PRINT_FIELD_U(", ", *attr, sample_max_stack);

	_PERF_CHECK_FIELD(__reserved_2);
	if (attr->__reserved_2)
		tprintf(" /* bytes 110..111: %#hx */", attr->__reserved_2);

	_PERF_CHECK_FIELD(aux_sample_size);
	PRINT_FIELD_U(", ", *attr, aux_sample_size);

	/* _PERF_CHECK_FIELD(__reserved_3);
	if (attr->__reserved_3)
		PRINT_FIELD_X(", ", *attr, __reserved_3); */

print_perf_event_attr_out:
	if ((attr->size && (attr->size > size)) ||
	    (!attr->size && (size < PERF_ATTR_SIZE_VER0)))
		tprints(", ...");

	tprints("}");
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
		if (!fetch_perf_event_attr(tcp, tcp->u_arg[0]))
			return 0;
	} else {
		print_perf_event_attr(tcp, tcp->u_arg[0]);
	}

	tprintf(", %d, %d, ",
		(int) tcp->u_arg[1],
		(int) tcp->u_arg[2]);
	printfd(tcp, tcp->u_arg[3]);
	tprints(", ");
	printflags64(perf_event_open_flags, tcp->u_arg[4], "PERF_FLAG_???");

	return RVAL_DECODED | RVAL_FD;
}
