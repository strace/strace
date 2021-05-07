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

#define PRINT_FIELD_HTONL_X(where_, field_)				\
	do {								\
		tprints_field_name(#field_);				\
		tprints_arg_begin("htonl");				\
		PRINT_VAL_X(ntohl((where_).field_));			\
		tprint_arg_end();					\
	} while (0)

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

	if (len > offset)
		tprint_array_begin();
	tprint_struct_begin();
	PRINT_FIELD_CSTRING(uh, prefix);
	tprint_struct_next();

	PRINT_FIELD_HTONL_X(uh, magic);
	tprint_struct_next();

	PRINT_FIELD_U(uh, header_size);
	tprint_struct_next();

	PRINT_FIELD_U(uh, properties_off);
	tprint_struct_next();

	PRINT_FIELD_U(uh, properties_len);
	tprint_struct_next();

	PRINT_FIELD_HTONL_X(uh, filter_subsystem_hash);
	tprint_struct_next();

	PRINT_FIELD_HTONL_X(uh, filter_devtype_hash);
	tprint_struct_next();

	PRINT_FIELD_HTONL_X(uh, filter_tag_bloom_hi);
	tprint_struct_next();

	PRINT_FIELD_HTONL_X(uh, filter_tag_bloom_lo);
	tprint_struct_end();

	if (len > offset) {
		tprint_array_next();
		printstrn(tcp, addr + offset, len - offset);
		tprint_array_end();
	}
}
