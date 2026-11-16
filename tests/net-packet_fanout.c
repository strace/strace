/*
 * Check decoding of getsockopt and setsockopt for PACKET_FANOUT.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdint.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/if_packet.h>

#include "xmalloc.h"

#ifdef INJECT_RETVAL
# define INJSTR " (INJECTED)"
#else
# define INJSTR ""
#endif

static const char *errstr;

static int
set_fanout(const void *optval, socklen_t len)
{
	int rc = setsockopt(-1, SOL_PACKET, PACKET_FANOUT, optval, len);
	errstr = sprintrc(rc);

#ifdef INJECT_RETVAL
	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Return value [%d] does not match"
				   " expectations [%d]", rc, INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
#endif

	return rc;
}

static int
get_fanout(void *optval, socklen_t *len)
{
	int rc = getsockopt(-1, SOL_PACKET, PACKET_FANOUT, optval, len);
	errstr = sprintrc(rc);

#ifdef INJECT_RETVAL
	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Return value [%d] does not match"
				   " expectations [%d]", rc, INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
#endif

	return rc;
}

static void
test_opt(char *pfx_str, const void *optval, unsigned int len, const char *decoded)
{
	set_fanout(optval, len);
	printf("s%s, %s, %u) = %s\n", pfx_str, decoded, len, errstr);

	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, optlen);
	*optlen = len;
	int rc = get_fanout((void *) optval, optlen);
	printf("g%s, ", pfx_str);
	if (rc < 0)
		printf("%p", optval);
	else
		printf("%s", decoded);
	printf(", [%u]) = %s\n", *optlen, errstr);
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(uint32_t, val);
	char decoded[512];
	char *pfx_str = xasprintf("etsockopt(-1, " XLAT_FMT ", " XLAT_FMT,
				  XLAT_ARGS(SOL_PACKET), XLAT_ARGS(PACKET_FANOUT));

	set_fanout(NULL, 0);
	printf("s%s, NULL, 0) = %s\n", pfx_str, errstr);

	*val = 42;
	snprintf(decoded, sizeof(decoded), "[42|" XLAT_FMT "<<16]",
		 XLAT_ARGS(PACKET_FANOUT_HASH));
	test_opt(pfx_str, val, sizeof(*val), decoded);

	*val = 7 | PACKET_FANOUT_LB << 16;
	snprintf(decoded, sizeof(decoded), "[7|" XLAT_FMT "<<16]",
		 XLAT_ARGS(PACKET_FANOUT_LB));
	test_opt(pfx_str, val, sizeof(*val), decoded);

	*val = 1U | ((uint32_t) PACKET_FANOUT_FLAG_DEFRAG << 16);
#if XLAT_RAW
	snprintf(decoded, sizeof(decoded), "[1|%#x<<16]", PACKET_FANOUT_FLAG_DEFRAG);
#else
	snprintf(decoded, sizeof(decoded), "[1|(" XLAT_FMT "|" XLAT_FMT ")<<16]",
		 XLAT_ARGS(PACKET_FANOUT_HASH),
		 XLAT_ARGS(PACKET_FANOUT_FLAG_DEFRAG));
#endif
	test_opt(pfx_str, val, sizeof(*val), decoded);

	*val = 9 | 0x8 << 16;
	snprintf(decoded, sizeof(decoded),
		 "[9|" XLAT_UNKNOWN(0x8, "PACKET_FANOUT_???") "<<16]");
	test_opt(pfx_str, val, sizeof(*val), decoded);

	/* unknown flag bit */
	*val = 3 | 0x100 << 16;
#if XLAT_RAW
	snprintf(decoded, sizeof(decoded), "[3|%#x<<16]", 0x100);
#else
	snprintf(decoded, sizeof(decoded),
		 "[3|(" XLAT_FMT "|"
		 XLAT_UNKNOWN(0x100, "PACKET_FANOUT_FLAG_???") ")<<16]",
		 XLAT_ARGS(PACKET_FANOUT_HASH));
