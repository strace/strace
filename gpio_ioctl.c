/*
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "print_fields.h"

#include "types/gpio.h"
#define XLAT_MACROS_ONLY
# include "xlat/gpio_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

static int
print_gpiochip_info(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_gpiochip_info info;

	if (entering(tcp))
		return 0;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &info))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_CSTRING("{", info, name);
	PRINT_FIELD_CSTRING(", ", info, label);
	PRINT_FIELD_U(", ", info, lines);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

#include "xlat/gpio_line_flags.h"

static int
print_gpioline_info(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_gpioline_info info;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &info))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		PRINT_FIELD_U("{", info, line_offset);
		tprints("}");
		return 0;
	}

	/* exiting */
	PRINT_FIELD_FLAGS("{", info, flags, gpio_line_flags, "GPIOLINE_FLAG_???");
	PRINT_FIELD_CSTRING(", ", info, name);
	PRINT_FIELD_CSTRING(", ", info, consumer);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
print_gpioline_info_unwatch(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct { uint32_t offset; } data;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &data)) {
		PRINT_FIELD_U("{", data, offset);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

#include "xlat/gpio_handle_flags.h"

static int
print_gpiohandle_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_gpiohandle_request hr;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &hr))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		PRINT_FIELD_FD("{", hr, fd, tcp);
		tprints("}");
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	PRINT_FIELD_U("{", hr, lines);
	PRINT_FIELD_ARRAY_UPTO(", ", hr, lineoffsets, hr.lines, tcp,
			       print_uint32_array_member);
	PRINT_FIELD_FLAGS(", ", hr, flags, gpio_handle_flags,
			  "GPIOHANDLE_REQUEST_???");
	PRINT_FIELD_ARRAY_UPTO(", ", hr, default_values, hr.lines, tcp,
			       print_uint8_array_member);
	PRINT_FIELD_CSTRING(", ", hr, consumer_label);
	tprints("}");
	return 0;
}

#include "xlat/gpio_event_flags.h"

static int
print_gpioevent_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_gpioevent_request er;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &er))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		PRINT_FIELD_FD("{", er, fd, tcp);
		tprints("}");
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	PRINT_FIELD_U("{", er, lineoffset);
	PRINT_FIELD_FLAGS(", ", er, handleflags, gpio_handle_flags,
			  "GPIOHANDLE_REQUEST_???");
	PRINT_FIELD_FLAGS(", ", er, eventflags, gpio_event_flags,
			  "GPIOEVENT_REQUEST_???");
	PRINT_FIELD_CSTRING(", ", er, consumer_label);
	tprints("}");
	return 0;
}

static void
print_gpiohandle_data(struct tcb *const tcp, const struct_gpiohandle_data *vals)
{
	PRINT_FIELD_ARRAY("{", *vals, values, tcp, print_uint8_array_member);
	tprints("}");
}

static int
print_gpiohandle_get_values(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_gpiohandle_data vals;

	if (entering(tcp))
		return 0;

	/* exiting */
	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &vals))
		print_gpiohandle_data(tcp, &vals);

	return RVAL_IOCTL_DECODED;
}

static int
print_gpiohandle_set_values(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_gpiohandle_data vals;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &vals))
		print_gpiohandle_data(tcp, &vals);

	return RVAL_IOCTL_DECODED;
}

static int
print_gpiohandle_set_config(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_gpiohandle_config hc;

	tprints(", ");
	if (umove_or_printaddr(tcp, arg, &hc))
		return RVAL_IOCTL_DECODED;

	PRINT_FIELD_FLAGS("{", hc, flags, gpio_handle_flags, "GPIOHANDLE_REQUEST_???");
	PRINT_FIELD_ARRAY(", ", hc, default_values, tcp, print_uint8_array_member);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

int
gpio_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	switch (code) {
	case GPIO_GET_CHIPINFO_IOCTL:
		return print_gpiochip_info(tcp, arg);
	case GPIO_GET_LINEINFO_UNWATCH_IOCTL:
		return print_gpioline_info_unwatch(tcp, arg);
	case GPIO_GET_LINEINFO_IOCTL:
	case GPIO_GET_LINEINFO_WATCH_IOCTL:
		return print_gpioline_info(tcp, arg);
	case GPIO_GET_LINEHANDLE_IOCTL:
		return print_gpiohandle_request(tcp, arg);
	case GPIO_GET_LINEEVENT_IOCTL:
		return print_gpioevent_request(tcp, arg);
	case GPIOHANDLE_GET_LINE_VALUES_IOCTL:
		return print_gpiohandle_get_values(tcp, arg);
	case GPIOHANDLE_SET_LINE_VALUES_IOCTL:
		return print_gpiohandle_set_values(tcp, arg);
	case GPIOHANDLE_SET_CONFIG_IOCTL:
		return print_gpiohandle_set_config(tcp, arg);
	}
	return RVAL_DECODED;
}
