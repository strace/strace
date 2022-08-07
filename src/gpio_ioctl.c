/*
 * Copyright (c) 2020-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <linux/gpio.h>

static int
print_gpiochip_info(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct gpiochip_info info;

	if (entering(tcp))
		return 0;

	tprint_arg_next();
	if (umove_or_printaddr(tcp, arg, &info))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_CSTRING(info, name);
	tprint_struct_next();
	PRINT_FIELD_CSTRING(info, label);
	tprint_struct_next();
	PRINT_FIELD_U(info, lines);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

#include "xlat/gpio_line_flags.h"

static int
print_gpioline_info(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct gpioline_info info;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &info))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_U(info, line_offset);
		tprint_struct_end();
		return 0;
	}

	/* exiting */
	tprint_struct_begin();
	PRINT_FIELD_FLAGS(info, flags, gpio_line_flags, "GPIOLINE_FLAG_???");
	tprint_struct_next();
	PRINT_FIELD_CSTRING(info, name);
	tprint_struct_next();
	PRINT_FIELD_CSTRING(info, consumer);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
print_gpioline_info_unwatch(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct { uint32_t offset; } data;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &data)) {
		tprint_struct_begin();
		PRINT_FIELD_U(data, offset);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

#include "xlat/gpio_handle_flags.h"

static int
print_gpiohandle_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct gpiohandle_request hr;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &hr))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_FD(hr, fd, tcp);
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	tprint_struct_begin();
	PRINT_FIELD_U(hr, lines);
	tprint_struct_next();
	PRINT_FIELD_ARRAY_UPTO(hr, lineoffsets, hr.lines, tcp,
			       print_uint_array_member);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(hr, flags, gpio_handle_flags,
			  "GPIOHANDLE_REQUEST_???");
	tprint_struct_next();
	PRINT_FIELD_ARRAY_UPTO(hr, default_values, hr.lines, tcp,
			       print_uint_array_member);
	tprint_struct_next();
	PRINT_FIELD_CSTRING(hr, consumer_label);
	tprint_struct_end();
	return 0;
}

#include "xlat/gpio_event_flags.h"

static int
print_gpioevent_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct gpioevent_request er;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &er))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_FD(er, fd, tcp);
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	tprint_struct_begin();
	PRINT_FIELD_U(er, lineoffset);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(er, handleflags, gpio_handle_flags,
			  "GPIOHANDLE_REQUEST_???");
	tprint_struct_next();
	PRINT_FIELD_FLAGS(er, eventflags, gpio_event_flags,
			  "GPIOEVENT_REQUEST_???");
	tprint_struct_next();
	PRINT_FIELD_CSTRING(er, consumer_label);
	tprint_struct_end();
	return 0;
}

static void
print_gpiohandle_data(struct tcb *const tcp, const struct gpiohandle_data *vals)
{
	tprint_struct_begin();
	PRINT_FIELD_ARRAY(*vals, values, tcp, print_uint_array_member);
	tprint_struct_end();
}

static int
print_gpiohandle_get_values(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct gpiohandle_data vals;

	if (entering(tcp))
		return 0;

	/* exiting */
	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &vals))
		print_gpiohandle_data(tcp, &vals);

	return RVAL_IOCTL_DECODED;
}

static int
print_gpiohandle_set_values(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct gpiohandle_data vals;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &vals))
		print_gpiohandle_data(tcp, &vals);

	return RVAL_IOCTL_DECODED;
}

