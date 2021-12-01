/*
 * Test strace's -x option.
 *
 * Copyright (c) 2020-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <unistd.h>

#ifndef STRACE_X
# define STRACE_X 1
#endif

#if STRACE_X == 1
# define XOUT(_, x_chars_, x_, xx_) x_
#elif STRACE_X == 2
# define XOUT(_, x_chars_, x_, xx_) xx_
#elif STRACE_X == 3
# define XOUT(_, x_chars_, x_, xx_) x_chars_
#elif STRACE_X == 0
# define XOUT(_, x_chars_, x_, xx_) _
#endif

int
main(void)
{
	static const struct {
		const char *path;
		const char *out;
	} test_vecs[] = {
		{ "test",
		  XOUT("test", "test", "test", "\\x74\\x65\\x73\\x74") },
		{ "\t\n\v\f\r hi~", XOUT("\\t\\n\\v\\f\\r hi~",
		  "\\t\\n\\v\\f\\r hi~", "\\t\\n\\v\\f\\r hi~",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e") },
		{ "\t\n\v\f\r\16 hi~", XOUT("\\t\\n\\v\\f\\r\\16 hi~",
		  "\\t\\n\\v\\f\\r\\x0e hi~",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x0e\\x20\\x68\\x69\\x7e",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x0e\\x20\\x68\\x69\\x7e") },
		{ "\10\t\n\v\f\r hi~", XOUT("\\10\\t\\n\\v\\f\\r hi~",
		  "\\x08\\t\\n\\v\\f\\r hi~",
		  "\\x08\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e",
		  "\\x08\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e") },
		{ "\t\n\v\f\r\37 hi~", XOUT("\\t\\n\\v\\f\\r\\37 hi~",
		  "\\t\\n\\v\\f\\r\\x1f hi~",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x1f\\x20\\x68\\x69\\x7e",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x1f\\x20\\x68\\x69\\x7e") },
		{ "\t\n\v\f\r hi~\177", XOUT("\\t\\n\\v\\f\\r hi~\\177",
		  "\\t\\n\\v\\f\\r hi~\\x7f",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e\\x7f",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e\\x7f") },
		{ "\t\n\v\f\r hi~\222", XOUT("\\t\\n\\v\\f\\r hi~\\222",
		  "\\t\\n\\v\\f\\r hi~\\x92",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e\\x92",
		  "\\x09\\x0a\\x0b\\x0c\\x0d\\x20\\x68\\x69\\x7e\\x92") },
		{ "\t\n\v\f\r hi~\377", XOUT("\\t\\n\\v\\f\\r hi~\\377",
		  "\\t\\n\\v\\f\\r hi~\\xff",
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
#if STRACE_X == 2
		print_quoted_hex(path, sizeof(path) - 1);
#else
# if STRACE_X != 0
		if (((c < ' ') || (c >= 0x7f)) && (c != '\t') && (c != '\n') &&
		    (c != '\v') && (c != '\f') && (c != '\r'))
#  if STRACE_X == 3
			printf("\"%c\\x%02hhx\"", path[0], path[1]);
#  else
			print_quoted_hex(path, sizeof(path) - 1);
#  endif
		else
# endif
			print_quoted_string(path);
#endif
		printf(") = %s\n", rc_str);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
