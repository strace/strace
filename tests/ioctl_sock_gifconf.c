/*
 * Check decoding of SIOCGIFCONF command of ioctl syscall.
 *
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <string.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#define MAX_STRLEN 1

static void
print_ifc_len(int val)
{
	if (val % (int) sizeof(struct ifreq))
		printf("%d", val);
	else
		printf("%d * sizeof(struct ifreq)",
		       val / (int) sizeof(struct ifreq));
}

static void
print_ifconf(struct ifconf *ifc, int in_len, char *in_buf, long rc)
{
	if (in_buf) {
		printf("{ifc_len=");
		print_ifc_len(in_len);

		if (in_len != ifc->ifc_len) {
			printf(" => ");
			print_ifc_len(ifc->ifc_len);
		}
	} else {
		printf("{ifc_len=");
		print_ifc_len(ifc->ifc_len);
	}

	printf(", ifc_buf=");

	if ((rc < 0) || !in_buf) {
		if (in_buf)
			printf("%p", in_buf);
		else
			printf("NULL");
	} else {
		int i;

		printf("[");
		for (i = 0; i < (ifc->ifc_len) &&
		    i < (int) (MAX_STRLEN * sizeof(struct ifreq));
		    i += sizeof(struct ifreq)) {
			struct ifreq *ifr = (struct ifreq *) (ifc->ifc_buf + i);
			struct sockaddr_in *const sa_in =
				(struct sockaddr_in *) &(ifr->ifr_addr);

			if (i)
				printf(", ");
			printf("{ifr_name=\"%s\", ifr_addr={sa_family=AF_INET, "
			       "sin_port=htons(%u), sin_addr=inet_addr(\"%s\")}"
			       "}", ifr->ifr_name, ntohs(sa_in->sin_port),
			       inet_ntoa(sa_in->sin_addr));
		}

		if ((size_t) (ifc->ifc_len - i) >= sizeof(struct ifreq))
			printf(", ...");

		printf("]");
	}

	printf("}");
}

static void
gifconf_ioctl(int fd, struct ifconf *ifc, bool ifc_valid)
{
	const char *errstr;
	int in_len;
	char *in_buf;
	long rc;

	if (ifc_valid) {
		in_len = ifc->ifc_len;
		in_buf = ifc->ifc_buf;
	}

	rc = ioctl(fd, SIOCGIFCONF, ifc);
	errstr = sprintrc(rc);

	printf("ioctl(%d, SIOCGIFCONF, ", fd);
	if (ifc_valid) {
		print_ifconf(ifc, in_len, in_buf, rc);
	} else {
		if (ifc)
			printf("%p", ifc);
		else
			printf("NULL");
	}

	printf(") = %s\n", errstr);
}

int
main(int argc, char *argv[])
{
	struct ifreq *ifr = tail_alloc(2 * sizeof(*ifr));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ifconf, ifc);

	struct sockaddr_in addr;
	int fd;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		perror_msg_and_skip("socket AF_INET");

	gifconf_ioctl(fd, NULL, false);
	gifconf_ioctl(fd, ifc + 1, false);

	ifc->ifc_len = 3141592653U;
	ifc->ifc_buf = NULL;
	gifconf_ioctl(fd, ifc, true);

	ifc->ifc_len = 0;
	ifc->ifc_buf = (char *) (ifr + 2);
	gifconf_ioctl(fd, ifc, true);

	ifc->ifc_len = 1;
	ifc->ifc_buf = (char *) (ifr + 1);
	gifconf_ioctl(fd, ifc, true);

	ifc->ifc_len = 1 * sizeof(*ifr);
	ifc->ifc_buf = (char *) (ifr + 1);
	gifconf_ioctl(fd, ifc, true);

	ifc->ifc_len = 2 * sizeof(*ifr);
	ifc->ifc_buf = (char *) (ifr + 1);
	gifconf_ioctl(fd, ifc, true);

	ifc->ifc_len = 2 * sizeof(*ifr) + 2;
	ifc->ifc_buf = (char *) ifr;
	gifconf_ioctl(fd, ifc, true);

	ifc->ifc_len = 3 * sizeof(*ifr) + 4;
	ifc->ifc_buf = (char *) ifr;
	gifconf_ioctl(fd, ifc, true);

	puts("+++ exited with 0 +++");
	return 0;
}
