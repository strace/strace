/*
 * Check HDIO_* ioctl decoding.
 *
 * Copyright (c) 2020-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/hdreg.h>
#include <sys/ioctl.h>
#include "xlat.h"

#include "xlat/hdio_busstates.h"
#include "xlat/hdio_ide_nice.h"

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

static long
do_ioctl_ptr(kernel_ulong_t cmd, const void *arg)
{
	return do_ioctl(cmd, (uintptr_t) arg);
}

int
main(int argc, char *argv[])
{
#ifdef INJECT_RETVAL
	unsigned long num_skip;
	bool locked = false;

	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		long ret = ioctl(-1, HDIO_GET_QDMA, 0);

		printf("ioctl(-1, %s, 0) = %s%s\n",
		       XLAT_STR(HDIO_GET_QDMA), sprintrc(ret),
		       ret == INJECT_RETVAL ? " (INJECTED)" : "");

		if (ret != INJECT_RETVAL)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Hasn't locked on ioctl(-1"
				   ", HDIO_GET_QDMA, 0) returning %d",
				   INJECT_RETVAL);
#endif

	long rc;

	/* Decoding is not supported */
	static const struct {
		uint32_t cmd;
		const char *str;
	} unsupp_cmds[] = {
		{ ARG_STR(HDIO_GET_QDMA) },
		{ ARG_STR(HDIO_SET_XFER) },
		{ ARG_STR(HDIO_TRISTATE_HWIF) },
		{ ARG_STR(HDIO_DRIVE_TASKFILE) },
		{ ARG_STR(HDIO_DRIVE_TASK) },
		{ ARG_STR(HDIO_SET_QDMA) },
#ifdef MPERS_IS_m32
		{ ARG_STR(HDIO_GET_UNMASKINTR) },
		{ ARG_STR(HDIO_GET_MULTCOUNT) },
		{ ARG_STR(HDIO_OBSOLETE_IDENTITY) },
		{ ARG_STR(HDIO_GET_KEEPSETTINGS) },
		{ ARG_STR(HDIO_GET_32BIT) },
		{ ARG_STR(HDIO_GET_NOWERR) },
		{ ARG_STR(HDIO_GET_DMA) },
		{ ARG_STR(HDIO_GET_NICE) },
		{ ARG_STR(HDIO_GET_IDENTITY) },
		{ ARG_STR(HDIO_GET_WCACHE) },
		{ ARG_STR(HDIO_GET_ACOUSTIC) },
		{ ARG_STR(HDIO_GET_ADDRESS) },
		{ ARG_STR(HDIO_GET_BUSSTATE) },
		{ ARG_STR(HDIO_DRIVE_RESET) },
		{ ARG_STR(HDIO_SET_MULTCOUNT) },
		{ ARG_STR(HDIO_SET_UNMASKINTR) },
		{ ARG_STR(HDIO_SET_KEEPSETTINGS) },
		{ ARG_STR(HDIO_SET_32BIT) },
		{ ARG_STR(HDIO_SET_NOWERR) },
		{ ARG_STR(HDIO_SET_DMA) },
		{ ARG_STR(HDIO_SET_PIO_MODE) },
		{ ARG_STR(HDIO_SCAN_HWIF) },
		{ ARG_STR(HDIO_UNREGISTER_HWIF) },
		{ ARG_STR(HDIO_SET_NICE) },
		{ ARG_STR(HDIO_SET_WCACHE) },
		{ ARG_STR(HDIO_SET_ACOUSTIC) },
		{ ARG_STR(HDIO_SET_BUSSTATE) },
		{ ARG_STR(HDIO_SET_ADDRESS) },
#endif /* MPERS_IS_m32 */
	};

	for (size_t i = 0; i < ARRAY_SIZE(unsupp_cmds); i++) {
		do_ioctl(unsupp_cmds[i].cmd, 0);
		printf("ioctl(-1, " XLAT_FMT ", 0) = %s\n",
		       XLAT_SEL(unsupp_cmds[i].cmd, unsupp_cmds[i].str),
		       errstr);

		do_ioctl(unsupp_cmds[i].cmd,
			 (unsigned long) 0xbadc0deddeadc0deULL);
		printf("ioctl(-1, " XLAT_FMT ", %#lx) = %s\n",
		       XLAT_SEL(unsupp_cmds[i].cmd, unsupp_cmds[i].str),
		       (unsigned long) 0xbadc0deddeadc0deULL, errstr);
	}


	/* HDIO_GETGEO */
	do_ioctl(HDIO_GETGEO, 0);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(HDIO_GETGEO), errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct hd_geometry, p_hd_geom);

	p_hd_geom->heads = 0xca;
	p_hd_geom->sectors = 0xfe;
	p_hd_geom->cylinders = 0xbabe;
	p_hd_geom->start = (unsigned long) 0xbadc0deddeadfaceULL;

	do_ioctl_ptr(HDIO_GETGEO, (char *) p_hd_geom + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(HDIO_GETGEO), (char *) p_hd_geom + 1, errstr);

	rc = do_ioctl_ptr(HDIO_GETGEO, p_hd_geom);
	printf("ioctl(-1, %s, ", XLAT_STR(HDIO_GETGEO));
	if (rc >= 0) {
		printf("{heads=202, sectors=254, cylinders=47806, start=%lu}",
		       (unsigned long) 0xbadc0deddeadfaceULL);
	} else {
		printf("%p", p_hd_geom);
	}
	printf(") = %s\n", errstr);

	/* HDIO_DRIVE_CMD */
	do_ioctl(HDIO_DRIVE_CMD, 0);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(HDIO_DRIVE_CMD), errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct hd_drive_cmd_hdr, p_hd_drive_cmd);
	struct hd_drive_cmd_hdr *p_hd_drive_cmd2 =
		tail_alloc(sizeof(*p_hd_drive_cmd2) + 16);
	struct hd_drive_cmd_hdr *p_hd_drive_cmd3 =
		tail_alloc(sizeof(*p_hd_drive_cmd3) + DEFAULT_STRLEN + 1);

	fill_memory(p_hd_drive_cmd2, sizeof(*p_hd_drive_cmd2) + 16);
	fill_memory(p_hd_drive_cmd3,
		    sizeof(*p_hd_drive_cmd3) + DEFAULT_STRLEN + 1);

	p_hd_drive_cmd->command = 0xca;
	p_hd_drive_cmd->sector_number = 0xff;
	p_hd_drive_cmd->feature = 0xee;
	p_hd_drive_cmd->sector_count = 0;

	do_ioctl_ptr(HDIO_DRIVE_CMD, (char *) p_hd_drive_cmd + 1);
	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(HDIO_DRIVE_CMD), (char *) p_hd_drive_cmd + 1, errstr);

	for (size_t i = 0; i < 2; i++) {
		p_hd_drive_cmd->sector_count = i;

		rc = do_ioctl_ptr(HDIO_DRIVE_CMD, p_hd_drive_cmd);
		printf("ioctl(-1, %s, {command=" XLAT_FMT ", sector_number=255"
		       ", feature=238, sector_count=%zu",
		       XLAT_STR(HDIO_DRIVE_CMD),
		       XLAT_SEL(0xca, "ATA_CMD_WRITE"), i);
		if (rc >= 0) {
			printf("} => {status=0xca, error=255, nsector=238");
			if (i)
				printf(", buf=%p", p_hd_drive_cmd + 1);
		}
		printf("}) = %s\n", errstr);
	}

	rc = do_ioctl_ptr(HDIO_DRIVE_CMD, p_hd_drive_cmd2);
	printf("ioctl(-1, %s, {command=0x80" NRAW(" /* ATA_CMD_??? */")
	       ", sector_number=129, feature=130, sector_count=131",
	       XLAT_STR(HDIO_DRIVE_CMD));
	if (rc >= 0) {
		printf("} => {status=0x80, error=129, nsector=130, buf=%p",
		       p_hd_drive_cmd2 + 1);
	}
	printf("}) = %s\n", errstr);

	rc = do_ioctl_ptr(HDIO_DRIVE_CMD, p_hd_drive_cmd3);
	printf("ioctl(-1, %s, {command=0x80" NRAW(" /* ATA_CMD_??? */")
	       ", sector_number=129, feature=130, sector_count=131",
	       XLAT_STR(HDIO_DRIVE_CMD));
	if (rc >= 0) {
		printf("} => {status=0x80, error=129, nsector=130, buf=");
		print_quoted_hex(p_hd_drive_cmd3 + 1, DEFAULT_STRLEN);
		printf("...");
	}
	printf("}) = %s\n", errstr);

