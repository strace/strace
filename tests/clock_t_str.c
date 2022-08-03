/*
 * Format clock_t-typed values.
 * Copyright (c) 2018-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

const char *
clock_t_str(uint64_t val, char *str, size_t str_size)
{
	static long clk_tck;
	static int precision;

	if (!clk_tck) {
		clk_tck = sysconf(_SC_CLK_TCK);
		precision = clk_tck > 1 ? MIN((int) ceil(log10(clk_tck - 1)), 9)
					: 0;
	}

	if ((clk_tck > 0) && val) {
		snprintf(str, str_size, "%" PRIu64 " /* %" PRIu64 ".%0*u s */",
			 val, val / clk_tck, precision,
			 (unsigned) round(((double) (val % clk_tck) / clk_tck)
					  * pow(10, precision)));
	} else {
		snprintf(str, str_size, "%" PRIu64, val);
	}

	return str;
}
