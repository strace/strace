/*
 * Test strace's -x option.
 *
 * Copyright (c) 2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <unistd.h>

#ifndef STRACE_XX
# define STRACE_XX 0
#endif

#if STRACE_XX
# define XOUT(x_, xx_) xx_
#else
# define XOUT(x_, xx_) x_
#endif

int
main(void)
{
	static const struct {
		const char *path;
		const char *out;
	} test_vecs[] = {
		{ "test", XOUT("test", "\\x74\\x65\\x73\\x74") },
		{ "\t\n\v\f\r hi~", XOUT("\\t\\n\\v\\f\\r hi~",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e") },
		{ "\t\n\v\f\r\16 hi~", XOUT(
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x0e\\x20\\x68\\x69\\x7e",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x0e\\x20\\x68\\x69\\x7e") },
		{ "\10\t\n\v\f\r hi~", XOUT(
		  "\\x08\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e",
		  "\\x08\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e") },
		{ "\t\n\v\f\r\37 hi~", XOUT(
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x1f\\x20\\x68\\x69\\x7e",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x1f\\x20\\x68\\x69\\x7e") },
		{ "\t\n\v\f\r hi~\177", XOUT(
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e\\x7f",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e\\x7f") },
		{ "\t\n\v\f\r hi~\222", XOUT(
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e\\x92",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e\\x92") },
		{ "\t\n\v\f\r hi~\377", XOUT(
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e\\xff",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e\\xff") },
	};
	static char path[] = "  ";

	const char *rc_str;

	for (size_t i = 0; i < ARRAY_SIZE(test_vecs); i++) {
		rc_str = sprintrc(chdir(test_vecs[i].path));
		printf("chdir(\"%s\") = %s\n", test_vecs[i].out, rc_str);
	}

	for (unsigned char c = 1; c < 255; c++) {
		path[1] = c;
		rc_str = sprintrc(chdir(path));

		printf("chdir(");
#if STRACE_XX
		print_quoted_hex(path, sizeof(path) - 1);
#else
		if (((c < ' ') || (c >= 0x7f)) && (c != '\t') && (c != '\n') &&
		    (c != '\v') && (c != '\f') && (c != '\r'))
			print_quoted_hex(path, sizeof(path) - 1);
		else
			print_quoted_string(path);
#endif
		printf(") = %s\n", rc_str);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
