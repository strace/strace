/*
 * Copyright (c) 2018 Harsha Sharma <harshasharmaiitr@gmail.com>
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
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

	tprint_struct_begin();
	tprint_struct_begin();
	PRINT_FIELD_CSTRING(uh, prefix);
	tprint_struct_next();
	tprints_field_name("magic");
	tprintf("htonl(%#x)", ntohl(uh.magic));
	tprint_struct_next();
	PRINT_FIELD_U(uh, header_size);
	tprint_struct_next();
	PRINT_FIELD_U(uh, properties_off);
	tprint_struct_next();
	PRINT_FIELD_U(uh, properties_len);
	tprint_struct_next();
	tprints_field_name("filter_subsystem_hash");
	tprintf("htonl(%#x)", ntohl(uh.filter_subsystem_hash));
	tprint_struct_next();
	tprints_field_name("filter_devtype_hash");
	tprintf("htonl(%#x)", ntohl(uh.filter_devtype_hash));
	tprint_struct_next();
	tprints_field_name("filter_tag_bloom_hi");
	tprintf("htonl(%#x)", ntohl(uh.filter_tag_bloom_hi));
	tprint_struct_next();
	tprints_field_name("filter_tag_bloom_lo");
	tprintf("htonl(%#x)", ntohl(uh.filter_tag_bloom_lo));
	tprint_struct_end();
	if (len > offset) {
		tprints(", ");
		printstrn(tcp, addr + offset, len - offset);
	}
	tprint_struct_end();
}
