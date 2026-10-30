/*
 * Check decoding of SO_ORIGINAL_DST and IP6T_SO_ORIGINAL_DST getsockopt
 * options.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifndef SO_ORIGINAL_DST
# define SO_ORIGINAL_DST 80
#endif
#ifndef IP6T_SO_ORIGINAL_DST
# define IP6T_SO_ORIGINAL_DST 80
#endif

#ifndef INJSTR
# define INJSTR ""
#endif

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct sockaddr_in, sin);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct sockaddr_in6, sin6);
	const char *errstr;
	socklen_t len;
	int rc;

	sin->sin_family = AF_INET;
	sin->sin_port = htons(31337);
	sin->sin_addr.s_addr = inet_addr("192.0.2.1");

	sin6->sin6_family = AF_INET6;
	sin6->sin6_port = htons(31337);
	sin6->sin6_flowinfo = 0;
	inet_pton(AF_INET6, "2001:db8::1", &sin6->sin6_addr);
	sin6->sin6_scope_id = 0;

	/* SOL_IP, a complete struct sockaddr_in */
	len = sizeof(*sin);
	rc = getsockopt(0, SOL_IP, SO_ORIGINAL_DST, sin, &len);
	errstr = sprintrc(rc);
	printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", ",
	       XLAT_ARGS(SOL_IP), XLAT_ARGS(SO_ORIGINAL_DST));
	if (INJSTR[0])
		printf("{sa_family=AF_INET, sin_port=htons(31337)"
		       ", sin_addr=inet_addr(\"192.0.2.1\")}");
	else
		printf("%p", sin);
	printf(", [%u]) = %s" INJSTR "\n", (unsigned int) sizeof(*sin), errstr);

	/* SOL_IP, optlen is too short for sa_family */
	len = 1;
	rc = getsockopt(0, SOL_IP, SO_ORIGINAL_DST, sin, &len);
	errstr = sprintrc(rc);
	printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", %p, [1]) = %s"
	       INJSTR "\n",
	       XLAT_ARGS(SOL_IP), XLAT_ARGS(SO_ORIGINAL_DST), sin, errstr);

	/* SOL_IPV6, a complete struct sockaddr_in6 */
	len = sizeof(*sin6);
	rc = getsockopt(0, SOL_IPV6, IP6T_SO_ORIGINAL_DST, sin6, &len);
	errstr = sprintrc(rc);
	printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", ",
	       XLAT_ARGS(SOL_IPV6), XLAT_ARGS(IP6T_SO_ORIGINAL_DST));
	if (INJSTR[0])
		printf("{sa_family=AF_INET6, sin6_port=htons(31337)"
		       ", sin6_flowinfo=htonl(0)"
		       ", inet_pton(AF_INET6, \"2001:db8::1\", &sin6_addr)"
		       ", sin6_scope_id=0}");
	else
		printf("%p", sin6);
	printf(", [%u]) = %s" INJSTR "\n", (unsigned int) sizeof(*sin6), errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
