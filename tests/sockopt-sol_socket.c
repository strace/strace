/*
 * Check decoding of getsockopt and setsockopt for SOL_SOCKET level.
 *
 * Copyright (c) 2017-2021 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2022 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <sys/socket.h>
#include <sys/un.h>

#define XLAT_MACROS_ONLY
# include "xlat/sock_options.h"
#undef XLAT_MACROS_ONLY

#ifdef INJECT_RETVAL
# define INJSTR " (INJECTED)"
#else
# define INJSTR ""
#endif

static int rc;
static const char *errstr;

struct intstr {
	int val;
	const char *str;
};

static int
get_sockopt(int fd, int name, void *val, socklen_t *len)
{
	rc = getsockopt(fd, SOL_SOCKET, name, val, len);
	errstr = sprintrc(rc);
	return rc;
}

static int
set_sockopt(int fd, int name, void *val, socklen_t len)
{
	rc = setsockopt(fd, SOL_SOCKET, name, val, len);
	errstr = sprintrc(rc);
	return rc;
}

static void
print_optval(int val, const struct intstr *vecs, size_t vecs_sz)
{
	for (size_t k = 0; k < vecs_sz; k++) {
		if (vecs[k].val == val) {
			printf("[%s]", vecs[k].str);
			return;
		}
	}

	printf("[%d]", val);
}

int
main(void)
{
	static const struct intstr int_vecs[] = {
		{ ARG_STR(0) },
		{ ARG_STR(1) },
		{ ARG_STR(1234567890) },
		{ ARG_STR(-1234567890) },
	};
	static const struct intstr txrehash_vecs[] = {
		{ ARG_XLAT_KNOWN(0, "SOCK_TXREHASH_DISABLED") },
		{ ARG_XLAT_KNOWN(1, "SOCK_TXREHASH_ENABLED") },
		{ ARG_XLAT_UNKNOWN(2, "SOCK_TXREHASH_???") },
		{ ARG_XLAT_UNKNOWN(254, "SOCK_TXREHASH_???") },
		{ ARG_XLAT_KNOWN(255, "SOCK_TXREHASH_DEFAULT") },
		{ ARG_XLAT_UNKNOWN(256, "SOCK_TXREHASH_???") },
		{ ARG_XLAT_UNKNOWN(511, "SOCK_TXREHASH_???") },
		{ ARG_XLAT_UNKNOWN(-1, "SOCK_TXREHASH_???") },
	};
	static const struct {
		int val;
		const char *str;
		const struct intstr *const vecs;
		size_t vecs_sz;
		size_t optsz;
	} names[] = {
		{ ARG_STR(SO_DEBUG), .optsz = sizeof(int) },
		{ ARG_STR(SO_REUSEADDR), .optsz = sizeof(int) },
		{ ARG_STR(SO_TYPE), /* TODO */ },
		/* SO_ERROR - see so_error test */
		{ ARG_STR(SO_DONTROUTE), .optsz = sizeof(int) },
		{ ARG_STR(SO_BROADCAST), .optsz = sizeof(int) },
		{ ARG_STR(SO_SNDBUF), .optsz = sizeof(int) },
		{ ARG_STR(SO_RCVBUF), .optsz = sizeof(int) },
		{ ARG_STR(SO_KEEPALIVE), .optsz = sizeof(int) },
		{ ARG_STR(SO_OOBINLINE), .optsz = sizeof(int) },
		{ ARG_STR(SO_NO_CHECK), .optsz = sizeof(int) },
		{ ARG_STR(SO_PRIORITY), .optsz = sizeof(int) },
		/* SO_LINGER - see so_linger test */
		{ ARG_STR(SO_BSDCOMPAT), .optsz = sizeof(int) },
		{ ARG_STR(SO_REUSEPORT), .optsz = sizeof(int) },
		{ ARG_STR(SO_PASSCRED), .optsz = sizeof(int) },
		/* SO_PEERCRED - see so_peercred test */
		{ ARG_STR(SO_RCVLOWAT), .optsz = sizeof(int) },
		{ ARG_STR(SO_SNDLOWAT), .optsz = sizeof(int) },
		{ ARG_STR(SO_RCVTIMEO_OLD), /* TODO */ },
		{ ARG_STR(SO_SNDTIMEO_OLD), /* TODO */ },
		{ ARG_STR(SO_SECURITY_AUTHENTICATION) },
		{ ARG_STR(SO_SECURITY_ENCRYPTION_TRANSPORT) },
		{ ARG_STR(SO_SECURITY_ENCRYPTION_NETWORK) },
		/* TODO: SO_BINDTODEVICE */
		{ ARG_STR(SO_DETACH_FILTER), .optsz = sizeof(int) },
		{ ARG_STR(SO_PEERNAME), /* TODO */ },
		{ ARG_STR(SO_TIMESTAMP_OLD), .optsz = sizeof(int) },
		{ ARG_STR(SO_ACCEPTCONN), .optsz = sizeof(int) },
		{ ARG_STR(SO_PEERSEC), /* TODO */ },
		{ ARG_STR(SO_SNDBUFFORCE), .optsz = sizeof(int) },
		{ ARG_STR(SO_RCVBUFFORCE), .optsz = sizeof(int) },
		{ ARG_STR(SO_PASSSEC), .optsz = sizeof(int) },
		{ ARG_STR(SO_TIMESTAMPNS_OLD), .optsz = sizeof(int) },
		{ ARG_STR(SO_MARK), .optsz = sizeof(int) },
		{ ARG_STR(SO_TIMESTAMPING_OLD), .optsz = sizeof(int) },
		{ ARG_STR(SO_PROTOCOL), /* TODO */ },
		{ ARG_STR(SO_DOMAIN), /* TODO */ },
		{ ARG_STR(SO_RXQ_OVFL), .optsz = sizeof(int) },
		{ ARG_STR(SO_WIFI_STATUS), .optsz = sizeof(int) },
		{ ARG_STR(SO_PEEK_OFF), .optsz = sizeof(int) },
		{ ARG_STR(SO_NOFCS), .optsz = sizeof(int) },
		{ ARG_STR(SO_LOCK_FILTER), .optsz = sizeof(int) },
		{ ARG_STR(SO_SELECT_ERR_QUEUE), .optsz = sizeof(int) },
		{ ARG_STR(SO_BUSY_POLL), .optsz = sizeof(int) },
		{ ARG_STR(SO_MAX_PACING_RATE), /* TODO */ },
		{ ARG_STR(SO_BPF_EXTENSIONS), /* TODO */ },
		{ ARG_STR(SO_INCOMING_CPU), .optsz = sizeof(int) },
		{ ARG_STR(SO_ATTACH_BPF), /* TODO */ },
		/* SO_ATTACH_REUSEPORT_CBPF - see sock_filter-v test */
		{ ARG_STR(SO_ATTACH_REUSEPORT_EBPF), /* TODO */ },
		{ ARG_STR(SO_CNX_ADVICE), .optsz = sizeof(int) },
		{ ARG_STR(SO_MEMINFO), /* TODO */ },
		{ ARG_STR(SO_INCOMING_NAPI_ID), .optsz = sizeof(int) },
		{ ARG_STR(SO_COOKIE), /* TODO */ },
		{ ARG_STR(SO_PEERGROUPS), /* TODO */ },
		{ ARG_STR(SO_ZEROCOPY), .optsz = sizeof(int) },
		{ ARG_STR(SO_TXTIME), /* TODO */ },
		{ ARG_STR(SO_BINDTOIFINDEX), /* TODO */ },
		{ ARG_STR(SO_TIMESTAMP_NEW), .optsz = sizeof(int) },
		{ ARG_STR(SO_TIMESTAMPNS_NEW), .optsz = sizeof(int) },
		{ ARG_STR(SO_TIMESTAMPING_NEW), .optsz = sizeof(int) },
		{ ARG_STR(SO_RCVTIMEO_NEW), /* TODO */ },
		{ ARG_STR(SO_SNDTIMEO_NEW), /* TODO */ },
		{ ARG_STR(SO_DETACH_REUSEPORT_BPF), .optsz = sizeof(int) },
		{ ARG_STR(SO_PREFER_BUSY_POLL), .optsz = sizeof(int) },
		{ ARG_STR(SO_BUSY_POLL_BUDGET), .optsz = sizeof(int) },
		{ ARG_STR(SO_NETNS_COOKIE), /* TODO */ },
		{ ARG_STR(SO_BUF_LOCK), /* TODO */ },
		{ ARG_STR(SO_RESERVE_MEM), .optsz = sizeof(int) },
		{ ARG_STR(SO_TXREHASH), ARRSZ_PAIR(txrehash_vecs), sizeof(int) },
		{ ARG_STR(SO_RCVMARK), .optsz = sizeof(int) },
		{ 76, NULL },
		{ -1, NULL },
	};

	char pfx_str[256];
	char sol_str[64];
	TAIL_ALLOC_OBJECT_CONST_PTR(int, val);
	TAIL_ALLOC_OBJECT_CONST_ARR(int, bigval, 2);
	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, len);
	void *const efault = val + 1;
	int fd = socket(AF_UNIX, SOCK_RAW, 0);
	if (fd < 0)
		perror_msg_and_skip("socket AF_UNIX SOCK_RAW");

	snprintf(sol_str, sizeof(sol_str), XLAT_FMT, XLAT_ARGS(SOL_SOCKET));

	for (size_t i = 0; i < ARRAY_SIZE(names); ++i) {
		static char so_str[64];
		const struct intstr *const vecs = names[i].vecs ?: int_vecs;
		size_t vecs_sz = names[i].vecs_sz ?: ARRAY_SIZE(int_vecs);

		if (names[i].str) {
			snprintf(so_str, sizeof(so_str), XLAT_FMT,
				 XLAT_SEL(names[i].val, names[i].str));
		} else {
			snprintf(so_str, sizeof(so_str),
				 "%#x" NRAW(" /* SO_??? */"), names[i].val);
		}

		snprintf(pfx_str, sizeof(pfx_str), "etsockopt(%d, %s, %s, ",
			 fd, sol_str, so_str);

		/* getsockopt */

		for (size_t j = 0; j < vecs_sz; j++) {
			/* classic */
			*len = sizeof(*val);
			*val = vecs[j].val;
			get_sockopt(fd, names[i].val, val, len);
			printf("g%s", pfx_str);
			if (rc < 0)
				printf("%p", val);
			else
				print_optval(*val, vecs, vecs_sz);
			printf(", [%d]) = %s" INJSTR "\n", *len, errstr);

			/* optlen larger than accessible memory */
			*len = sizeof(*val) + 1;
			get_sockopt(fd, names[i].val, val, len);
			printf("g%s", pfx_str);
			if (rc < 0 || (!names[i].optsz && *len > sizeof(*val)))
				printf("%p", val);
			else
				print_optval(*val, vecs, vecs_sz);
			printf(", [%d", (int) sizeof(*val) + 1);
			if ((int) sizeof(*val) + 1 != *len)
				printf(" => %d", *len);
			printf("]) = %s" INJSTR "\n", errstr);

			/* optlen larger than necessary */
			*len = sizeof(*val) + 1;
			*bigval = vecs[j].val;
			get_sockopt(fd, names[i].val, bigval, len);
			printf("g%s", pfx_str);
			if (rc < 0) {
				printf("%p", bigval);
			} else {
				if (*len == sizeof(*val) || names[i].optsz)
					print_optval(*val, vecs, vecs_sz);
				else
					print_quoted_memory(bigval, *len);
			}
			printf(", [%d", (int) sizeof(*val) + 1);
			if ((int) sizeof(*val) + 1 != *len)
				printf(" => %d", *len);
			printf("]) = %s" INJSTR "\n", errstr);
		}

		/* zero optlen - print returned optlen */
		*len = 0;
#ifdef INJECT_RETVAL
		*val = vecs[0].val;
#endif
		get_sockopt(fd, names[i].val, NULL, len);
		printf("g%sNULL, [0", pfx_str);
		if (*len)
			printf(" => %d", *len);
		printf("]) = %s" INJSTR "\n", errstr);

		/* optlen shorter than necessary */
		*len = sizeof(*val) - 1;
		get_sockopt(fd, names[i].val, val, len);
		printf("g%s", pfx_str);
		if (rc < 0)
			printf("%p", val);
		else if (names[i].optsz)
			print_quoted_hex(val, sizeof(*val) - 1);
		else
			print_quoted_memory(val, sizeof(*val) - 1);
		printf(", [%d", (int) sizeof(*val) - 1);
		if ((int) sizeof(*val) - 1 != *len)
			printf(" => %d", *len);
		printf("]) = %s" INJSTR "\n", errstr);

		/* optval EFAULT - print address */
		*len = sizeof(*val);
		get_sockopt(fd, names[i].val, efault, len);
		printf("g%s%p, [%d]) = %s" INJSTR "\n",
		       pfx_str, efault, *len, errstr);

		/* optlen EFAULT - print address */
		get_sockopt(fd, names[i].val, val, len + 1);
		printf("g%s%p, %p) = %s" INJSTR "\n",
		       pfx_str, val, len + 1, errstr);


		/* setsockopt */

		for (size_t j = 0; j < vecs_sz; j++) {
			/* classic */
			*val = vecs[j].val;
			set_sockopt(fd, names[i].val, val, sizeof(*val));
			printf("s%s[%s], %d) = %s" INJSTR "\n",
			       pfx_str, vecs[j].str, (int) sizeof(*val), errstr);

			/* optlen larger than necessary */
			set_sockopt(fd, names[i].val, val, sizeof(*val) + 1);
			printf("s%s", pfx_str);
			if (names[i].optsz && names[i].optsz <= sizeof(*val))
				printf("[%s]", vecs[j].str);
			else
				printf("%p", val);
			printf(", %d) = %s" INJSTR "\n",
			       (int) sizeof(*val) + 1, errstr);
		}

		/* optlen < 0 - print address */
		*val = vecs[0].val;
		set_sockopt(fd, names[i].val, val, -1U);
		printf("s%s%p, -1) = %s" INJSTR "\n", pfx_str, val, errstr);

		/* optlen smaller than necessary */
		set_sockopt(fd, names[i].val, val, sizeof(*val) - 1);
		printf("s%s", pfx_str);
		if (names[i].optsz)
			printf("%p", val);
		else
			print_quoted_memory(val, sizeof(*val) - 1);
		printf(", %d) = %s" INJSTR "\n", (int) sizeof(*val) - 1, errstr);

		/* optval EFAULT - print address */
		set_sockopt(fd, names[i].val, efault, sizeof(*val));
		printf("s%s%p, %d) = %s" INJSTR "\n",
		       pfx_str, efault, (int) sizeof(*val), errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
