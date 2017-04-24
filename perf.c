/*
 * Copyright (c) 2013 Ben Noordhuis <info@bnoordhuis.nl>
 * Copyright (c) 2013-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
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

static int
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

	attr = xcalloc(1, sizeof(*attr));

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

#define PRINT_XLAT(prefix, xlat, x, dflt) \
	do { \
		tprints(prefix); \
		printxval_search(xlat, x, dflt); \
	} while (0)

static void
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

	PRINT_XLAT("{type=", perf_type_id, attr->type, "PERF_TYPE_???");
	tprints(", size=");
	printxval(perf_attr_size, attr->size, "PERF_ATTR_SIZE_???");

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
		PRINT_XLAT(", config=", perf_hw_id, attr->config,
		           "PERF_COUNT_HW_???");
		break;
	case PERF_TYPE_SOFTWARE:
		PRINT_XLAT(", config=", perf_sw_ids, attr->config,
		           "PERF_COUNT_SW_???");
		break;
	case PERF_TYPE_TRACEPOINT:
		/*
		 * "The value to use in config can be obtained from under
		 * debugfs tracing/events/../../id if ftrace is enabled in the
                 * kernel."
		 */
		tprintf(", config=%" PRIu64, attr->config);
		break;
	case PERF_TYPE_HW_CACHE:
		/*
		 * (perf_hw_cache_id) | (perf_hw_cache_op_id << 8) |
		 * (perf_hw_cache_op_result_id << 16)
		 */
		PRINT_XLAT(", config=", perf_hw_cache_id, attr->config & 0xFF,
		           "PERF_COUNT_HW_CACHE_???");
		PRINT_XLAT("|", perf_hw_cache_op_id, (attr->config >> 8) & 0xFF,
		           "PERF_COUNT_HW_CACHE_OP_???");
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
		PRINT_XLAT("<<8|", perf_hw_cache_op_result_id,
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
		tprintf(", config=%#" PRIx64, attr->config);
		break;
	}

	if (abbrev(tcp))
		goto print_perf_event_attr_out;

	if (attr->freq)
		tprintf(", sample_freq=%" PRIu64, attr->sample_freq);
	else
		tprintf(", sample_period=%" PRIu64, attr->sample_period);

	tprints(", sample_type=");
	printflags64(perf_event_sample_format, attr->sample_type,
		"PERF_SAMPLE_???");

	tprints(", read_format=");
	printflags64(perf_event_read_format, attr->read_format,
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
	        ", write_backward=%u",
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
	        attr->write_backward);

	/*
	 * Print it only in case it is non-zero, since it may contain flags we
	 * are not aware about.
	 */
	if (attr->__reserved_1) {
		tprintf(", __reserved_1=%#" PRIx64,
		        (uint64_t) attr->__reserved_1);
		tprints_comment("Bits 63..28");
	}

	if (attr->watermark)
		tprintf(", wakeup_watermark=%u", attr->wakeup_watermark);
	else
		tprintf(", wakeup_events=%u", attr->wakeup_events);

	if (attr->type == PERF_TYPE_BREAKPOINT)
		/* Any combination of R/W with X is deemed invalid */
		PRINT_XLAT(", bp_type=", hw_breakpoint_type, attr->bp_type,
		           (attr->bp_type <=
		                   (HW_BREAKPOINT_X | HW_BREAKPOINT_RW)) ?
		                           "HW_BREAKPOINT_INVALID" :
		                           "HW_BREAKPOINT_???");

	if (attr->type == PERF_TYPE_BREAKPOINT)
		tprintf(", bp_addr=%#" PRIx64, attr->bp_addr);
	else
		tprintf(", config1=%#" PRIx64, attr->config1);

	/*
	 * Fields after bp_addr/config1 are optional and may not present; check
	 * against size is needed.
	 */

	_PERF_CHECK_FIELD(bp_len);
	if (attr->type == PERF_TYPE_BREAKPOINT)
		tprintf(", bp_len=%" PRIu64, attr->bp_len);
	else
		tprintf(", config2=%#" PRIx64, attr->config2);

	_PERF_CHECK_FIELD(branch_sample_type);
	if (attr->sample_type & PERF_SAMPLE_BRANCH_STACK) {
		tprints(", branch_sample_type=");
		printflags64(perf_branch_sample_type, attr->branch_sample_type,
		             "PERF_SAMPLE_BRANCH_???");
	}

	_PERF_CHECK_FIELD(sample_regs_user);
	/*
	 * "This bit mask defines the set of user CPU registers to dump on
	 * samples. The layout of the register mask is architecture-specific and
	 * described in the kernel header
	 * arch/ARCH/include/uapi/asm/perf_regs.h."
	 */
	tprintf(", sample_regs_user=%#" PRIx64, attr->sample_regs_user);

	_PERF_CHECK_FIELD(sample_stack_user);
	/*
	 * "size of the user stack to dump if PERF_SAMPLE_STACK_USER is
	 * specified."
	 */
	if (attr->sample_type & PERF_SAMPLE_STACK_USER)
		tprintf(", sample_stack_user=%#" PRIx32,
		        attr->sample_stack_user);

	if (attr->use_clockid) {
		_PERF_CHECK_FIELD(clockid);
		tprints(", clockid=");
		printxval(clocknames, attr->clockid, "CLOCK_???");
	}

	_PERF_CHECK_FIELD(sample_regs_intr);
	tprintf(", sample_regs_intr=%#" PRIx64, attr->sample_regs_intr);

	_PERF_CHECK_FIELD(aux_watermark);
	tprintf(", aux_watermark=%" PRIu32, attr->aux_watermark);

	_PERF_CHECK_FIELD(sample_max_stack);
	tprintf(", sample_max_stack=%" PRIu16, attr->sample_max_stack);

	/* _PERF_CHECK_FIELD(__reserved_2);
	tprintf(", __reserved2=%" PRIu16, attr->__reserved_2); */

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

	tprintf(", %d, %d, %d, ",
		(int) tcp->u_arg[1],
		(int) tcp->u_arg[2],
		(int) tcp->u_arg[3]);
	printflags64(perf_event_open_flags, tcp->u_arg[4], "PERF_FLAG_???");

	return RVAL_DECODED | RVAL_FD;
}