static int
print_gpiohandle_set_config(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct gpiohandle_config hc;

	tprint_arg_next();
	if (umove_or_printaddr(tcp, arg, &hc))
		return RVAL_IOCTL_DECODED;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(hc, flags, gpio_handle_flags, "GPIOHANDLE_REQUEST_???");
	tprint_struct_next();
	PRINT_FIELD_ARRAY(hc, default_values, tcp, print_uint_array_member);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

#include "xlat/gpio_v2_line_flags.h"
#include "xlat/gpio_v2_line_attr_ids.h"

static void
print_gpio_v2_line_attribute_raw(const struct gpio_v2_line_attribute *attr,
				 bool as_field)
{
	if (as_field) {
		tprints_field_name("attr");
		tprint_struct_begin();
	}
	PRINT_FIELD_U(*attr, id);
	if (attr->padding) {
		tprint_struct_next();
		PRINT_FIELD_X(*attr, padding);
	}
	tprint_struct_next();
	tprints_field_name("data");
	PRINT_VAL_X(attr->values);
	if (as_field)
		tprint_struct_end();
}

static void
print_gpio_v2_line_attribute(const struct gpio_v2_line_attribute *attr,
			     bool as_field)
{
	if (attr->padding) {
		/* unexpected padding usage so decode fields raw */
		print_gpio_v2_line_attribute_raw(attr, as_field);
		return;
	}
	switch (attr->id) {
	case GPIO_V2_LINE_ATTR_ID_FLAGS:
		PRINT_FIELD_FLAGS(*attr, flags, gpio_v2_line_flags,
				  "GPIO_V2_LINE_FLAG_???");
		break;
	case GPIO_V2_LINE_ATTR_ID_OUTPUT_VALUES:
		PRINT_FIELD_X(*attr, values);
		break;
	case GPIO_V2_LINE_ATTR_ID_DEBOUNCE:
		PRINT_FIELD_U(*attr, debounce_period_us);
		break;
	default:
		/* unknown id so decode fields raw */
		print_gpio_v2_line_attribute_raw(attr, as_field);
		break;
	}
}

static void
print_gpio_v2_line_config_attribute(const struct gpio_v2_line_config_attribute *attr)
{
	tprint_struct_begin();
	print_gpio_v2_line_attribute(&attr->attr, true);
	tprint_struct_next();
	PRINT_FIELD_X(*attr, mask);
	tprint_struct_end();
}

static bool
print_gpio_v2_line_attr_array_member(struct tcb *tcp, void *elem_buf,
				     size_t elem_size, void *data)
{
	tprint_struct_begin();
	print_gpio_v2_line_attribute(elem_buf, false);
	tprint_struct_end();

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
			  const struct gpio_v2_line_config *lc)
{
	tprint_struct_begin();
	PRINT_FIELD_FLAGS(*lc, flags, gpio_v2_line_flags,
			  "GPIO_V2_LINE_FLAG_???");
	tprint_struct_next();
	PRINT_FIELD_U(*lc, num_attrs);
	if (!IS_ARRAY_ZERO(lc->padding)) {
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(*lc, padding);
	}
	if (lc->num_attrs) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY_UPTO(*lc, attrs, lc->num_attrs, tcp,
				       print_gpio_v2_line_config_attr_array_member);
	}
	tprint_struct_end();
}

static int
print_gpio_v2_line_info(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct gpio_v2_line_info li;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &li))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_U(li, offset);
		tprint_struct_end();
		return 0;
	}

	/* exiting */
	tprint_struct_begin();
	PRINT_FIELD_CSTRING(li, name);
	tprint_struct_next();
	PRINT_FIELD_CSTRING(li, consumer);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(li, flags, gpio_v2_line_flags, "GPIO_V2_LINE_FLAG_???");
	tprint_struct_next();
	PRINT_FIELD_U(li, num_attrs);
	if (li.num_attrs) {
		tprint_struct_next();
		PRINT_FIELD_ARRAY_UPTO(li, attrs, li.num_attrs, tcp,
				       print_gpio_v2_line_attr_array_member);
	}
	if (!IS_ARRAY_ZERO(li.padding)) {
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(li, padding);
	}
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
print_gpio_v2_line_request(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct gpio_v2_line_request lr;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &lr))
		return RVAL_IOCTL_DECODED;

	if (exiting(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_FD(lr, fd, tcp);
		tprint_struct_end();
		return RVAL_IOCTL_DECODED;
	}

	/* entering */
	tprint_struct_begin();
	PRINT_FIELD_U(lr, num_lines);
	tprint_struct_next();
	PRINT_FIELD_ARRAY_UPTO(lr, offsets, lr.num_lines, tcp,
			       print_uint_array_member);
	tprint_struct_next();
	PRINT_FIELD_CSTRING(lr, consumer);
	tprint_struct_next();
	PRINT_FIELD_OBJ_TCB_PTR(lr, config, tcp,
				print_gpio_v2_line_config);
	if (lr.event_buffer_size) {
		tprint_struct_next();
		PRINT_FIELD_U(lr, event_buffer_size);
	}
	if (!IS_ARRAY_ZERO(lr.padding)) {
		tprint_struct_next();
		PRINT_FIELD_X_ARRAY(lr, padding);
	}
	tprint_struct_end();
	return 0;
}

static int
print_gpio_v2_line_get_values(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct gpio_v2_line_values vals;

	if (entering(tcp))
		tprint_arg_next();
	else if (syserror(tcp))
		return RVAL_IOCTL_DECODED;
	else
		tprint_value_changed();

	if (umove_or_printaddr(tcp, arg, &vals))
		return RVAL_IOCTL_DECODED;

	if (entering(tcp)) {
		tprint_struct_begin();
		PRINT_FIELD_X(vals, mask);
		tprint_struct_end();
		return 0;
	}

	/* exiting */
	tprint_struct_begin();
	PRINT_FIELD_X(vals, bits);
	tprint_struct_end();

	return RVAL_IOCTL_DECODED;
}

static int
print_gpio_v2_line_set_values(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct gpio_v2_line_values vals;

	tprint_arg_next();
	if (!umove_or_printaddr(tcp, arg, &vals)) {
		tprint_struct_begin();
		PRINT_FIELD_X(vals, bits);
		tprint_struct_next();
		PRINT_FIELD_X(vals, mask);
		tprint_struct_end();
	}

	return RVAL_IOCTL_DECODED;
}

static int
print_gpio_v2_line_set_config(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct gpio_v2_line_config lc;

	tprint_arg_next();
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
