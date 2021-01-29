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
	tprint_struct_next();
	PRINT_FIELD_ARRAY_UPTO(hr, lineoffsets, hr.lines, tcp,
			       print_uint32_array_member);
	PRINT_FIELD_FLAGS(", ", hr, flags, gpio_handle_flags,
			  "GPIOHANDLE_REQUEST_???");
	tprint_struct_next();
	PRINT_FIELD_ARRAY_UPTO(hr, default_values, hr.lines, tcp,
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
	tprint_struct_begin();
	PRINT_FIELD_ARRAY(*vals, values, tcp, print_uint8_array_member);
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
	tprint_struct_next();
	PRINT_FIELD_ARRAY(hc, default_values, tcp, print_uint8_array_member);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

#include "xlat/gpio_v2_line_flags.h"
#include "xlat/gpio_v2_line_attr_ids.h"

static void
print_gpio_v2_line_attribute_raw(const struct_gpio_v2_line_attribute *attr,
				 bool as_field)
{
	if (as_field)
		tprints("attr={");
	PRINT_FIELD_U("", *attr, id);
	if (attr->padding) {
		PRINT_FIELD_X(", ", *attr, padding);
	}
	tprintf(", data=%#" PRIx64, attr->values);
	if (as_field)
		tprints("}");
}

static void
print_gpio_v2_line_attribute(const struct_gpio_v2_line_attribute *attr,
			     bool as_field)
{
	if (attr->padding) {
		/* unexpected padding usage so decode fields raw */
		print_gpio_v2_line_attribute_raw(attr, as_field);
		return;
	}
	switch (attr->id) {
	case GPIO_V2_LINE_ATTR_ID_FLAGS:
		PRINT_FIELD_FLAGS("", *attr, flags, gpio_v2_line_flags,
				  "GPIO_V2_LINE_FLAG_???");
		break;
	case GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES:
		PRINT_FIELD_X("", *attr, values);
		break;
	case GPIO_V2_LINE_ATTR_ID_DEBOUNCE:
		PRINT_FIELD_U("", *attr, debounce_period_us);
		break;
	default:
		/* unknown id so decode fields raw */
		print_gpio_v2_line_attribute_raw(attr, as_field);
		break;
	}
}

static void
print_gpio_v2_line_config_attribute(const struct_gpio_v2_line_config_attribute *attr)
{
	tprints("{");
	print_gpio_v2_line_attribute(&attr->attr, true);
	PRINT_FIELD_X(", ", *attr, mask);
	tprints("}");
}

static bool
print_gpio_v2_line_attr_array_member(struct tcb *tcp, void *elem_buf,
				     size_t elem_size, void *data)
{
	tprints("{");
	print_gpio_v2_line_attribute(elem_buf, false);
	tprints("}");

	return true;
}

static bool
print_gpio_v2_line_config_attr_array_member(struct tcb *tcp, void *elem_buf,
					    size_t elem_size, void *data)
{
	print_gpio_v2_line_config_attribute(elem_buf);

	return true;
}

static void
print_gpio_v2_line_config(struct tcb *const tcp,
			  const struct_gpio_v2_line_config *lc)
{
	PRINT_FIELD_FLAGS("{", *lc, flags, gpio_v2_line_flags,
			  "GPIO_V2_LINE_FLAG_???");
	PRINT_FIELD_U(", ", *lc, num_attrs);
	if (!IS_ARRAY_ZERO(lc->padding)) {
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(*lc, padding);
	}
	if (lc->num_attrs) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY_UPTO(*lc, attrs, lc->num_attrs, tcp,
				       print_gpio_v2_line_config_attr_array_member);
	}
	tprints("}");
}

static int
print_gpio_v2_line_info(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_gpio_v2_line_info li;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &li))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		PRINT_FIELD_U("{", li, offset);
		tprints("}");
		return 0;
	}

	/* exiting */
	PRINT_FIELD_CSTRING("{", li, name);
	PRINT_FIELD_CSTRING(", ", li, consumer);
	PRINT_FIELD_FLAGS(", ", li, flags, gpio_v2_line_flags, "GPIO_V2_LINE_FLAG_???");
	PRINT_FIELD_U(", ", li, num_attrs);
	if (li.num_attrs) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY_UPTO(li, attrs, li.num_attrs, tcp,
				       print_gpio_v2_line_attr_array_member);
	}
	if (!IS_ARRAY_ZERO(li.padding)) {
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(li, padding);
	}
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
print_gpio_v2_line_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_gpio_v2_line_request lr;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &lr))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		PRINT_FIELD_FD("{", lr, fd, tcp);
		tprints("}");
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	PRINT_FIELD_U("{", lr, num_lines);
	tprint_struct_next();
	PRINT_FIELD_ARRAY_UPTO(lr, offsets, lr.num_lines, tcp,
			       print_uint32_array_member);
	tprint_struct_next();
	PRINT_FIELD_OBJ_TCB_PTR(lr, config, tcp,
				print_gpio_v2_line_config);
	PRINT_FIELD_CSTRING(", ", lr, consumer);
	if (lr.event_buffer_size) {
		PRINT_FIELD_U(", ", lr, event_buffer_size);
	}
	if (!IS_ARRAY_ZERO(lr.padding)) {
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(lr, padding);
	}
	tprints("}");
	return 0;
}

static int
print_gpio_v2_line_get_values(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_gpio_v2_line_values vals;

	if (entering(tcp))
		tprints(", ");
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprints(" => ");

	if (umove_or_printaddr(tcp, arg, &vals))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		PRINT_FIELD_X("{", vals, mask);
		tprints("}");
		return 0;
	}

	/* exiting */
	PRINT_FIELD_X("{", vals, bits);
	tprints("}");

	return RVAL_IOCTL_DECODED;
}

static int
print_gpio_v2_line_set_values(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_gpio_v2_line_values vals;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &vals)) {
		PRINT_FIELD_X("{", vals, bits);
		PRINT_FIELD_X(", ", vals, mask);
		tprints("}");
	}

	return RVAL_IOCTL_DECODED;
}

static int
print_gpio_v2_line_set_config(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct_gpio_v2_line_config lc;

	tprints(", ");
	if (!umove_or_printaddr(tcp, arg, &lc))
		print_gpio_v2_line_config(tcp, &lc);

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
	case GPIO_V2_GET_LINEINFO_IOCTL:
	case GPIO_V2_GET_LINEINFO_WATCH_IOCTL:
		return print_gpio_v2_line_info(tcp, arg);
	case GPIO_V2_GET_LINE_IOCTL:
		return print_gpio_v2_line_request(tcp, arg);
	case GPIO_V2_LINE_SET_CONFIG_IOCTL:
		return print_gpio_v2_line_set_config(tcp, arg);
	case GPIO_V2_LINE_GET_VALUES_IOCTL:
		return print_gpio_v2_line_get_values(tcp, arg);
	case GPIO_V2_LINE_SET_VALUES_IOCTL:
		return print_gpio_v2_line_set_values(tcp, arg);
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
