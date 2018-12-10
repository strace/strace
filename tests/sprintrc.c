/*
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>

enum sprintrc_fmt {
	SPRINTRC_FMT_RAW,
	SPRINTRC_FMT_GREP,
};

/**
 * Provides pointer to static string buffer with printed return code in format
 * used by strace - with errno and error message.
 *
 * @param rc  Return code.
 * @param fmt Output format. Currently, raw (used for diff matching) and grep
 *            (for extended POSIX regex-based pattern matching) formats are
 *            supported.
 * @return    Pointer to (statically allocated) buffer containing decimal
 *            representation of return code and errno/error message in case @rc
 *            is equal to -1.
 */
static inline const char *
sprintrc_ex(long rc, enum sprintrc_fmt fmt)
{
	static const char *formats[] = {
		[SPRINTRC_FMT_RAW] = "-1 %s (%m)",
		[SPRINTRC_FMT_GREP] = "-1 %s \\(%m\\)",
	};
	static char buf[4096];

	if (rc == 0)
		return "0";

	int ret = (rc == -1)
		? snprintf(buf, sizeof(buf), formats[fmt], errno2name())
		: snprintf(buf, sizeof(buf), "%ld", rc);

	if (ret < 0)
		perror_msg_and_fail("snprintf");
	if ((size_t) ret >= sizeof(buf))
		error_msg_and_fail("snprintf overflow: got %d, expected"
				   " no more than %zu", ret, sizeof(buf));

	return buf;
}

const char *
sprintrc(long rc)
{
	return sprintrc_ex(rc, SPRINTRC_FMT_RAW);
}

const char *
sprintrc_grep(long rc)
{
	return sprintrc_ex(rc, SPRINTRC_FMT_GREP);
}
