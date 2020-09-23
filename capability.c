/*
 * Copyright (c) 2000 Wichert Akkerman <wakkerma@debian.org>
 * Copyright (c) 2011 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2020 The strace developers.
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

	tprints("{version=");
	printxval(cap_version, h->version,
		  "_LINUX_CAPABILITY_VERSION_???");
	tprints(", pid=");
	printpid(tcp, h->pid, PT_TGID);
	tprints("}");
}

static void
print_cap_bits(const uint32_t lo, const uint32_t hi)
{
	if (lo || !hi)
		printflags(cap_mask0, lo, "CAP_???");

	if (hi) {
		if (lo)
			tprints("|");
		printflags(cap_mask1, hi, "CAP_???");
	}
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

	tprints("{effective=");
	print_cap_bits(data[0].effective, len > 1 ? data[1].effective : 0);
	tprints(", permitted=");
	print_cap_bits(data[0].permitted, len > 1 ? data[1].permitted : 0);
	tprints(", inheritable=");
	print_cap_bits(data[0].inheritable, len > 1 ? data[1].inheritable : 0);
	tprints("}");
}

SYS_FUNC(capget)
{
	const struct user_cap_header_struct *h;

	if (entering(tcp)) {
		h = get_cap_header(tcp, tcp->u_arg[0]);
		print_cap_header(tcp, tcp->u_arg[0], h);
		tprints(", ");
	} else {
		h = syserror(tcp) ? NULL : get_cap_header(tcp, tcp->u_arg[0]);
		print_cap_data(tcp, tcp->u_arg[1], h);
	}
	return 0;
}

SYS_FUNC(capset)
{
	const struct user_cap_header_struct *const h =
		get_cap_header(tcp, tcp->u_arg[0]);
	print_cap_header(tcp, tcp->u_arg[0], h);
	tprints(", ");
	print_cap_data(tcp, tcp->u_arg[1], h);

	return RVAL_DECODED;
}
