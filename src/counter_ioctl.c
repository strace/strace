/*
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/ioctl.h>
#include <linux/counter.h>

#include "xlat/counter_ioctl_component_types.h"
#include "xlat/counter_ioctl_event_types.h"
#include "xlat/counter_ioctl_scopes.h"


static void
print_struct_counter_component(const struct counter_component *const cc)
{
	tprint_struct_begin();
	PRINT_FIELD_XVAL(*cc, type, counter_ioctl_component_types,
			 "COUNTER_COMPONENT_???");
	tprint_struct_next();
	PRINT_FIELD_XVAL(*cc, scope, counter_ioctl_scopes,
			 "COUNTER_SCOPE_???");
	tprint_struct_next();
	PRINT_FIELD_U(*cc, parent);
	tprint_struct_next();
	PRINT_FIELD_U(*cc, id);
	tprint_struct_end();
}

static void
print_struct_counter_watch(struct tcb *const tcp, const kernel_ulong_t addr)
{
	CHECK_IOCTL_SIZE(COUNTER_ADD_WATCH_IOCTL, 6);
	CHECK_TYPE_SIZE(struct counter_watch, 6);
	struct counter_watch w;

	if (umove_or_printaddr(tcp, addr, &w))
		return;

	tprint_struct_begin();
	PRINT_FIELD_OBJ_PTR(w, component, print_struct_counter_component);
	tprint_struct_next();
	PRINT_FIELD_XVAL(w, event, counter_ioctl_event_types,
			 "COUNTER_EVENT_???");
	tprint_struct_next();
	PRINT_FIELD_U(w, channel);
	tprint_struct_end();
}

int
counter_ioctl(struct tcb *const tcp, const unsigned int code,
	      const kernel_ulong_t arg)
{
	switch (code) {
	case COUNTER_ADD_WATCH_IOCTL:
		tprint_arg_next();
		print_struct_counter_watch(tcp, arg);
		return RVAL_IOCTL_DECODED;

	case COUNTER_ENABLE_EVENTS_IOCTL:
	case COUNTER_DISABLE_EVENTS_IOCTL:
		return RVAL_IOCTL_DECODED;

	default:
		return RVAL_DECODED;
	}
}
