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
set_importance(const void *optval, socklen_t len)
{
	int rc = setsockopt(-1, SOL_TIPC, TIPC_IMPORTANCE, optval, len);
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
get_importance(void *optval, socklen_t *len)
{
	int rc = getsockopt(-1, SOL_TIPC, TIPC_IMPORTANCE, optval, len);
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
	TAIL_ALLOC_OBJECT_CONST_PTR(int, imp);
	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, len);

	static const struct {
		int optval;
		const char *str;
	} imp_optvals[] = {
		{ ARG_STR(TIPC_LOW_IMPORTANCE) },
		{ ARG_STR(TIPC_MEDIUM_IMPORTANCE) },
		{ ARG_STR(TIPC_HIGH_IMPORTANCE) },
		{ ARG_STR(TIPC_CRITICAL_IMPORTANCE) },
	};

	char *pfx_str = xasprintf("etsockopt(-1, " XLAT_FMT ", " XLAT_FMT,
				  XLAT_ARGS(SOL_TIPC),
				  XLAT_ARGS(TIPC_IMPORTANCE));

	/* classic setsockopt */
	unsigned int i = 0;

	for (i = 0; i < ARRAY_SIZE(imp_optvals); i++) {
		*imp = imp_optvals[i].optval;
		set_importance(imp, sizeof(*imp));
		printf("s%s, [", pfx_str);
#if XLAT_RAW
		printf("%#x", imp_optvals[i].optval);
#elif XLAT_VERBOSE
		printf("%#x /* %s */", imp_optvals[i].optval, imp_optvals[i].str);
#else
		printf("%s", imp_optvals[i].str);
#endif
		printf("], %u) = %s\n", (unsigned int) sizeof(*imp), errstr);
	}

	*imp = 5;
	set_importance(imp, sizeof(*imp));
	printf("s%s, [%s], %u) = %s\n",
	       pfx_str, XLAT_UNKNOWN(0x5, "TIPC_???_IMPORTANCE"),
	       (unsigned int) sizeof(*imp), errstr);

	/* classic getsockopt */
	*len = sizeof(*imp);
	for (i = 0; i < ARRAY_SIZE(imp_optvals); i++) {
		*imp = imp_optvals[i].optval;

		int rc = get_importance(imp, len);
		printf("g%s, ", pfx_str);
		if (rc < 0)
			printf("%p", imp);
		else {
#if XLAT_RAW
			printf("[%#x]", imp_optvals[i].optval);
#elif XLAT_VERBOSE
			printf("[%#x /* %s */]", imp_optvals[i].optval, imp_optvals[i].str);
#else
			printf("[%s]", imp_optvals[i].str);
#endif
		}
		printf(", [%u]) = %s\n", *len, errstr);
	}

	*imp = 5;
	int rc = get_importance(imp, len);
	printf("g%s, ", pfx_str);
	if (rc < 0)
		printf("%p, ", imp);
	else
		printf("[%s], ", XLAT_UNKNOWN(0x5, "TIPC_???_IMPORTANCE"));
	printf("[%u]) = %s\n", *len, errstr);

	*imp = 1;

	/* setsockopt with optlen smaller than usual */
	set_importance(imp, sizeof(*imp) - 1);
	printf("s%s, %p, %u) = %s\n",
	       pfx_str, imp, (unsigned int) sizeof(*imp) - 1, errstr);

	/* getsockopt with optlen smaller than usual */
	*len = sizeof(*imp) - 1;
	get_importance(imp, len);
	printf("g%s, %p, [%u]) = %s\n",
	       pfx_str, imp, *len, errstr);

	/* setsockopt with optlen larger than usual */
	set_importance( imp, sizeof(*imp) + 1);
	printf("s%s, [%s], %u) = %s\n",
	       pfx_str, XLAT_KNOWN(0x1, "TIPC_MEDIUM_IMPORTANCE"),
	       (unsigned int) sizeof(*imp) + 1, errstr);

	/* getsockopt with optlen larger than usual */
	*len = sizeof(*imp) + 1;

	rc = get_importance(imp, len);
	printf("g%s, ", pfx_str);
	if (rc < 0)
		printf("%p", imp);
	else
		printf("[%s]",
		       XLAT_KNOWN(0x1, "TIPC_MEDIUM_IMPORTANCE"));
	printf(", [%u]) = %s\n", *len, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