#endif
	test_opt(pfx_str, val, sizeof(*val), decoded);

	/* known flag with an unknown bit beside it */
	*val = 5U | (((uint32_t) PACKET_FANOUT_FLAG_DEFRAG | 0x100) << 16);
#if XLAT_RAW
	snprintf(decoded, sizeof(decoded), "[5|%#x<<16]",
		 PACKET_FANOUT_FLAG_DEFRAG | 0x100);
#elif XLAT_VERBOSE
	snprintf(decoded, sizeof(decoded), "[5|(" XLAT_FMT "|%#x /* %s */)<<16]",
		 XLAT_ARGS(PACKET_FANOUT_HASH),
		 PACKET_FANOUT_FLAG_DEFRAG | 0x100,
		 "PACKET_FANOUT_FLAG_DEFRAG|0x100");
#else
	snprintf(decoded, sizeof(decoded), "[5|(" XLAT_FMT "|" XLAT_FMT "|%#x)<<16]",
		 XLAT_ARGS(PACKET_FANOUT_HASH),
		 XLAT_ARGS(PACKET_FANOUT_FLAG_DEFRAG), 0x100);
#endif
	test_opt(pfx_str, val, sizeof(*val), decoded);

	TAIL_ALLOC_OBJECT_CONST_PTR(struct fanout_args, args);
	*args = (struct fanout_args) {
		.id = 8,
		.type_flags = PACKET_FANOUT_QM | PACKET_FANOUT_FLAG_ROLLOVER |
			      PACKET_FANOUT_FLAG_UNIQUEID,
		.max_num_members = 256
	};
#if XLAT_RAW
	snprintf(decoded, sizeof(decoded),
		 "{id=8, type_flags=%#x, max_num_members=256}",
		 PACKET_FANOUT_QM | PACKET_FANOUT_FLAG_ROLLOVER |
		 PACKET_FANOUT_FLAG_UNIQUEID);
#elif XLAT_VERBOSE
	snprintf(decoded, sizeof(decoded),
		 "{id=8, type_flags=" XLAT_FMT "|%#x /* %s */, max_num_members=256}",
		 XLAT_ARGS(PACKET_FANOUT_QM),
		 PACKET_FANOUT_FLAG_ROLLOVER | PACKET_FANOUT_FLAG_UNIQUEID,
		 "PACKET_FANOUT_FLAG_ROLLOVER|PACKET_FANOUT_FLAG_UNIQUEID");
#else
	snprintf(decoded, sizeof(decoded),
		 "{id=8, type_flags=" XLAT_FMT "|" XLAT_FMT "|" XLAT_FMT
		 ", max_num_members=256}",
		 XLAT_ARGS(PACKET_FANOUT_QM),
		 XLAT_ARGS(PACKET_FANOUT_FLAG_ROLLOVER),
		 XLAT_ARGS(PACKET_FANOUT_FLAG_UNIQUEID));
#endif
	test_opt(pfx_str, args, sizeof(*args), decoded);

	*args = (struct fanout_args) {
		.id = 0,
		.type_flags = PACKET_FANOUT_HASH,
		.max_num_members = 0
	};
	snprintf(decoded, sizeof(decoded),
		 "{id=0, type_flags=" XLAT_FMT ", max_num_members=0}",
		 XLAT_ARGS(PACKET_FANOUT_HASH));
	test_opt(pfx_str, args, sizeof(*args), decoded);

	/* unexpected lengths */
	set_fanout(val, sizeof(*val) - 1);
	printf("s%s, %p, %u) = %s\n",
	       pfx_str, val, (unsigned int) sizeof(*val) - 1, errstr);

	set_fanout(val + 1, sizeof(*val));
	printf("s%s, %p, %u) = %s\n",
	       pfx_str, val + 1, (unsigned int) sizeof(*val), errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
