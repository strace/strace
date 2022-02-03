/*
 * Check decoding of LIRC_* commands of the ioctl syscall.
 *
 * Copyright (c) 2022 Eugene Syromyatnikov <evgsyr@gmail.com>.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include "scno.h"

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <linux/ioctl.h>
#include <linux/lirc.h>

#ifndef INJECT_RETVAL
# define INJECT_RETVAL 0
#endif

#if INJECT_RETVAL
# define INJ_STR " (INJECTED)"
#else
# define INJ_STR ""
#endif

/* A hack for handling different types of _IOC() on various platforms */
#if XLAT_RAW
# define XLAT_ARGS_U(a_) (unsigned int) (a_)
#elif XLAT_VERBOSE
# define XLAT_ARGS_U(a_) (unsigned int) (a_), #a_
#else
# define XLAT_ARGS_U(a_) #a_
#endif

static const char *errstr;

static long
sys_ioctl(kernel_long_t fd, kernel_ulong_t cmd, kernel_ulong_t arg)
{
	const long rc = syscall(__NR_ioctl, fd, cmd, arg);
	errstr = sprintrc(rc);
	return rc;
}

int
main(int argc, char **argv)
{
	static const struct {
		uint32_t val;
		const char *str;
	} dirs[] = {
		{ ARG_STR(_IOC_NONE) },
		{ ARG_STR(_IOC_READ) },
		{ ARG_STR(_IOC_WRITE) },
		{ ARG_STR(_IOC_READ|_IOC_WRITE) },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, value);
	*value = 0xbeefcafe;


	/*
	 * Start of output marker.  printf is in front of ioctl() here because
	 * musl calls an ioctl before the first output to stdout, specifically,
	 * ioctl(TIOCGWINSZ) in src/stdio/__stdout_write.c:__stdout_write.
	 */
	errno = EBADF;
	printf("ioctl(-1, " XLAT_FMT ", NULL) = -1 EBADF (%m)\n",
	       XLAT_ARGS_U(LIRC_GET_FEATURES));
	fflush(NULL);
	sys_ioctl(-1, LIRC_GET_FEATURES, 0);


#if INJECT_RETVAL
	if (argc == 1)
		return 0;

	if (argc < 3)
		error_msg_and_fail("Usage: %s NUM_SKIP INJECT_RETVAL", argv[0]);

	unsigned long num_skip = strtoul(argv[1], NULL, 0);
	long inject_retval = strtol(argv[2], NULL, 0);
	bool locked = false;
	long rc;

	if (inject_retval < 0)
		error_msg_and_fail("Expected non-negative INJECT_RETVAL, "
				   "but got %ld", inject_retval);

	for (unsigned long i = 0; i < num_skip; i++) {
		rc = sys_ioctl(-1, LIRC_GET_FEATURES, 0);
		printf("ioctl(-1, " XLAT_FMT ", NULL) = %s%s\n",
		       XLAT_ARGS_U(LIRC_GET_FEATURES),
		       errstr, rc == inject_retval ? " (INJECTED)" : "");

		if (rc != inject_retval)
			continue;

		locked = true;
		break;
	}

	if (!locked) {
		error_msg_and_fail("Have not locked on ioctl(-1"
				   ", SECCOMP_IOCTL_NOTIF_RECV, NULL) "
				   "returning %lu", inject_retval);
	}
#endif /* INJECT_RETVAL */


	/* Unknown lirc ioctl */
#define C(dir_, nr_, size_) _IOC((dir_), 'i', (nr_), (size_))
#define L SIZEOF_LONG
#define __ (_IOC_NONE)
#define R_ (_IOC_READ)
#define _W (_IOC_WRITE)
#define RW (_IOC_READ|_IOC_WRITE)
	static const struct {
		unsigned int cmd;
		bool skip; /**< Skip IOCs that are decoded by strace */
		const char *name;
	} named_ioctls[] = {
		{ C(__,   0,   0), 0, "I915_PERF_IOCTL_ENABLE" },
		{ C(__,   1,   0), 0, "I915_PERF_IOCTL_DISABLE" },
		{ C(__,   2,   0), 0, "I915_PERF_IOCTL_CONFIG" },

		{ C(R_,  14,   2), 0, "IPMICTL_REGISTER_FOR_CMD" },
		{ C(R_,  15,   2), 0, "IPMICTL_UNREGISTER_FOR_CMD" },

		{ C(R_,   0,   4), 1, "LIRC_GET_FEATURES" },
		{ C(R_,   1,   4), 1, "LIRC_GET_SEND_MODE" },
		{ C(R_,   2,   4), 1, "LIRC_GET_REC_MODE" },
		{ C(R_,   7,   4), 1, "LIRC_GET_REC_RESOLUTION" },
		{ C(R_,   8,   4), 1, "I2OVALIDATE or LIRC_GET_MIN_TIMEOUT" },
		{ C(R_,   9,   4), 1, "LIRC_GET_MAX_TIMEOUT" },
		{ C(R_,  15,   4), 1, "LIRC_GET_LENGTH" },
		{ C(R_,  16,   4), 0, "IPMICTL_SET_GETS_EVENTS_CMD" },
		{ C(R_,  17,   4), 0, "IPMICTL_SET_MY_ADDRESS_CMD" },
		{ C(R_,  18,   4), 0, "IPMICTL_GET_MY_ADDRESS_CMD" },
		{ C(R_,  19,   4), 0, "IPMICTL_SET_MY_LUN_CMD" },
		{ C(R_,  20,   4), 0, "IPMICTL_GET_MY_LUN_CMD" },
		{ C(R_,  24,   4), 0, "IPMICTL_SET_MY_CHANNEL_ADDRESS_CMD" },
		{ C(R_,  25,   4), 0, "IPMICTL_GET_MY_CHANNEL_ADDRESS_CMD" },
		{ C(R_,  26,   4), 0, "IPMICTL_SET_MY_CHANNEL_LUN_CMD" },
		{ C(R_,  27,   4), 0, "IPMICTL_GET_MY_CHANNEL_LUN_CMD" },
		{ C(R_,  30,   4), 0, "IPMICTL_GET_MAINTENANCE_MODE_CMD" },
		{ C(R_,  36,   4), 1, "LIRC_GET_REC_TIMEOUT" },
		{ C(R_, 128,   4), 0, "I8K_BIOS_VERSION" },
		{ C(R_, 129,   4), 0, "I8K_MACHINE_ID" },
#if SIZEOF_LONG == 4
		{ C(R_, 130,   4), 0, "I8K_POWER_STATUS" },
		{ C(R_, 131,   4), 0, "I8K_FN_STATUS" },
		{ C(R_, 132,   4), 0, "I8K_GET_TEMP" },
#endif
		{ C(R_, 144,   4), 0, "IIO_GET_EVENT_FD_IOCTL" },

		{ C(R_,  12,   8), 0, "I2OPASSTHRU32" },
		{ C(R_,  22,   8), 0, "IPMICTL_SET_TIMING_PARMS_CMD" },
		{ C(R_,  23,   8), 0, "IPMICTL_GET_TIMING_PARMS_CMD" },
#if SIZEOF_LONG == 8
		{ C(R_, 130,   8), 0, "I8K_POWER_STATUS" },
		{ C(R_, 131,   8), 0, "I8K_FN_STATUS" },
		{ C(R_, 132,   8), 0, "I8K_GET_TEMP" },
#endif

		{ C(R_,  28,  12), 0, "IPMICTL_REGISTER_FOR_CMD_CHANS" },
		{ C(R_,  29,  12), 0, "IPMICTL_UNREGISTER_FOR_CMD_CHANS" },

#if SIZEOF_LONG == 8
		{ C(R_,  12,  16), 0, "I2OPASSTHRU" },
#endif

#if SIZEOF_LONG == 4
		{ C(R_,  13,  20), 0, "IPMICTL_SEND_COMMAND" },

		{ C(R_,  21,  28), 0, "IPMICTL_SEND_COMMAND_SETTIME" },
#endif

		{ C(R_,   0,  32), 0, "I2OGETIOPS" },

#if SIZEOF_LONG == 8
		{ C(R_,  13,  40), 0, "IPMICTL_SEND_COMMAND" },

		{ C(R_,  21,  48), 0, "IPMICTL_SEND_COMMAND_SETTIME" },
#endif

		{ C(_W,  17,   4), 1, "LIRC_SET_SEND_MODE" },
		{ C(_W,  18,   4), 1, "LIRC_SET_REC_MODE" },
		{ C(_W,  19,   4), 1, "LIRC_SET_SEND_CARRIER" },
		{ C(_W,  20,   4), 1, "LIRC_SET_REC_CARRIER" },
		{ C(_W,  21,   4), 1, "LIRC_SET_SEND_DUTY_CYCLE" },
		{ C(_W,  23,   4), 1, "LIRC_SET_TRANSMITTER_MASK" },
		{ C(_W,  24,   4), 1, "LIRC_SET_REC_TIMEOUT" },
		{ C(_W,  25,   4), 1, "LIRC_SET_REC_TIMEOUT_REPORTS" },
		{ C(_W,  29,   4), 1, "LIRC_SET_MEASURE_CARRIER_MODE" },
		{ C(_W,  31,   4), 1, "IPMICTL_SET_MAINTENANCE_MODE_CMD or "
				      "LIRC_SET_MEASURE_CARRIER_MODE" },
		{ C(_W,  35,   4), 1, "LIRC_SET_WIDEBAND_RECEIVER" },

		{ C(_W,  10,  12), 0, "I2OEVTREG" },

#if SIZEOF_LONG == 4
		{ C(RW, 133,   4), 0, "I8K_GET_SPEED" },
		{ C(RW, 134,   4), 0, "I8K_GET_FAN" },
		{ C(RW, 135,   4), 0, "I8K_SET_FAN" },
#endif
		{ C(RW, 145,   4), 0, "IIO_BUFFER_GET_FD_IOCTL" },

#if SIZEOF_LONG == 8
		{ C(RW, 133,   8), 0, "I8K_GET_SPEED" },
		{ C(RW, 134,   8), 0, "I8K_GET_FAN" },
		{ C(RW, 135,   8), 0, "I8K_SET_FAN" },
#endif

		{ C(RW,   1, L*3), 0, "I2OHRTGET" },
		{ C(RW,   2, L*3), 0, "I2OLCTGET" },

		{ C(RW,   3, 8+L*4), 0, "I2OPARMSET" },
		{ C(RW,   4, 8+L*4), 0, "I2OPARMGET" },
#if SIZEOF_LONG == 4
		{ C(RW,  11,  24), 0, "IPMICTL_RECEIVE_MSG_TRUNC" },
		{ C(RW,  12,  24), 0, "IPMICTL_RECEIVE_MSG" },
#endif

#if defined(__m68k__)
		{ C(RW,   5,  26), 0, "I2OSWDL" },
		{ C(RW,   6,  26), 0, "I2OSWUL" },
		{ C(RW,   7,  26), 0, "I2OSWDEL" },
		{ C(RW,   9,  26), 0, "I2OHTML" },
#else
		{ C(RW,   5,  8+L*5), 0, "I2OSWDL" },
		{ C(RW,   6,  8+L*5), 0, "I2OSWUL" },
		{ C(RW,   7,  8+L*5), 0, "I2OSWDEL" },
		{ C(RW,   9,  8+L*5), 0, "I2OHTML" },
#endif
#if SIZEOF_LONG == 8
		{ C(RW,  11,  48), 0, "IPMICTL_RECEIVE_MSG_TRUNC" },
		{ C(RW,  12,  48), 0, "IPMICTL_RECEIVE_MSG" },
#endif
	};
#undef C
#undef L
#undef __
#undef R_
#undef _W
#undef RW

	size_t pos = 0;

	static const char dummy[64] = "OH HAI THAR! 01234567890"
				      "01234567890123456789"
				      "01234567890123456789";

	for (size_t i = 0; i < ARRAY_SIZE(dirs); i++) {
		for (unsigned int j = 0; j <= 64; j += 2) {
			for (unsigned int k = 0; k <= 255; k++) {
				const unsigned int ioc = _IOC(dirs[i].val, 'i',
							      k, j);
#if !XLAT_RAW
				char ioc_buf[256];
				const char *ioc_str;
#endif

				if (pos < ARRAY_SIZE(named_ioctls)
				    && ioc == named_ioctls[pos].cmd) {
#if !XLAT_RAW
					ioc_str = named_ioctls[pos].name;
#endif
					if (named_ioctls[pos++].skip)
						continue;
				}
#if !XLAT_RAW
				else {
					snprintf(ioc_buf, sizeof(ioc_buf),
						 "_IOC(%s, 0x69, %#x, %#x)",
						 dirs[i].str, k, j);
					ioc_str = ioc_buf;
				}
#endif

				sys_ioctl(-1, ioc, 0);
				printf("ioctl(-1, " XLAT_KNOWN_FMT("%#x", "%s")
				       ", 0) = %s" INJ_STR "\n",
				       XLAT_SEL(ioc, ioc_str), errstr);

				sys_ioctl(-1, ioc, (uintptr_t) dummy);
				printf("ioctl(-1, " XLAT_KNOWN_FMT("%#x", "%s")
				       ", %#lx) = %s" INJ_STR "\n",
				       XLAT_SEL(ioc, ioc_str),
				       (unsigned long) (uintptr_t) dummy,
				       errstr);
			}
		}
	}


	/* LIRC_GET_FEATURES */
	static const struct strval32 features_vals[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1, "LIRC_CAN_SEND_RAW") },
		{ ARG_XLAT_KNOWN(0xa1fac0de,
				 "LIRC_CAN_SEND_PULSE"
				 "|LIRC_CAN_SEND_MODE2"
				 "|LIRC_CAN_SEND_SCANCODE"
				 "|LIRC_CAN_SEND_LIRCCODE"
				 "|LIRC_CAN_REC_PULSE"
				 "|LIRC_CAN_REC_SCANCODE"
				 "|LIRC_CAN_REC_LIRCCODE"
				 "|LIRC_CAN_SET_REC_CARRIER"
				 "|LIRC_CAN_GET_REC_RESOLUTION"
				 "|LIRC_CAN_SET_REC_CARRIER_RANGE"
				 "|0xe0c0c0") },
		{ ARG_XLAT_KNOWN(0xdeadbeef,
				 "LIRC_CAN_SEND_RAW"
				 "|LIRC_CAN_SEND_PULSE"
				 "|LIRC_CAN_SEND_MODE2"
				 "|LIRC_CAN_SEND_SCANCODE"
				 "|LIRC_CAN_SET_SEND_DUTY_CYCLE"
				 "|LIRC_CAN_SET_TRANSMITTER_MASK"
				 "|LIRC_CAN_REC_RAW"
				 "|LIRC_CAN_REC_MODE2"
				 "|LIRC_CAN_REC_SCANCODE"
				 "|LIRC_CAN_MEASURE_CARRIER"
				 "|LIRC_CAN_USE_WIDEBAND_RECEIVER"
				 "|LIRC_CAN_SET_REC_FILTER"
				 "|LIRC_CAN_SET_REC_TIMEOUT"
				 "|LIRC_CAN_SET_REC_DUTY_CYCLE_RANGE"
				 "|LIRC_CAN_SET_REC_CARRIER_RANGE"
				 "|0xa0b8e0") },
		{ ARG_XLAT_UNKNOWN(0xe0f8e0, "LIRC_CAN_???") },
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, features);

	sys_ioctl(-1, LIRC_GET_FEATURES, 0);
	printf("ioctl(-1, " XLAT_FMT ", NULL) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(LIRC_GET_FEATURES), errstr);

	sys_ioctl(-1, LIRC_GET_FEATURES, (uintptr_t) features + 1);
	printf("ioctl(-1, " XLAT_FMT ", %#lx) = %s" INJ_STR "\n",
	       XLAT_ARGS_U(LIRC_GET_FEATURES),
	       (unsigned long) (uintptr_t) features + 1, errstr);

	for (size_t i = 0; i < ARRAY_SIZE(features_vals); i++) {
		*features = features_vals[i].val;
		sys_ioctl(-1, LIRC_GET_FEATURES, (uintptr_t) features);
		printf("ioctl(-1, " XLAT_FMT ", "
#if INJECT_RETVAL
		       "[%s]"
#else
		       "%#lx"
#endif
		       ") = %s" INJ_STR "\n",
		       XLAT_ARGS_U(LIRC_GET_FEATURES),
#if INJECT_RETVAL
		       features_vals[i].str
#else
		       (unsigned long) (uintptr_t) features
#endif
		       , errstr);
	}


	/* LIRC_[SG]ET_{SEND,REC}_MODE */
	static const struct strval32 mode_cmds[] = {
		{ ARG_STR(LIRC_GET_SEND_MODE) },
		{ ARG_STR(LIRC_GET_REC_MODE) },
		{ ARG_STR(LIRC_SET_SEND_MODE) },
		{ ARG_STR(LIRC_SET_REC_MODE) },
	};
	static const struct strval32 modes[] = {
		{ ARG_XLAT_UNKNOWN(0, "LIRC_MODE_???") },
		{ ARG_XLAT_KNOWN(0x1, "LIRC_MODE_RAW") },
		{ ARG_XLAT_KNOWN(0x2, "LIRC_MODE_PULSE") },
		{ ARG_XLAT_UNKNOWN(0x3, "LIRC_MODE_???") },
		{ ARG_XLAT_KNOWN(0x4, "LIRC_MODE_MODE2") },
		{ ARG_XLAT_UNKNOWN(0x20, "LIRC_MODE_???") },
		{ ARG_XLAT_UNKNOWN(0xdeadface, "LIRC_MODE_???") },
	};
	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, mode);

	for (size_t i = 0; i < ARRAY_SIZE(mode_cmds); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(modes); j++) {
			*mode = modes[j].val;

			sys_ioctl(-1, mode_cmds[i].val, 0);
			printf("ioctl(-1, " XLAT_FMT ", NULL) = %s" INJ_STR
			       "\n",
			       XLAT_SEL(mode_cmds[i].val, mode_cmds[i].str),
			       errstr);

			sys_ioctl(-1, mode_cmds[i].val, (uintptr_t) mode + 4);
			printf("ioctl(-1, " XLAT_FMT ", %#lx) = %s" INJ_STR
			       "\n",
			       XLAT_SEL(mode_cmds[i].val, mode_cmds[i].str),
			       (unsigned long) (uintptr_t) mode + 4, errstr);

			sys_ioctl(-1, mode_cmds[i].val, (uintptr_t) mode);
			printf("ioctl(-1, " XLAT_FMT ", ",
			       XLAT_SEL(mode_cmds[i].val, mode_cmds[i].str));
			if (_IOC_DIR(mode_cmds[i].val) == _IOC_WRITE
			    || INJECT_RETVAL) {
				printf("[%s]", modes[j].str);
			} else {
				printf("%p", mode);
			}
			printf(") = %s" INJ_STR "\n", errstr);
		}
	}


	/* u32 cmds */
	static const struct strval32 u32_cmds[] = {
		{ ARG_STR(LIRC_GET_REC_RESOLUTION) },
		{ LIRC_GET_MIN_TIMEOUT, "I2OVALIDATE or LIRC_GET_MIN_TIMEOUT" },
		{ ARG_STR(LIRC_GET_MAX_TIMEOUT) },
		{ ARG_STR(LIRC_GET_LENGTH) },
		{ ARG_STR(LIRC_SET_SEND_CARRIER) },
		{ ARG_STR(LIRC_SET_REC_CARRIER) },
		{ ARG_STR(LIRC_SET_SEND_DUTY_CYCLE) },
		{ ARG_STR(LIRC_SET_REC_TIMEOUT) },
		{ LIRC_SET_REC_CARRIER_RANGE,
		  "IPMICTL_SET_MAINTENANCE_MODE_CMD or "
		  "LIRC_SET_REC_CARRIER_RANGE" },
		{ ARG_STR(LIRC_GET_REC_TIMEOUT) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(u32_cmds); i++) {
		sys_ioctl(-1, u32_cmds[i].val, 0);
		printf("ioctl(-1, " XLAT_FMT ", NULL) = %s" INJ_STR "\n",
		       XLAT_SEL(u32_cmds[i].val, u32_cmds[i].str), errstr);

		sys_ioctl(-1, u32_cmds[i].val, (uintptr_t) value + 4);
		printf("ioctl(-1, " XLAT_FMT ", %#lx) = %s" INJ_STR "\n",
		       XLAT_SEL(u32_cmds[i].val, u32_cmds[i].str),
		       (unsigned long) (uintptr_t) value + 4, errstr);

		sys_ioctl(-1, u32_cmds[i].val, (uintptr_t) value);
		printf("ioctl(-1, " XLAT_FMT ", ",
		       XLAT_SEL(u32_cmds[i].val, u32_cmds[i].str));
		if (_IOC_DIR(u32_cmds[i].val) == _IOC_WRITE || INJECT_RETVAL)
			printf("[3203386110]");
		else
			printf("%p", value);
		printf(") = %s" INJ_STR "\n", errstr);
	}


	/* x32 cmds */
	static const struct strval32 x32_cmds[] = {
		{ ARG_STR(LIRC_SET_TRANSMITTER_MASK) },
		{ ARG_STR(LIRC_SET_REC_TIMEOUT_REPORTS) },
		{ ARG_STR(LIRC_SET_MEASURE_CARRIER_MODE) },
		{ ARG_STR(LIRC_SET_WIDEBAND_RECEIVER) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(x32_cmds); i++) {
		sys_ioctl(-1, x32_cmds[i].val, 0);
		printf("ioctl(-1, " XLAT_FMT ", NULL) = %s" INJ_STR "\n",
		       XLAT_SEL(x32_cmds[i].val, x32_cmds[i].str), errstr);

		sys_ioctl(-1, x32_cmds[i].val, (uintptr_t) value + 4);
		printf("ioctl(-1, " XLAT_FMT ", %#lx) = %s" INJ_STR "\n",
		       XLAT_SEL(x32_cmds[i].val, x32_cmds[i].str),
		       (unsigned long) (uintptr_t) value + 4, errstr);

		sys_ioctl(-1, x32_cmds[i].val, (uintptr_t) value);
		printf("ioctl(-1, " XLAT_FMT ", ",
		       XLAT_SEL(x32_cmds[i].val, x32_cmds[i].str));
		if (_IOC_DIR(x32_cmds[i].val) == _IOC_WRITE || INJECT_RETVAL)
			printf("[0xbeefcafe]");
		else
			printf("%p", value);
		printf(") = %s" INJ_STR "\n", errstr);
	}


	puts("+++ exited with 0 +++");
	return 0;
}
