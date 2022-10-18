/*
 * Copyright (c) 2000 Wichert Akkerman <wakkerma@debian.org>
 * Copyright (c) 2011 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2014-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

/* these constants are the same as in <linux/capability.h> */
enum {
#include "caps0.h"
};

#include "xlat/cap_mask0.h"

/* these constants are CAP_TO_INDEX'ed constants from <linux/capability.h> */
enum {
#include "caps1.h"
};

#include "xlat/cap_mask1.h"

/* these constants are the same as in <linux/capability.h> */
enum {
	_LINUX_CAPABILITY_VERSION_1 = 0x19980330,
	_LINUX_CAPABILITY_VERSION_2 = 0x20071026,
	_LINUX_CAPABILITY_VERSION_3 = 0x20080522
};

#include "xlat/cap_version.h"

struct user_cap_header_struct {
	uint32_t version;
	int pid;
};

struct user_cap_data_struct {
	uint32_t effective;
	uint32_t permitted;
	uint32_t inheritable;
};

static const struct user_cap_header_struct *
get_cap_header(struct tcb *const tcp, const kernel_ulong_t addr)
{
	static struct user_cap_header_struct header;

	if (!addr || !verbose(tcp))
		return NULL;

	if (umove(tcp, addr, &header) < 0)
		return NULL;

	return &header;
}

static void
print_cap_header(struct tcb *const tcp, const kernel_ulong_t addr,
		 const struct user_cap_header_struct *const h)
{
	if (!addr || !h) {
		printaddr(addr);
		return;
	}

	tprint_struct_begin();
	PRINT_FIELD_XVAL(*h, version, cap_version,
			 "_LINUX_CAPABILITY_VERSION_???");
	tprint_struct_next();
	PRINT_FIELD_TGID(*h, pid, tcp);
	tprint_struct_end();
}

static void
print_cap_bits(const uint32_t lo, const uint32_t hi)
{
	tprint_flags_begin();
	if (lo || !hi)
		printflags_in(cap_mask0, lo, "CAP_???");

	if (hi) {
		if (lo)
			tprint_flags_or();
		printflags_in(cap_mask1, hi, "CAP_???");
	}
	tprint_flags_end();
}

static void
print_cap_data(struct tcb *const tcp, const kernel_ulong_t addr,
	       const struct user_cap_header_struct *const h)
{
	struct user_cap_data_struct data[2];
	unsigned int len;

	if (!addr || !h) {
		printaddr(addr);
		return;
	}

	if (_LINUX_CAPABILITY_VERSION_2 == h->version ||
	    _LINUX_CAPABILITY_VERSION_3 == h->version)
		len = 2;
	else
		len = 1;

	if (umoven_or_printaddr(tcp, addr, len * sizeof(data[0]), data))
		return;

	tprint_struct_begin();
	tprints_field_name("effective");
	print_cap_bits(data[0].effective, len > 1 ? data[1].effective : 0);
	tprint_struct_next();
	tprints_field_name("permitted");
	print_cap_bits(data[0].permitted, len > 1 ? data[1].permitted : 0);
	tprint_struct_next();
	tprints_field_name("inheritable");
	print_cap_bits(data[0].inheritable, len > 1 ? data[1].inheritable : 0);
	tprint_struct_end();
}

SYS_FUNC(capget)
{
	const struct user_cap_header_struct *h;

	if (entering(tcp)) {
		/* hdrp */
		h = get_cap_header(tcp, tcp->u_arg[0]);
		print_cap_header(tcp, tcp->u_arg[0], h);
		tprint_arg_next();
	} else {
		/* datap */
		h = syserror(tcp) ? NULL : get_cap_header(tcp, tcp->u_arg[0]);
		print_cap_data(tcp, tcp->u_arg[1], h);
	}
	return 0;
}

SYS_FUNC(capset)
{
	/* hdrp */
	const struct user_cap_header_struct *const h =
		get_cap_header(tcp, tcp->u_arg[0]);
	print_cap_header(tcp, tcp->u_arg[0], h);
	tprint_arg_next();

	/* datap */
	print_cap_data(tcp, tcp->u_arg[1], h);

	return RVAL_DECODED;
}
