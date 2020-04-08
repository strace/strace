/*
 * Check decoding of PTP_* commands of ioctl syscall.
 *
 * Copyright (c) 2018 Harsha Sharma <harshasharmaiitr@gmail.com>
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#ifdef HAVE_STRUCT_PTP_SYS_OFFSET

# include <fcntl.h>
# include <inttypes.h>
# include <stdio.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/ioctl.h>
# include <linux/ptp_clock.h>

# ifndef PTP_CLOCK_GETCAPS2
#  define PTP_CLOCK_GETCAPS2 _IOR(PTP_CLK_MAGIC, 10, struct ptp_clock_caps)
# endif
# ifndef PTP_EXTTS_REQUEST2
#  define PTP_EXTTS_REQUEST2 _IOW(PTP_CLK_MAGIC, 11, struct ptp_extts_request)
# endif
# ifndef PTP_PEROUT_REQUEST2
#  define PTP_PEROUT_REQUEST2 _IOW(PTP_CLK_MAGIC, 12, struct ptp_perout_request)
# endif
# ifndef PTP_ENABLE_PPS2
#  define PTP_ENABLE_PPS2 _IOW(PTP_CLK_MAGIC, 13, int)
# endif
# ifndef PTP_SYS_OFFSET2
#  define PTP_SYS_OFFSET2 _IOW(PTP_CLK_MAGIC, 14, struct ptp_sys_offset)
# endif

# include "xlat.h"
# include "xlat/ptp_extts_flags.h"
# include "xlat/ptp_perout_flags.h"

static void
test_no_device(void)
{
	const char *errstr;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_clock_caps, caps);
	fill_memory(caps, sizeof(*caps));

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_sys_offset, sysoff);
	fill_memory(sysoff, sizeof(*sysoff));

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_extts_request, extts);
	fill_memory(extts, sizeof(*extts));

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ptp_perout_request, perout);
	fill_memory(perout, sizeof(*perout));

	/* PTP_CLOCK_GETCAPS */
	errstr = sprintrc(ioctl(-1, PTP_CLOCK_GETCAPS, NULL));
	printf("ioctl(-1, PTP_CLOCK_GETCAPS, NULL) = %s\n", errstr);
	errstr = sprintrc(ioctl(-1, PTP_CLOCK_GETCAPS, caps));
	printf("ioctl(-1, PTP_CLOCK_GETCAPS, %p) = %s\n", caps, errstr);

	/* PTP_SYS_OFFSET */
	errstr = sprintrc(ioctl(-1, PTP_SYS_OFFSET, NULL));
	printf("ioctl(-1, PTP_SYS_OFFSET, NULL) = %s\n", errstr);
	errstr = sprintrc(ioctl(-1, PTP_SYS_OFFSET, sysoff));
	printf("ioctl(-1, PTP_SYS_OFFSET, {n_samples=%u}) = %s\n",
	       sysoff->n_samples, errstr);

	/* PTP_ENABLE_PPS */
	errstr = sprintrc(ioctl(-1, PTP_ENABLE_PPS, 0));
	printf("ioctl(-1, PTP_ENABLE_PPS, 0) = %s\n", errstr);
	errstr = sprintrc(ioctl(-1, PTP_ENABLE_PPS, 1));
	printf("ioctl(-1, PTP_ENABLE_PPS, 1) = %s\n", errstr);

	/* PTP_EXTTS_REQUEST */
	errstr = sprintrc(ioctl(-1, PTP_EXTTS_REQUEST, NULL));
	printf("ioctl(-1, PTP_EXTTS_REQUEST, NULL) = %s\n", errstr);
	errstr = sprintrc(ioctl(-1, PTP_EXTTS_REQUEST, extts));
	printf("ioctl(-1, PTP_EXTTS_REQUEST, {index=%d, flags=", extts->index);
	printflags(ptp_extts_flags, extts->flags, "PTP_???");
	printf("}) = %s\n", errstr);

	/* PTP_PEROUT_REQUEST */
	errstr = sprintrc(ioctl(-1, PTP_PEROUT_REQUEST, NULL));
	printf("ioctl(-1, PTP_PEROUT_REQUEST, NULL) = %s\n", errstr);
	errstr = sprintrc(ioctl(-1, PTP_PEROUT_REQUEST, perout));
	printf("ioctl(-1, PTP_PEROUT_REQUEST, {start={sec=%" PRId64
	       ", nsec=%" PRIu32 "}, period={sec=%" PRId64 ", nsec=%" PRIu32 "}"
	       ", index=%d, flags=",
	       (int64_t) perout->start.sec, perout->start.nsec,
	       (int64_t)perout->period.sec, perout->period.nsec, perout->index);
	printflags(ptp_perout_flags, perout->flags, "PTP_???");
	printf("}) = %s\n", errstr);

	/* PTP_CLOCK_GETCAPS2 */
	errstr = sprintrc(ioctl(-1, PTP_CLOCK_GETCAPS2, NULL));
	printf("ioctl(-1, PTP_CLOCK_GETCAPS2, NULL) = %s\n", errstr);
	errstr = sprintrc(ioctl(-1, PTP_CLOCK_GETCAPS2, caps));
	printf("ioctl(-1, PTP_CLOCK_GETCAPS2, %p) = %s\n", caps, errstr);

	/* PTP_SYS_OFFSET2 */
	errstr = sprintrc(ioctl(-1, PTP_SYS_OFFSET2, NULL));
	printf("ioctl(-1, PTP_SYS_OFFSET2, NULL) = %s\n", errstr);
	errstr = sprintrc(ioctl(-1, PTP_SYS_OFFSET2, sysoff));
	printf("ioctl(-1, PTP_SYS_OFFSET2, {n_samples=%u}) = %s\n",
	       sysoff->n_samples, errstr);

	/* PTP_ENABLE_PPS2 */
	errstr = sprintrc(ioctl(-1, PTP_ENABLE_PPS2, 0));
	printf("ioctl(-1, PTP_ENABLE_PPS2, 0) = %s\n", errstr);
	errstr = sprintrc(ioctl(-1, PTP_ENABLE_PPS2, 1));
	printf("ioctl(-1, PTP_ENABLE_PPS2, 1) = %s\n", errstr);

	/* PTP_EXTTS_REQUEST2 */
	errstr = sprintrc(ioctl(-1, PTP_EXTTS_REQUEST2, NULL));
	printf("ioctl(-1, PTP_EXTTS_REQUEST2, NULL) = %s\n", errstr);
	errstr = sprintrc(ioctl(-1, PTP_EXTTS_REQUEST2, extts));
	printf("ioctl(-1, PTP_EXTTS_REQUEST2, {index=%d, flags=", extts->index);
	printflags(ptp_extts_flags, extts->flags, "PTP_???");
	printf("}) = %s\n", errstr);

	/* PTP_PEROUT_REQUEST2 */
	errstr = sprintrc(ioctl(-1, PTP_PEROUT_REQUEST2, NULL));
	printf("ioctl(-1, PTP_PEROUT_REQUEST2, NULL) = %s\n", errstr);
	errstr = sprintrc(ioctl(-1, PTP_PEROUT_REQUEST2, perout));
	printf("ioctl(-1, PTP_PEROUT_REQUEST2, {start={sec=%" PRId64
	       ", nsec=%" PRIu32 "}, period={sec=%" PRId64 ", nsec=%" PRIu32 "}"
	       ", index=%d, flags=",
	       (int64_t) perout->start.sec, perout->start.nsec,
	       (int64_t)perout->period.sec, perout->period.nsec, perout->index);
	printflags(ptp_perout_flags, perout->flags, "PTP_???");
	printf("}) = %s\n", errstr);

	/* unrecognized */
	ioctl(-1, _IOC(_IOC_READ, PTP_CLK_MAGIC, 0xff, 0xfe), 0);
	printf("ioctl(-1, _IOC(_IOC_READ, %#x, 0xff, 0xfe), 0) = %s\n",
	       PTP_CLK_MAGIC, errstr);

	const unsigned long arg = (unsigned long) 0xfacefeeddeadbeefULL;
	ioctl(-1, _IOC(_IOC_WRITE, PTP_CLK_MAGIC, 0xfd, 0xfc), arg);
	printf("ioctl(-1, _IOC(_IOC_WRITE, %#x, 0xfd, 0xfc), %#lx)"
	       " = %s\n", PTP_CLK_MAGIC, arg, errstr);
}

int
main(void)
{
	test_no_device();

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_STRUCT_PTP_SYS_OFFSET")

#endif /* HAVE_STRUCT_PTP_SYS_OFFSET */
