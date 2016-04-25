/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include "defs.h"

#ifdef ALPHA

static int
decode_getxxid(struct tcb *tcp, const char *what)
{
	if (entering(tcp))
		return 0;

	long rval = getrval2(tcp);
	if (rval == -1)
		return 0;
	static const char const fmt[] = "%s %ld";
	static char outstr[sizeof(fmt) + 3 * sizeof(rval)];
	snprintf(outstr, sizeof(outstr), fmt, what, rval);
	tcp->auxstr = outstr;
	return RVAL_STR;
}

SYS_FUNC(getxpid)
{
	return decode_getxxid(tcp, "ppid");
}

SYS_FUNC(getxuid)
{
	return decode_getxxid(tcp, "euid");
}

SYS_FUNC(getxgid)
{
	return decode_getxxid(tcp, "egid");
}

SYS_FUNC(osf_statfs)
{
	printpath(tcp, tcp->u_arg[0]);
	tprints(", ");
	printaddr(tcp->u_arg[1]);
	tprints(", ");
	tprintf("%lu", tcp->u_arg[2]);
	return RVAL_DECODED;
}

SYS_FUNC(osf_fstatfs)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printaddr(tcp->u_arg[1]);
	tprints(", ");
	tprintf("%lu", tcp->u_arg[2]);
	return RVAL_DECODED;
}

#endif /* ALPHA */