/* HDIO compat has never been supported by the kernel.  */
#ifndef MPERS_IS_m32

	void *const efault = tail_alloc(1) + 1;

	/* HDIO_DRIVE_RESET */
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(HDIO_DRIVE_RESET), errstr);
	do_ioctl_ptr(HDIO_DRIVE_RESET, 0);

	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(HDIO_DRIVE_RESET), efault, errstr);
	do_ioctl(HDIO_DRIVE_RESET, (uintptr_t) efault);

	printf("ioctl(-1, %s, [%#x, %#x, %#x]) = %s\n",
	       XLAT_STR(HDIO_DRIVE_RESET),
	       (unsigned int) 0xdeadbeef,
	       (unsigned int) 0xAAAAAAAA,
	       (unsigned int) 0xbeefbeef,
	       errstr);
	int drive_reset_args[3] = {0xdeadbeef, 0xAAAAAAAA, 0xbeefbeef};
	do_ioctl_ptr(HDIO_DRIVE_RESET, &drive_reset_args);

	/* HDIO_SCAN_HWIF */
	printf("ioctl(-1, %s, NULL) = %s\n", XLAT_STR(HDIO_SCAN_HWIF), errstr);
	do_ioctl_ptr(HDIO_SCAN_HWIF, 0);

	printf("ioctl(-1, %s, %p) = %s\n",
	       XLAT_STR(HDIO_SCAN_HWIF), efault, errstr);
	do_ioctl(HDIO_SCAN_HWIF, (uintptr_t) efault);

	printf("ioctl(-1, %s, [%#x, %#x, %#x]) = %s\n",
	       XLAT_STR(HDIO_SCAN_HWIF),
	       (unsigned int) 0xdeadbeef,
	       (unsigned int) 0xAAAAAAAA,
	       (unsigned int) 0xbeefbeef,
	       errstr);
	int scan_hwif_args[3] = {0xdeadbeef, 0xAAAAAAAA, 0xbeefbeef};
	do_ioctl_ptr(HDIO_SCAN_HWIF, &scan_hwif_args);

	/* Getters of the form ioctl(..., ..., &val) */
	static const struct {
		uint32_t cmd;
		const char *str;
	} getter_cmds[] = {
		{ ARG_STR(HDIO_GET_32BIT) },
		{ ARG_STR(HDIO_GET_ACOUSTIC) },
		{ ARG_STR(HDIO_GET_ADDRESS) },
		{ ARG_STR(HDIO_GET_DMA) },
		{ ARG_STR(HDIO_GET_KEEPSETTINGS) },
		{ ARG_STR(HDIO_GET_MULTCOUNT) },
		{ ARG_STR(HDIO_GET_NOWERR) },
		{ ARG_STR(HDIO_GET_UNMASKINTR) },
		{ ARG_STR(HDIO_GET_WCACHE) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(getter_cmds); i++) {
		kernel_ulong_t val = (kernel_ulong_t) 0xfacefeeddeadbeefULL;

		rc = do_ioctl_ptr(getter_cmds[i].cmd, &val);
		printf("ioctl(-1, " XLAT_FMT ", ",
		       XLAT_SEL(getter_cmds[i].cmd, getter_cmds[i].str));
		if (rc >= 0) {
			printf("[%llu]", (unsigned long long) val);
		} else {
			printf("%p", &val);
		}
		printf(") = %s\n", errstr);
	}

	/* Setters of the form ioctl(..., ..., val) */
	static const struct {
		uint32_t cmd;
		const char *str;
	} setter_cmds[] = {
			{ ARG_STR(HDIO_SET_32BIT) },
			{ ARG_STR(HDIO_SET_ACOUSTIC) },
			{ ARG_STR(HDIO_SET_ADDRESS) },
			{ ARG_STR(HDIO_SET_DMA) },
			{ ARG_STR(HDIO_SET_KEEPSETTINGS) },
			{ ARG_STR(HDIO_SET_MULTCOUNT) },
			{ ARG_STR(HDIO_SET_NOWERR) },
			{ ARG_STR(HDIO_SET_PIO_MODE) },
			{ ARG_STR(HDIO_SET_UNMASKINTR) },
			{ ARG_STR(HDIO_SET_WCACHE) },
			{ ARG_STR(HDIO_UNREGISTER_HWIF) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(setter_cmds); i++) {
		unsigned long val = (unsigned long) 0xfacefeeddeadbeefULL;

		rc = do_ioctl(setter_cmds[i].cmd, val);
		printf("ioctl(-1, " XLAT_FMT ", %llu) = %s\n",
		       XLAT_SEL(setter_cmds[i].cmd, setter_cmds[i].str),
		       (unsigned long long) val, errstr);
	}

	/* HDIO_OBSOLETE_IDENTITY */
	do_ioctl_ptr(HDIO_OBSOLETE_IDENTITY, NULL);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(HDIO_OBSOLETE_IDENTITY), errstr);

	char obsolete_identity[142];
	rc = do_ioctl_ptr(HDIO_OBSOLETE_IDENTITY, obsolete_identity);

	printf("ioctl(-1, %s, ", XLAT_STR(HDIO_OBSOLETE_IDENTITY));

	if (rc >= 0) {
		print_quoted_memory(obsolete_identity, DEFAULT_STRLEN);
		printf("...");
	} else {
		printf("%p", obsolete_identity);
	}
	printf(") = %s\n", errstr);

	/* HDIO_GET_IDENTITY */
	do_ioctl_ptr(HDIO_GET_IDENTITY, NULL);
	printf("ioctl(-1, %s, NULL) = %s\n",
	       XLAT_STR(HDIO_GET_IDENTITY), errstr);

	char identity[512];
	rc = do_ioctl_ptr(HDIO_GET_IDENTITY, identity);

	printf("ioctl(-1, %s, ", XLAT_STR(HDIO_GET_IDENTITY));

	if (rc >= 0) {
		print_quoted_memory(identity, DEFAULT_STRLEN);
		printf("...");
	} else {
		printf("%p", identity);
	}
	printf(") = %s\n", errstr);

	/* HDIO_GET_NICE */
	kernel_ulong_t nice_val = (kernel_ulong_t) 0xfacefeedded1ffffULL;

	rc = do_ioctl_ptr(HDIO_GET_NICE, &nice_val);
	printf("ioctl(-1, %s, ", XLAT_STR(HDIO_GET_NICE));
	if (rc >= 0) {
		printf("[");
# if XLAT_RAW
		printf("%#llx", (unsigned long long) nice_val);
# else
#  if XLAT_VERBOSE
			printf("%#llx /* ", (unsigned long long) nice_val);
#  endif
		printflags(hdio_ide_nice, nice_val, "IDE_NICE_???");
#  if XLAT_VERBOSE
			printf(" */");
#  endif
# endif
		printf("]");
	} else {
		printf("%p", &nice_val);
	}
	printf(") = %s\n", errstr);

	/* HDIO_SET_NICE */
	nice_val = (unsigned long) 0xfacefeedded2ffffULL;
	do_ioctl(HDIO_SET_NICE, nice_val);
	printf("ioctl(-1, %s, ", XLAT_STR(HDIO_SET_NICE));
# if XLAT_RAW
	printf("%#llx", (unsigned long long) nice_val);
# else
#  if XLAT_VERBOSE
	printf("%#llx /* ", (unsigned long long) nice_val);
#  endif
	printflags(hdio_ide_nice, nice_val, "IDE_NICE_???");
#  if XLAT_VERBOSE
	printf(" */");
#  endif
# endif
	printf(") = %s\n", errstr);

	/* HDIO_GET_BUSSTATE */
	kernel_ulong_t busstate_value = (kernel_ulong_t) 0xfacefeedded3ffffULL;

	rc = do_ioctl_ptr(HDIO_GET_BUSSTATE, &busstate_value);
	printf("ioctl(-1, %s, ", XLAT_STR(HDIO_GET_BUSSTATE));
	if (rc >= 0) {
		printf("[");
		printxval(hdio_busstates, busstate_value, "BUSSTATE_???");
		printf("]");
	} else {
		printf("%p", &busstate_value);
	}
	printf(") = %s\n", errstr);


	/* HDIO_SET_BUSSTATE */
	busstate_value = (unsigned long) 0xfacefeedded4ffffULL;

	do_ioctl(HDIO_SET_BUSSTATE, busstate_value);
	printf("ioctl(-1, %s, ", XLAT_STR(HDIO_SET_BUSSTATE));
	printxval(hdio_busstates, busstate_value, "BUSSTATE_???");
	printf(") = %s\n", errstr);

#endif /* !MPERS_IS_m32 */

	puts("+++ exited with 0 +++");
	return 0;
}
