/*
 * Copyright (c) 2023 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <limits.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#ifndef IP_LOCAL_PORT_RANGE
# define IP_LOCAL_PORT_RANGE 51
#endif

#ifndef INJSTR
# define INJSTR ""
#endif

int
main(void)
{
	static const struct strval32 opts[] = {
		{ 0,           NULL },
		{ 1,           "1.." },
		{ 12345,       "12345.." },
		{ 0xbeef,      "48879.." },
		{ 0x10000,     "..1" },
		{ 12345 << 16, "..12345" },
		{ 0x12341234U, "4660..4660" },
		{ 0x87654321U, "17185..34661" },
		{ 0xbadc0dedU, "3565..47836" },
		{ 0xbeef0000U, "..48879" },
		{ 0xbeef0001U, "1..48879" },
		{ 0xbeefbeefU, "48879..48879" },
		{ 0xbeeffaceU, NULL },
	};

	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, ports);
	TAIL_ALLOC_OBJECT_CONST_ARR(unsigned int, big_ports, 2);
	const char *errstr;
	socklen_t len;
	int rc;

	for (unsigned int i = 0; i < ARRAY_SIZE(opts); ++i) {
		*ports = opts[i].val;

		/* optlen < 0, EINVAL */
		len = -1;
		rc = setsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, ports, len);
		printf("setsockopt(0, " XLAT_FMT ", " XLAT_FMT ", %p, -1) = %s"
		       INJSTR "\n",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE),
		       ports, sprintrc(rc));

		rc = getsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, ports, &len);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", %p, [-1]) = %s"
		       INJSTR "\n",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE),
		       ports, sprintrc(rc));


		/* optlen = 0, EINVAL */
		len = 0;
		rc = setsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, ports, len);
		printf("setsockopt(0, " XLAT_FMT ", " XLAT_FMT ", \"\", 0) = %s"
		       INJSTR "\n",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE),
		       sprintrc(rc));

		rc = getsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, ports, &len);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", %p, [0]) = %s"
		       INJSTR "\n",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE),
		       ports, sprintrc(rc));


		/* optlen < sizeof(int), EINVAL */
		len = sizeof(*ports) - 1;
		rc = setsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, ports, len);
		errstr = sprintrc(rc);
		printf("setsockopt(0, " XLAT_FMT ", " XLAT_FMT ", ",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE));
		print_quoted_hex(ports, len);
		printf(", 3) = %s" INJSTR "\n", errstr);

		rc = getsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, ports, &len);
		errstr = sprintrc(rc);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", ",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE));
		if (INJSTR[0])
			print_quoted_hex(ports, len);
		else
			printf("%p",  ports);
		printf(", [3]) = %s" INJSTR "\n", errstr);


		/* optval EFAULT */
		len = sizeof(*ports);
		rc = setsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, ports + 1, len);
		printf("setsockopt(0, " XLAT_FMT ", " XLAT_FMT ", %p, 4) = %s"
		       INJSTR "\n",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE),
		       ports + 1, sprintrc(rc));

		rc = getsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, ports + 1, &len);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", %p, [4]) = %s"
		       INJSTR "\n",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE),
		       ports + 1, sprintrc(rc));


		/* classic */
		len = sizeof(*ports);
		rc = setsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, ports, len);
		printf("setsockopt(0, " XLAT_FMT ", " XLAT_FMT ", [%#x%s%s%s]"
		       ", 4) = %s" INJSTR "\n",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE),
		       *ports,
		       !XLAT_RAW && opts[i].str ? " /* " : "",
		       !XLAT_RAW && opts[i].str ? opts[i].str : "",
		       !XLAT_RAW && opts[i].str ? " */" : "",
		       sprintrc(rc));

		rc = getsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, ports, &len);
		errstr = sprintrc(rc);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", ",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE));
		if (INJSTR[0]) {
			printf("[%#x%s%s%s]",
			       *ports,
			       !XLAT_RAW && opts[i].str ? " /* " : "",
			       !XLAT_RAW && opts[i].str ? opts[i].str : "",
			       !XLAT_RAW && opts[i].str ? " */" : "");
		} else {
			printf("%p",  ports);
		}
		printf(", [4]) = %s" INJSTR "\n", errstr);


		/* optval > sizeof(int), EFAULT */
		len = sizeof(*ports) + 1;
		rc = setsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, ports, len);
		printf("setsockopt(0, " XLAT_FMT ", " XLAT_FMT ", %p, 5) = %s"
		       INJSTR "\n",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE),
		       ports, sprintrc(rc));

		rc = getsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, ports, &len);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", %p, [5]) = %s"
		       INJSTR "\n",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE),
		       ports, sprintrc(rc));


		/* optlen > sizeof(int), EINVAL */
		len = sizeof(*ports) * 2;
		big_ports[0] = opts[i].val;
		big_ports[1] = opts[i].val;
		rc = setsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, big_ports, len);
		errstr = sprintrc(rc);
		printf("setsockopt(0, " XLAT_FMT ", " XLAT_FMT ", ",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE));
		print_quoted_hex(big_ports, len);
		printf(", 8) = %s" INJSTR "\n", errstr);

		rc = getsockopt(0, SOL_IP, IP_LOCAL_PORT_RANGE, big_ports, &len);
		errstr = sprintrc(rc);
		printf("getsockopt(0, " XLAT_FMT ", " XLAT_FMT ", ",
		       XLAT_ARGS(SOL_IP), XLAT_ARGS(IP_LOCAL_PORT_RANGE));
		if (INJSTR[0])
			print_quoted_hex(big_ports, len);
		else
			printf("%p",  big_ports);
		printf(", [8]) = %s" INJSTR "\n", errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
