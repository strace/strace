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
		for (long n = clk_tck - 1; n > 0 && precision < 9; n /= 10)
			++precision;
	}

	if ((clk_tck > 0) && val) {
		const unsigned int scale = pow(10, precision);
		const unsigned int frac = nearbyint((double) (val % clk_tck)
						   * scale / clk_tck);
		const uint64_t sec = val / clk_tck + frac / scale;

		if (precision)
			snprintf(str, str_size,
				 "%" PRIu64 " /* %" PRIu64 ".%0*u s */",
				 val, sec, precision, frac % scale);
		else
			snprintf(str, str_size, "%" PRIu64 " /* %" PRIu64 " s */",
				 val, sec);
	} else {
		snprintf(str, str_size, "%" PRIu64, val);
	}

	return str;
}
