/*
 * Check GPIO_* ioctl decoding.
 *
 * Copyright (c) 2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include "types/gpio.h"
#define XLAT_MACROS_ONLY
# include "xlat/gpio_event_flags.h"
# include "xlat/gpio_handle_flags.h"
# include "xlat/gpio_ioctl_cmds.h"
# include "xlat/gpio_line_flags.h"
#undef XLAT_MACROS_ONLY

#ifdef HAVE_STRUCT_GPIOCHIP_INFO
# define struct_gpiochip_info struct gpiochip_info
#endif

#ifdef HAVE_STRUCT_GPIOLINE_INFO
# define struct_gpioline_info struct gpioline_info
#endif

#ifdef HAVE_STRUCT_GPIOHANDLE_REQUEST
# define struct_gpiohandle_request struct gpiohandle_request
#endif

#ifdef HAVE_STRUCT_GPIOEVENT_REQUEST
# define struct_gpioevent_request struct gpioevent_request
#endif

#ifdef HAVE_STRUCT_GPIOHANDLE_DATA
# define struct_gpiohandle_data struct gpiohandle_data
#endif

#ifdef HAVE_STRUCT_GPIOHANDLE_CONFIG
# define struct_gpiohandle_config struct gpiohandle_config
#endif

# define str_event_flags	XLAT_KNOWN(0x3, "GPIOEVENT_REQUEST_BOTH_EDGES")
# define str_handle_flags	XLAT_KNOWN(0x14, \
	"GPIOHANDLE_REQUEST_ACTIVE_LOW|GPIOHANDLE_REQUEST_OPEN_SOURCE")
# define str_info_flags		XLAT_KNOWN(0xc, \
	"GPIOLINE_FLAG_ACTIVE_LOW|GPIOLINE_FLAG_OPEN_DRAIN")

#define UNK_GPIO_FLAG 0x8000

# define str_handle_unk_flag	XLAT_UNKNOWN(UNK_GPIO_FLAG, "GPIOHANDLE_REQUEST_???")
# define str_info_unk_flag	XLAT_UNKNOWN(UNK_GPIO_FLAG, "GPIOLINE_FLAG_???")

#if VERBOSE
# define str_line_seq		"[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, " \
		"16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, " \
		"33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, " \
		"50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64]"
# define str_default_seq	"[1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, " \
		"29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, " \
		"63, 65, 67, 69, 71, 73, 75, 77, 79, 81, 83, 85, 87, 89, 91, 93, 95, " \
		"97, 99, 101, 103, 105, 107, 109, 111, 113, 115, 117, 119, 121, 123, " \
		"125, 127]"
#else
# define str_line_seq		"[1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, " \
		"16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, ...]"
# define str_default_seq	"[1, 3, 5, 7, 9, 11, 13, 15, 17, 19, 21, 23, 25, 27, " \
		"29, 31, 33, 35, 37, 39, 41, 43, 45, 47, 49, 51, 53, 55, 57, 59, 61, " \
		"63, ...]"
#endif

static const char *errstr;

static long
do_ioctl(kernel_ulong_t cmd, kernel_ulong_t arg)
{
	long rc = ioctl(-1, cmd, arg);

	errstr = sprintrc(rc);

#ifdef INJECT_RETVAL
	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Got a return value of %ld != %ld",
				   rc, (long) INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
#endif

	return rc;
}

static inline long
do_ioctl_ptr(kernel_ulong_t cmd, const void *arg)
{
	return do_ioctl(cmd, (uintptr_t) arg);
}

static void
test_print_gpiochip_info(void)
{
	long rc;

	do_ioctl(GPIO_GET_CHIPINFO_IOCTL, 0);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(GPIO_GET_CHIPINFO_IOCTL), errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct_gpiochip_info, p_chipinfo);

	p_chipinfo->lines = 0xca;
	strcpy(p_chipinfo->name, "chip name");
	strcpy(p_chipinfo->label, "chip label");

	do_ioctl_ptr(GPIO_GET_CHIPINFO_IOCTL, (char *) p_chipinfo + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(GPIO_GET_CHIPINFO_IOCTL), (char *) p_chipinfo + 1, errstr);

	rc = do_ioctl_ptr(GPIO_GET_CHIPINFO_IOCTL, p_chipinfo);
	printf("ioctl(-1, %s, ", XLAT_STR(GPIO_GET_CHIPINFO_IOCTL));
	if (rc >= 0)
		printf("{name=\"chip name\", label=\"chip label\", lines=202}");
	else
		printf("%p", p_chipinfo);
	printf(") = %s\n", errstr);
}

static void
test_print_gpioline_info(void)
{
	long rc;

	do_ioctl(GPIO_GET_LINEINFO_IOCTL, 0);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(GPIO_GET_LINEINFO_IOCTL), errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct_gpioline_info, p_lineinfo);

	p_lineinfo->line_offset = 0x32;
	p_lineinfo->flags = GPIOLINE_FLAG_ACTIVE_LOW|GPIOLINE_FLAG_OPEN_DRAIN;
	strcpy(p_lineinfo->name, "line name");
	strcpy(p_lineinfo->consumer, "line consumer");

	do_ioctl_ptr(GPIO_GET_LINEINFO_IOCTL, (char *) p_lineinfo + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(GPIO_GET_LINEINFO_IOCTL), (char *) p_lineinfo + 1, errstr);

	/* GPIO_GET_LINEINFO_IOCTL */
	rc = do_ioctl_ptr(GPIO_GET_LINEINFO_IOCTL, p_lineinfo);
	printf("ioctl(-1, %s, {line_offset=50}",
	       XLAT_STR(GPIO_GET_LINEINFO_IOCTL));
	if (rc >= 0)
		printf(" => {flags=" str_info_flags
		       ", name=\"line name\", consumer=\"line consumer\"}");
	printf(") = %s\n", errstr);

	/* GPIO_GET_LINEINFO_WATCH_IOCTL */
	rc = do_ioctl_ptr(GPIO_GET_LINEINFO_WATCH_IOCTL, p_lineinfo);
	printf("ioctl(-1, %s, {line_offset=50}",
	       XLAT_STR(GPIO_GET_LINEINFO_WATCH_IOCTL));
	if (rc >= 0)
		printf(" => {flags=" str_info_flags
		       ", name=\"line name\", consumer=\"line consumer\"}");
	printf(") = %s\n", errstr);

	/* unknown flag */
	p_lineinfo->flags = UNK_GPIO_FLAG;
	rc = do_ioctl_ptr(GPIO_GET_LINEINFO_IOCTL, p_lineinfo);
	printf("ioctl(-1, %s, {line_offset=50}",
	       XLAT_STR(GPIO_GET_LINEINFO_IOCTL));
	if (rc >= 0)
		printf(" => {flags=" str_info_unk_flag
		       ", name=\"line name\", consumer=\"line consumer\"}");
	printf(") = %s\n", errstr);
}

