/*
 * Copyright (c) 2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifndef IP_PROTOCOL
# define IP_PROTOCOL 52
#endif

#define INJSTR " (INJECTED)"

int
main(void)
{
	static const struct strval32 opts[] = {
		{ ARG_XLAT_KNOWN(0xff, "IPPROTO_RAW") },
		{ ARG_XLAT_UNKNOWN(0xfe, "IPPROTO_???") },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, protocol);
	TAIL_ALLOC_OBJECT_CONST_ARR(unsigned int, big_protocol, 2);
	const char *errstr;
	socklen_t len;
	int rc;

	for (unsigned int i = 0; i < ARRAY_SIZE(opts); ++i) {
		*protocol = opts[i].val;

		/* optlen < 0, EINVAL */
		len = -1;
		rc = getsockopt(0, SOL_IP, IP_PROTOCOL, protocol, &len);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", %p, [-1]) = %s"
		       INJSTR "\n",
			XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_PROTOCOL),
			protocol, sprintrc(rc));


		/* optlen = 0, EINVAL */
		len = 0;
		rc = getsockopt(0, SOL_IP, IP_PROTOCOL, protocol, &len);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", %p, [0]) = %s"
		       INJSTR "\n",
			XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_PROTOCOL),
			protocol, sprintrc(rc));


		/* optlen < sizeof(int), EINVAL */
		len = sizeof(*protocol) - 1;
		rc = getsockopt(0, SOL_IP, IP_PROTOCOL, protocol, &len);
		errstr = sprintrc(rc);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", ",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_PROTOCOL));
		print_quoted_hex(protocol, len);
		printf(", [3]) = %s" INJSTR "\n", errstr);


		/* optval EFAULT */
		len = sizeof(*protocol);
		rc = getsockopt(0, SOL_IP, IP_PROTOCOL, protocol + 1, &len);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", %p, [4]) = %s"
		       INJSTR "\n",
			XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_PROTOCOL),
			protocol + 1, sprintrc(rc));


		/* classic */
		len = sizeof(*protocol);
		rc = getsockopt(0, SOL_IP, IP_PROTOCOL, protocol, &len);
		errstr = sprintrc(rc);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", [%s], [4])"
		       " = %s" INJSTR "\n",
			XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_PROTOCOL),
			opts[i].str, errstr);


		/* optval > sizeof(int), EFAULT */
		len = sizeof(*protocol) + 1;
		rc = getsockopt(0, SOL_IP, IP_PROTOCOL, protocol, &len);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", %p, [5]) = %s"
		       INJSTR "\n",
			XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_PROTOCOL),
			protocol, sprintrc(rc));


		/* optlen > sizeof(int), EINVAL */
		len = sizeof(*protocol) * 2;
		big_protocol[0] = opts[i].val;
		big_protocol[1] = opts[i].val;
		rc = getsockopt(0, SOL_IP, IP_PROTOCOL, big_protocol, &len);
		errstr = sprintrc(rc);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", ",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_PROTOCOL));
		print_quoted_hex(big_protocol, len);
		printf(", [8]) = %s" INJSTR "\n", errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
