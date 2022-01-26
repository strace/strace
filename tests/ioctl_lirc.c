/*
 * Check LIRC_* ioctl decoding
 *
 * Copyright (C) 2022 Sean Young <sean@mess.org>
 */

#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <linux/lirc.h>

static const char *errstr;

static long
do_ioctl(kernel_ulong_t cmd, unsigned int *arg)
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
		long ret = ioctl(-1, LIRC_GET_FEATURES, 0);

		printf("ioctl(-1, LIRC_GET_FEATURES, NULL) = %s%s\n",
		       sprintrc(ret),
		       ret == INJECT_RETVAL ? " (INJECTED)" : "");

		if (ret != INJECT_RETVAL)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Hasn't locked on ioctl(-1"
				   ", LIRC_GET_FEATURES, NULL) returning %d",
				   INJECT_RETVAL);
#endif

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, value);

	*value = 12000;
	do_ioctl(LIRC_SET_REC_TIMEOUT, value);
	printf("ioctl(-1, LIRC_SET_REC_TIMEOUT, [12000]) = %s\n", errstr);

	*value = 1;
	do_ioctl(LIRC_SET_WIDEBAND_RECEIVER, value);
	printf("ioctl(-1, LIRC_SET_WIDEBAND_RECEIVER, [1]) = %s\n", errstr);

	*value = 0;
	do_ioctl(LIRC_SET_MEASURE_CARRIER_MODE, value);
	printf("ioctl(-1, LIRC_SET_MEASURE_CARRIER_MODE, [0]) = %s\n", errstr);

	*value = 4294967295;
	do_ioctl(LIRC_SET_REC_TIMEOUT_REPORTS, value);
	printf("ioctl(-1, LIRC_SET_REC_TIMEOUT_REPORTS, [4294967295]) = %s\n",
	       errstr);

	*value = 33;
	do_ioctl(LIRC_SET_SEND_DUTY_CYCLE, value);
	printf("ioctl(-1, LIRC_SET_SEND_DUTY_CYCLE, [33]) = %s\n", errstr);

	*value = 38000;
	do_ioctl(LIRC_SET_SEND_CARRIER, value);
	printf("ioctl(-1, LIRC_SET_SEND_CARRIER, [38000]) = %s\n", errstr);

	*value = 20000;
	do_ioctl(LIRC_SET_REC_CARRIER, value);
	printf("ioctl(-1, LIRC_SET_REC_CARRIER, [20000]) = %s\n", errstr);

	*value = 40000;
	do_ioctl(LIRC_SET_REC_CARRIER_RANGE, value);
	printf("ioctl(-1, IPMICTL_SET_MAINTENANCE_MODE_CMD or "
	       "LIRC_SET_REC_CARRIER_RANGE, [40000]) = %s\n",
	       errstr);

	*value = 2;
	do_ioctl(LIRC_SET_SEND_MODE, value);
	printf("ioctl(-1, LIRC_SET_SEND_MODE, [LIRC_MODE_PULSE]) = %s\n",
	       errstr);

	*value = 8;
	do_ioctl(LIRC_SET_SEND_MODE, value);
	printf("ioctl(-1, LIRC_SET_SEND_MODE, [LIRC_MODE_SCANCODE]) = %s\n",
	       errstr);

	*value = 4;
	do_ioctl(LIRC_SET_REC_MODE, value);
	printf("ioctl(-1, LIRC_SET_REC_MODE, [LIRC_MODE_MODE2]) = %s\n",
	       errstr);

	*value = 16;
	do_ioctl(LIRC_SET_REC_MODE, value);
	printf("ioctl(-1, LIRC_SET_REC_MODE, [LIRC_MODE_LIRCCODE]) = %s\n",
	       errstr);

	*value = 3;
	do_ioctl(LIRC_SET_REC_MODE, value);
	printf("ioctl(-1, LIRC_SET_REC_MODE, [0x3 /* LIRC_MODE_??? */]) = %s\n",
	       errstr);

	*value = 31;
	do_ioctl(LIRC_SET_TRANSMITTER_MASK, value);
	printf("ioctl(-1, LIRC_SET_TRANSMITTER_MASK, [0x1f]) = %s\n",
	       errstr);

	/* read ioctls */

#ifdef INJECT_RETVAL

	*value = LIRC_CAN_SEND_PULSE | LIRC_CAN_SET_SEND_DUTY_CYCLE |
		LIRC_CAN_GET_REC_RESOLUTION | LIRC_CAN_USE_WIDEBAND_RECEIVER |
		0x008000000;
	do_ioctl(LIRC_GET_FEATURES, value);
	printf("ioctl(-1, LIRC_GET_FEATURES, [LIRC_CAN_SEND_PULSE"
	       "|LIRC_CAN_SET_SEND_DUTY_CYCLE|LIRC_CAN_USE_WIDEBAND_RECEIVER"
	       "|LIRC_CAN_GET_REC_RESOLUTION|0x8000000]) = %s\n",
	       errstr);

	*value = 0;
	do_ioctl(LIRC_GET_FEATURES, value);
	printf("ioctl(-1, LIRC_GET_FEATURES, [0]) = %s\n",
	       errstr);

	*value = 1;
	do_ioctl(LIRC_GET_REC_MODE, value);
	printf("ioctl(-1, LIRC_GET_REC_MODE, [LIRC_MODE_RAW]) = %s\n",
	       errstr);

	*value = 2;
	do_ioctl(LIRC_GET_SEND_MODE, value);
	printf("ioctl(-1, LIRC_GET_SEND_MODE, [LIRC_MODE_PULSE]) = %s\n",
	       errstr);

	*value = 120;
	do_ioctl(LIRC_GET_REC_RESOLUTION, value);
	printf("ioctl(-1, LIRC_GET_REC_RESOLUTION, [120]) = %s\n",
	       errstr);

	*value = 120000;
	do_ioctl(LIRC_GET_REC_TIMEOUT, value);
	printf("ioctl(-1, LIRC_GET_REC_TIMEOUT, [120000]) = %s\n",
	       errstr);

	*value = 1100;
	do_ioctl(LIRC_GET_MIN_TIMEOUT, value);
	printf("ioctl(-1, I2OVALIDATE or LIRC_GET_MIN_TIMEOUT, [1100]) = %s\n",
	       errstr);

	*value = 10100;
	do_ioctl(LIRC_GET_MAX_TIMEOUT, value);
	printf("ioctl(-1, LIRC_GET_MAX_TIMEOUT, [10100]) = %s\n",
	       errstr);

	*value = 15;
	do_ioctl(LIRC_GET_LENGTH, value);
	printf("ioctl(-1, LIRC_GET_LENGTH, [15]) = %s\n",
	       errstr);

	do_ioctl(LIRC_GET_FEATURES, value + 1);
	printf("ioctl(-1, LIRC_GET_FEATURES, %p) = %s\n",
	       value + 1, errstr);

#else

	do_ioctl(LIRC_GET_FEATURES, value);
	printf("ioctl(-1, LIRC_GET_FEATURES, %p) = %s\n",
	       value, errstr);

#endif

	do_ioctl(_IO('i', 0xff), value);
	printf("ioctl(-1, _IOC(_IOC_NONE, 0x69, 0xff, 0), %p) = %s\n",
	       value, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
