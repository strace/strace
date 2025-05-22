/*
 * Copyright (c) 2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <sys/socket.h>
#include <linux/tipc.h>

#include "xmalloc.h"

static const char *errstr;

static int
set_group_req(const void *optval, socklen_t len)
{
	int rc = setsockopt(-1, SOL_TIPC, TIPC_GROUP_JOIN, optval, len);
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
get_group_req(void *optval, socklen_t *len)
{
	int rc = getsockopt(-1, SOL_TIPC, TIPC_GROUP_JOIN, optval, len);
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

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct tipc_group_req, mreq);
	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, len);

	char *pfx_str = xasprintf("etsockopt(-1, " XLAT_FMT ", " XLAT_FMT,
				  XLAT_ARGS(SOL_TIPC),
				  XLAT_ARGS(TIPC_GROUP_JOIN));

	mreq->type = 42;
	mreq->instance = 2;
	mreq->scope = 2;
	mreq->flags = 2;

	/* classic setsockopt */
	set_group_req(mreq, sizeof(*mreq));
	printf("s%s, {type=%u, instance=%u, scope=%s, flags=%s}, %u) = %s\n",
	       pfx_str, mreq->type, mreq->instance,
	       XLAT_KNOWN(0x2, "TIPC_CLUSTER_SCOPE"),
	       XLAT_KNOWN(0x2, "TIPC_GROUP_MEMBER_EVTS"),
	       (unsigned int) sizeof(*mreq), errstr);

	mreq->scope = 3;
	mreq->flags = 3;

	set_group_req(mreq, sizeof(*mreq));
	printf("s%s, {type=%u, instance=%u, scope=%s, flags=%s}, %u) = %s\n",
	       pfx_str, mreq->type, mreq->instance,
	       XLAT_KNOWN(0x3, "TIPC_NODE_SCOPE"),
	       XLAT_KNOWN(0x3, "TIPC_GROUP_LOOPBACK|TIPC_GROUP_MEMBER_EVTS"),
	       (unsigned int) sizeof(*mreq), errstr);

	mreq->scope = 4;
	mreq->flags = 4;

	set_group_req(mreq, sizeof(*mreq));
	printf("s%s, {type=%u, instance=%u, scope=%s, flags=%s}, %u) = %s\n",
	       pfx_str, mreq->type, mreq->instance,
	       XLAT_UNKNOWN(0x4, "TIPC_???_SCOPE"),
	       XLAT_UNKNOWN(0x4, "TIPC_GROUP_???"),
	       (unsigned int) sizeof(*mreq), errstr);

	/* classic getsockopt */
	*len = sizeof(*mreq);
	int rc = get_group_req(mreq, len);

	printf("g%s, ", pfx_str);
	if (rc < 0)
		printf("%p, ", mreq);
	else
		printf("{type=%u, instance=%u, scope=%s, flags=%s}, ",
		       mreq->type, mreq->instance,
		       XLAT_UNKNOWN(0x4, "TIPC_???_SCOPE"),
		       XLAT_UNKNOWN(0x4, "TIPC_GROUP_???"));
	printf("[%u]) = %s\n", *len, errstr);

	mreq->scope = 3;
	mreq->flags = 1;

	rc = get_group_req(mreq, len);
	printf("g%s, ", pfx_str);
	if (rc < 0)
		printf("%p, ", mreq);
	else
		printf("{type=%u, instance=%u, scope=%s, flags=%s}, ",
		       mreq->type, mreq->instance,
		       XLAT_KNOWN(0x3, "TIPC_NODE_SCOPE"),
		       XLAT_KNOWN(0x1, "TIPC_GROUP_LOOPBACK"));
	printf("[%u]) = %s\n", *len, errstr);

	/* setsockopt with optlen smaller than usual */
	set_group_req(mreq, sizeof(*mreq) - 1);

	printf("s%s, %p, %u) = %s\n",
	       pfx_str, mreq, (unsigned int) sizeof(*mreq) - 1, errstr);

	/* getsockopt with optlen smaller than usual */
	*len = sizeof(*mreq) - 1;
	get_group_req(mreq, len);

	printf("g%s, %p, [%u]) = %s\n",
	       pfx_str, mreq, *len, errstr);

	/* setsockopt with optlen larger than usual */
	set_group_req(mreq, sizeof(*mreq) + 1);

	printf("s%s, {type=%u, instance=%u, scope=%s, flags=%s}, %u) = %s\n",
	       pfx_str, mreq->type, mreq->instance,
	       XLAT_KNOWN(0x3, "TIPC_NODE_SCOPE"),
	       XLAT_KNOWN(0x1, "TIPC_GROUP_LOOPBACK"),
	       (unsigned int) sizeof(*mreq) + 1, errstr);

	/* getsockopt with optlen larger than usual */
	*len = sizeof(*mreq) + 1;
	rc = get_group_req(mreq, len);

	printf("g%s, ", pfx_str);
	if (rc < 0)
		printf("%p, ", mreq);
	else
		printf("{type=%u, instance=%u, scope=%s, flags=%s}, ",
		       mreq->type, mreq->instance,
		       XLAT_KNOWN(0x3, "TIPC_NODE_SCOPE"),
		       XLAT_KNOWN(0x1, "TIPC_GROUP_LOOPBACK"));
	printf("[%u]) = %s\n", *len, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
