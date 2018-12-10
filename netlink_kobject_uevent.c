/*
 * Copyright (c) 2018 Harsha Sharma <harshasharmaiitr@gmail.com>
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"
#include "netlink_kobject_uevent.h"

#include <arpa/inet.h>

void
decode_netlink_kobject_uevent(struct tcb *tcp, kernel_ulong_t addr,
			      kernel_ulong_t len)
{
	struct udev_monitor_netlink_header uh;
	const char *prefix = "libudev";
	unsigned int offset = sizeof(uh);

	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)) ||
	    !addr || len < offset || umove(tcp, addr, &uh) ||
	    strcmp(uh.prefix, prefix) != 0) {
		printstrn(tcp, addr, len);
		return;
	}

	PRINT_FIELD_CSTRING("{{", uh, prefix);
	tprintf(", magic=htonl(%#x)", ntohl(uh.magic));
	PRINT_FIELD_U(", ", uh, header_size);
	PRINT_FIELD_U(", ", uh, properties_off);
	PRINT_FIELD_U(", ", uh, properties_len);
	tprintf(", filter_subsystem_hash=htonl(%#x)", ntohl(uh.filter_subsystem_hash));
	tprintf(", filter_devtype_hash=htonl(%#x)", ntohl(uh.filter_devtype_hash));
	tprintf(", filter_tag_bloom_hi=htonl(%#x)", ntohl(uh.filter_tag_bloom_hi));
	tprintf(", filter_tag_bloom_lo=htonl(%#x)", ntohl(uh.filter_tag_bloom_lo));
	tprints("}");
	if (len > offset) {
		tprints(", ");
		printstrn(tcp, addr + offset, len - offset);
	}
	tprints("}");
}