static void
test_print_gpioline_info_unwatch(void)
{
	do_ioctl_ptr(GPIO_GET_LINEINFO_UNWATCH_IOCTL, 0);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(GPIO_GET_LINEINFO_UNWATCH_IOCTL), errstr);

	TAIL_ALLOC_OBJECT_VAR_PTR(uint32_t, p_offset);

	*p_offset = 0;
	do_ioctl_ptr(GPIO_GET_LINEINFO_UNWATCH_IOCTL, p_offset);
	printf("ioctl(-1, %s, {offset=0}) = %s\n",
	       XLAT_STR(GPIO_GET_LINEINFO_UNWATCH_IOCTL), errstr);

	*p_offset = 78;
	do_ioctl_ptr(GPIO_GET_LINEINFO_UNWATCH_IOCTL, p_offset);
	printf("ioctl(-1, %s, {offset=78}) = %s\n",
	       XLAT_STR(GPIO_GET_LINEINFO_UNWATCH_IOCTL), errstr);
}

static void
test_print_gpiohandle_request(void)
{
	long rc;

	do_ioctl(GPIO_GET_LINEHANDLE_IOCTL, 0);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(GPIO_GET_LINEHANDLE_IOCTL), errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct_gpiohandle_request, p_handle_request);

	p_handle_request->lines = 4;
	p_handle_request->lineoffsets[0] = 0x12;
	p_handle_request->lineoffsets[1] = 0x23;
	p_handle_request->lineoffsets[2] = 0x34;
	p_handle_request->lineoffsets[3] = 0x45;
	p_handle_request->default_values[0] = 0x0;
	p_handle_request->default_values[1] = 0x1;
	p_handle_request->default_values[2] = 0x2;
	p_handle_request->default_values[3] = 0x6;
	p_handle_request->flags = GPIOHANDLE_REQUEST_ACTIVE_LOW|GPIOHANDLE_REQUEST_OPEN_SOURCE;
	strcpy(p_handle_request->consumer_label, "line consumer");
	p_handle_request->fd = 0x64;

	do_ioctl_ptr(GPIO_GET_LINEHANDLE_IOCTL, (char *) p_handle_request + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(GPIO_GET_LINEHANDLE_IOCTL), (char *) p_handle_request + 1, errstr);

	rc = do_ioctl_ptr(GPIO_GET_LINEHANDLE_IOCTL, p_handle_request);
	printf("ioctl(-1, %s, {lines=4, lineoffsets=[18, 35, 52, 69], flags=" str_handle_flags
	       ", default_values=[0, 1, 2, 6], consumer_label=\"line consumer\"}",
	       XLAT_STR(GPIO_GET_LINEHANDLE_IOCTL));
	if (rc >= 0)
		printf(" => {fd=100}");
	printf(") = %s\n", errstr);

	/* lines > GPIOHANDLES_MAX */
	p_handle_request->lines = GPIOHANDLES_MAX + 1;
	for (int i = 0; i < GPIOHANDLES_MAX; i++) {
		p_handle_request->lineoffsets[i] = i + 1;
		p_handle_request->default_values[i] = 2*i + 1;
	}
	rc = do_ioctl_ptr(GPIO_GET_LINEHANDLE_IOCTL, p_handle_request);
	printf("ioctl(-1, %s, {lines=65, lineoffsets=" str_line_seq
	       ", flags=" str_handle_flags ", default_values=" str_default_seq
	       ", consumer_label=\"line consumer\"}",
	       XLAT_STR(GPIO_GET_LINEHANDLE_IOCTL));
	if (rc >= 0)
		printf(" => {fd=100}");
	printf(") = %s\n", errstr);
}

