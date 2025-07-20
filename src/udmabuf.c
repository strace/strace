/*
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/udmabuf.h>
#include "xlat/udmabuf_flags.h"

static int
print_udmabuf_create(struct tcb *tcp, kernel_ulong_t arg)
{
	struct udmabuf_create uc;

	tprints_arg_next_name("argp");
	if (umove_or_printaddr(tcp, arg, &uc))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_FD(uc, memfd, tcp);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(uc, flags, udmabuf_flags, "UDMABUF_FLAGS_???");
	tprint_struct_next();
	PRINT_FIELD_U(uc, offset);
	tprint_struct_next();
	PRINT_FIELD_U(uc, size);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED | RVAL_FD;
}

static bool
print_udmabuf_create_item(struct tcb *tcp, void *data, size_t size, void *closure)
{
	struct udmabuf_create_item *item = data;

	tprint_struct_begin();
	PRINT_FIELD_FD(*item, memfd, tcp);
	tprint_struct_next();
	PRINT_FIELD_U(*item, offset);
	tprint_struct_next();
	PRINT_FIELD_U(*item, size);
	tprint_struct_end();

	return true;
}

static int
print_udmabuf_create_list(struct tcb *tcp, kernel_ulong_t arg)
{
	struct udmabuf_create_list uc;

	tprints_arg_next_name("argp");
	if (umove_or_printaddr(tcp, arg, &uc))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(uc, flags, udmabuf_flags, "UDMABUF_FLAGS_???");
	tprint_struct_next();
	PRINT_FIELD_U(uc, count);

	static const size_t list_offset = offsetof(struct udmabuf_create_list, list);
	struct udmabuf_create_item item;

	tprint_struct_next();
	tprints_field_name("list");
	print_array(tcp, arg + list_offset, uc.count, &item, sizeof(item),
		    tfetch_mem, print_udmabuf_create_item, NULL);

	tprint_struct_end();

	return RVAL_IOCTL_DECODED | RVAL_FD;
}

int
udmabuf_ioctl(struct tcb *const tcp, const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case UDMABUF_CREATE:
		return print_udmabuf_create(tcp, arg);
	case UDMABUF_CREATE_LIST:
		return print_udmabuf_create_list(tcp, arg);
	default:
		return RVAL_DECODED;
	}
}
