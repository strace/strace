/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <sys/personality.h>

int main(void)
{
	const unsigned int good_type = PER_BSD;
	const char *good_type_str = "PER_BSD";

	const unsigned int bad_type = 0x1f;
	const char *bad_type_str = "0x1f /\\* PER_\\?\\?\\? \\*/";

	const unsigned int good_flags =
		SHORT_INODE | WHOLE_SECONDS | STICKY_TIMEOUTS;
	const char *good_flags_str =
		"SHORT_INODE\\|WHOLE_SECONDS\\|STICKY_TIMEOUTS";

	const unsigned int bad_flags = 0x10000;
	const char *bad_flags_str = "0x10000";

	const unsigned int saved_pers = personality(0xffffffff);
	printf("personality\\(0xffffffff\\) = %#x \\([^)]*\\)\n", saved_pers);

	/* PER_LINUX */
	personality(PER_LINUX);
	printf("personality\\(PER_LINUX\\) = %#x \\([^)]*\\)\n", saved_pers);

	personality(0xffffffff);
	puts("personality\\(0xffffffff\\) = 0 \\(PER_LINUX\\)");

	personality(good_flags);
	printf("personality\\(PER_LINUX\\|%s\\) = 0 \\(PER_LINUX\\)\n",
	       good_flags_str);

	personality(bad_flags);
	printf("personality\\(PER_LINUX\\|%s\\)"
	       " = %#x \\(PER_LINUX\\|%s\\)\n",
	       bad_flags_str, good_flags, good_flags_str);

	personality(good_flags | bad_flags);
	printf("personality\\(PER_LINUX\\|%s\\|%s\\)"
	       " = %#x \\(PER_LINUX\\|%s\\)\n",
	       good_flags_str, bad_flags_str, bad_flags, bad_flags_str);

	/* another valid type */
	personality(good_type);
	printf("personality\\(%s\\) = %#x \\(PER_LINUX\\|%s\\|%s\\)\n",
	       good_type_str, good_flags | bad_flags,
	       good_flags_str, bad_flags_str);

	personality(good_type | good_flags);
	printf("personality\\(%s\\|%s\\) = %#x \\(%s\\)\n",
	       good_type_str, good_flags_str, good_type, good_type_str);

	personality(good_type | bad_flags);
	printf("personality\\(%s\\|%s\\) = %#x \\(%s\\|%s\\)\n",
	       good_type_str, bad_flags_str, good_type | good_flags,
	       good_type_str, good_flags_str);

	personality(good_type | good_flags | bad_flags);
	printf("personality\\(%s\\|%s\\|%s\\) = %#x \\(%s\\|%s\\)\n",
	       good_type_str, good_flags_str, bad_flags_str,
	       good_type | bad_flags,
	       good_type_str, bad_flags_str);

	/* invalid type */
	personality(bad_type);
	printf("personality\\(%s\\) = %#x \\(%s\\|%s\\|%s\\)\n",
	       bad_type_str, good_type | good_flags | bad_flags,
	       good_type_str, good_flags_str, bad_flags_str);

	personality(bad_type | good_flags);
	printf("personality\\(%s\\|%s\\) = %#x \\(%s\\)\n",
	       bad_type_str, good_flags_str, bad_type, bad_type_str);

	personality(bad_type | bad_flags);
	printf("personality\\(%s\\|%s\\) = %#x \\(%s\\|%s\\)\n",
	       bad_type_str, bad_flags_str, bad_type | good_flags,
	       bad_type_str, good_flags_str);

	personality(bad_type | good_flags | bad_flags);
	printf("personality\\(%s\\|%s\\|%s\\) = %#x \\(%s\\|%s\\)\n",
	       bad_type_str, good_flags_str, bad_flags_str,
	       bad_type | bad_flags, bad_type_str, bad_flags_str);

	personality(saved_pers);
	printf("personality\\([^)]*\\) = %#x \\(%s\\|%s\\|%s\\)\n",
	       bad_type | good_flags | bad_flags,
	       bad_type_str, good_flags_str, bad_flags_str);

	return 0;
}