static void
test_print_gpioevent_request(void)
{
	long rc;

	do_ioctl(GPIO_GET_LINEEVENT_IOCTL, 0);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(GPIO_GET_LINEEVENT_IOCTL), errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct_gpioevent_request, p_event_request);

	p_event_request->lineoffset = 4;
	p_event_request->handleflags = GPIOHANDLE_REQUEST_ACTIVE_LOW|GPIOHANDLE_REQUEST_OPEN_SOURCE;
	p_event_request->eventflags = GPIOEVENT_REQUEST_BOTH_EDGES;
	strcpy(p_event_request->consumer_label, "line consumer");
	p_event_request->fd = 0x65;

	do_ioctl_ptr(GPIO_GET_LINEEVENT_IOCTL, (char *) p_event_request + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(GPIO_GET_LINEEVENT_IOCTL), (char *) p_event_request + 1, errstr);

	rc = do_ioctl_ptr(GPIO_GET_LINEEVENT_IOCTL, p_event_request);
	printf("ioctl(-1, %s, {lineoffset=4, handleflags=" str_handle_flags
	       ", eventflags=" str_event_flags ", consumer_label=\"line consumer\"}",
	       XLAT_STR(GPIO_GET_LINEEVENT_IOCTL));
	if (rc >= 0)
		printf(" => {fd=101}");
	printf(") = %s\n", errstr);
}

static void
test_print_gpiohandle_get_values(void)
{
	long rc;

	do_ioctl(GPIOHANDLE_GET_LINE_VALUES_IOCTL, 0);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(GPIOHANDLE_GET_LINE_VALUES_IOCTL), errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct_gpiohandle_data, p_handle_data);

	for (int i = 0; i < GPIOHANDLES_MAX; i++)
		p_handle_data->values[i] = i + 1;

	do_ioctl_ptr(GPIOHANDLE_GET_LINE_VALUES_IOCTL, (char *) p_handle_data + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(GPIOHANDLE_GET_LINE_VALUES_IOCTL), (char *) p_handle_data + 1, errstr);

	rc = do_ioctl_ptr(GPIOHANDLE_GET_LINE_VALUES_IOCTL, p_handle_data);
	printf("ioctl(-1, %s, ", XLAT_STR(GPIOHANDLE_GET_LINE_VALUES_IOCTL));
	if (rc >= 0)
		printf("{values=" str_line_seq "}");
	else
		printf("%p", p_handle_data);
	printf(") = %s\n", errstr);
}

