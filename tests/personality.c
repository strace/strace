/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2015-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <sys/personality.h>

#if XLAT_RAW
# define linux_type_str	"0"
# define good_type_str	"0x6"
# define bad_type_str	"0x1f"
# define good_flags_str	"0x7000000"
# define bad_flags_str	"0x10000"
# define good_bad_flags_str	"0x7010000"
#elif XLAT_VERBOSE
# define linux_type_str	"0 /\\* PER_LINUX \\*/"
# define good_type_str	"0x6 /\\* PER_BSD \\*/"
# define bad_type_str	"0x1f /\\* PER_\\?\\?\\? \\*/"
# define good_flags_str	\
	"0x7000000 /\\* SHORT_INODE\\|WHOLE_SECONDS\\|STICKY_TIMEOUTS \\*/"
# define bad_flags_str	"0x10000"
# define good_bad_flags_str	\
	"0x7010000 /\\* SHORT_INODE\\|WHOLE_SECONDS\\|STICKY_TIMEOUTS" \
		"\\|0x10000 \\*/"
#else
# define linux_type_str	"PER_LINUX"
# define good_type_str	"PER_BSD"
# define bad_type_str	"0x1f /\\* PER_\\?\\?\\? \\*/"
# define good_flags_str	"SHORT_INODE\\|WHOLE_SECONDS\\|STICKY_TIMEOUTS"
# define bad_flags_str	"0x10000"
# define good_bad_flags_str \
	"SHORT_INODE\\|WHOLE_SECONDS\\|STICKY_TIMEOUTS\\|0x10000"
#endif

int main(void)
{
	const unsigned int good_type = PER_BSD;

	const unsigned int bad_type = 0x1f;

	const unsigned int good_flags =
		SHORT_INODE | WHOLE_SECONDS | STICKY_TIMEOUTS;

	const unsigned int bad_flags = 0x10000;

	const unsigned int saved_pers = personality(0xffffffff);
	printf("personality\\(0xffffffff\\) = %#x \\([^)]*\\)\n", saved_pers);

	/* PER_LINUX */
	personality(PER_LINUX);
	printf("personality\\(%s\\) = %#x \\([^)]*\\)\n",
	       linux_type_str, saved_pers);

	personality(0xffffffff);
	printf("personality\\(0xffffffff\\) = 0 \\(%s\\)\n", linux_type_str);

	personality(good_flags);
	printf("personality\\(%s\\|%s\\) = 0 \\(%s\\)\n",
	       linux_type_str, good_flags_str, linux_type_str);

	personality(bad_flags);
	printf("personality\\(%s\\|%s\\)"
	       " = %#x \\(%s\\|%s\\)\n",
	       linux_type_str, bad_flags_str,
	       good_flags, linux_type_str, good_flags_str);

	personality(good_flags | bad_flags);
	printf("personality\\(%s\\|%s\\)"
	       " = %#x \\(%s\\|%s\\)\n",
	       linux_type_str, good_bad_flags_str,
	       bad_flags, linux_type_str, bad_flags_str);

	/* another valid type */
	personality(good_type);
	printf("personality\\(%s\\) = %#x \\(%s\\|%s\\)\n",
	       good_type_str, good_flags | bad_flags,
	       linux_type_str, good_bad_flags_str);

	personality(good_type | good_flags);
	printf("personality\\(%s\\|%s\\) = %#x \\(%s\\)\n",
	       good_type_str, good_flags_str, good_type, good_type_str);

	personality(good_type | bad_flags);
	printf("personality\\(%s\\|%s\\) = %#x \\(%s\\|%s\\)\n",
	       good_type_str, bad_flags_str, good_type | good_flags,
	       good_type_str, good_flags_str);

	personality(good_type | good_flags | bad_flags);
	printf("personality\\(%s\\|%s\\) = %#x \\(%s\\|%s\\)\n",
	       good_type_str, good_bad_flags_str,
	       good_type | bad_flags,
	       good_type_str, bad_flags_str);

	/* invalid type */
	personality(bad_type);
	printf("personality\\(%s\\) = %#x \\(%s\\|%s\\)\n",
	       bad_type_str, good_type | good_flags | bad_flags,
	       good_type_str, good_bad_flags_str);

	personality(bad_type | good_flags);
	printf("personality\\(%s\\|%s\\) = %#x \\(%s\\)\n",
	       bad_type_str, good_flags_str, bad_type, bad_type_str);

	personality(bad_type | bad_flags);
	printf("personality\\(%s\\|%s\\) = %#x \\(%s\\|%s\\)\n",
	       bad_type_str, bad_flags_str, bad_type | good_flags,
	       bad_type_str, good_flags_str);

	personality(bad_type | good_flags | bad_flags);
	printf("personality\\(%s\\|%s\\) = %#x \\(%s\\|%s\\)\n",
	       bad_type_str, good_bad_flags_str,
	       bad_type | bad_flags, bad_type_str, bad_flags_str);

	personality(saved_pers);
	printf("personality\\([^)]*\\) = %#x \\(%s\\|%s\\)\n",
	       bad_type | good_flags | bad_flags,
	       bad_type_str, good_bad_flags_str);

	return 0;
}
