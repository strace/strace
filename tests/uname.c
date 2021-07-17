/*
 * Check decoding of uname syscall.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#include <stdio.h>
#include <sys/utsname.h>
#include <unistd.h>

int main(int ac, char **av)
{
	int abbrev = ac > 1;
	TAIL_ALLOC_OBJECT_CONST_PTR(struct utsname, uname);
	int rc = syscall(__NR_uname, uname);
	printf("uname({sysname=");
	print_quoted_string(uname->sysname);
	printf(", nodename=");
	print_quoted_string(uname->nodename);
	if (abbrev) {
		printf(", ...");
	} else {
		printf(", release=");
		print_quoted_string(uname->release);
		printf(", version=");
		print_quoted_string(uname->version);
		printf(", machine=");
		print_quoted_string(uname->machine);
#ifdef HAVE_STRUCT_UTSNAME_DOMAINNAME
		printf(", domainname=");
		print_quoted_string(uname->domainname);
#endif
	}
	printf("}) = %d\n", rc);

	puts("+++ exited with 0 +++");
	return 0;
}