static void
test_print_gpiohandle_set_values(void)
{
	do_ioctl(GPIOHANDLE_SET_LINE_VALUES_IOCTL, 0);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(GPIOHANDLE_SET_LINE_VALUES_IOCTL), errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct_gpiohandle_data, p_handle_data);

	for (int i = 0; i < GPIOHANDLES_MAX; i++)
		p_handle_data->values[i] = i + 1;

	do_ioctl_ptr(GPIOHANDLE_SET_LINE_VALUES_IOCTL, (char *) p_handle_data + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(GPIOHANDLE_SET_LINE_VALUES_IOCTL), (char *) p_handle_data + 1, errstr);

	do_ioctl_ptr(GPIOHANDLE_SET_LINE_VALUES_IOCTL, p_handle_data);
	printf("ioctl(-1, %s, {values=" str_line_seq "}) = %s\n",
	       XLAT_STR(GPIOHANDLE_SET_LINE_VALUES_IOCTL),
	       errstr);
}

static void
test_print_gpiohandle_set_config(void)
{
	do_ioctl(GPIOHANDLE_SET_CONFIG_IOCTL, 0);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(GPIOHANDLE_SET_CONFIG_IOCTL), errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct_gpiohandle_config, p_handle_config);

	p_handle_config->flags = GPIOHANDLE_REQUEST_ACTIVE_LOW|GPIOHANDLE_REQUEST_OPEN_SOURCE;
	for (int i = 0; i < GPIOHANDLES_MAX; i++)
		p_handle_config->default_values[i] = i + 1;

	do_ioctl_ptr(GPIOHANDLE_SET_CONFIG_IOCTL, (char *) p_handle_config + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(GPIOHANDLE_SET_CONFIG_IOCTL), (char *) p_handle_config + 1, errstr);

	do_ioctl_ptr(GPIOHANDLE_SET_CONFIG_IOCTL, p_handle_config);
	printf("ioctl(-1, %s, {flags=" str_handle_flags
	       ", default_values=" str_line_seq "}) = %s\n",
	       XLAT_STR(GPIOHANDLE_SET_CONFIG_IOCTL), errstr);

	/* unknown flag */
	p_handle_config->flags = UNK_GPIO_FLAG;
	do_ioctl_ptr(GPIOHANDLE_SET_CONFIG_IOCTL, p_handle_config);
	printf("ioctl(-1, %s, {flags=" str_handle_unk_flag
	       ", default_values=" str_line_seq "}) = %s\n",
	       XLAT_STR(GPIOHANDLE_SET_CONFIG_IOCTL), errstr);
}

int
main(int argc, char *argv[])
{
	unsigned long unknown_gpio_cmd =
		_IOC(_IOC_READ|_IOC_WRITE, 0xb4, 0x5e, 0xfed) |
		(unsigned long) 0xfacefeed00000000ULL;
	unsigned long cmd_arg = (unsigned long) 0xdeadbeefbadc0dedULL;
#ifdef INJECT_RETVAL
	unsigned long num_skip;
	bool locked = false;

	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		long ret = ioctl(-1, GPIO_GET_CHIPINFO_IOCTL, 0);

		printf("ioctl(-1, %s, NULL) = %s%s\n",
		       XLAT_STR(GPIO_GET_CHIPINFO_IOCTL), sprintrc(ret),
		       ret == INJECT_RETVAL ? " (INJECTED)" : "");

		if (ret != INJECT_RETVAL)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Hasn't locked on ioctl(-1"
				   ", GPIO_GET_CHIPINFO_IOCTL, NULL) returning %d",
				   INJECT_RETVAL);
#endif
	/* unknown GPIO command */
	do_ioctl(unknown_gpio_cmd, cmd_arg);
	printf("ioctl(-1, " XLAT_FMT ", %#lx) = %s\n",
		XLAT_SEL((unsigned int) unknown_gpio_cmd,
		"_IOC(_IOC_READ|_IOC_WRITE, 0xb4, 0x5e, 0xfed)"),
		cmd_arg, errstr);

	/* GPIO v1 ioctls */
	test_print_gpiochip_info();
	test_print_gpioline_info();
	test_print_gpioline_info_unwatch();
	test_print_gpiohandle_request();
	test_print_gpioevent_request();
	test_print_gpiohandle_get_values();
	test_print_gpiohandle_set_values();
	test_print_gpiohandle_set_config();

	puts("+++ exited with 0 +++");
	return 0;
}
