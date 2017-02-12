/*
 * Copyright (c) 2000 Wichert Akkerman <wakkerma@debian.org>
 * Copyright (c) 2011 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
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
	tprintf(", pid=%d}", h->pid);
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
