/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
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

/* Who has STREAMS syscalls?
 * Linux hasn't. Solaris has (had?).
 * Just in case I miss something, retain in for Sparc...
 */
#if (defined SPARC || defined SPARC64) \
 && (defined SYS_putpmsg || defined SYS_getpmsg)

# ifdef HAVE_STROPTS_H
#  include <stropts.h>
# else
struct strbuf {
	int     maxlen;                 /* no. of bytes in buffer */
	int     len;                    /* no. of bytes returned */
	const char *buf;                /* pointer to data */
};
#  define MORECTL 1
#  define MOREDATA 2
# endif

static void
printstrbuf(struct tcb *tcp, struct strbuf *sbp, int getting)
{
	if (sbp->maxlen == -1 && getting)
		tprints("{maxlen=-1}");
	else {
		tprints("{");
		if (getting)
			tprintf("maxlen=%d, ", sbp->maxlen);
		tprintf("len=%d, buf=", sbp->len);
		printstr(tcp, (unsigned long) sbp->buf, sbp->len);
		tprints("}");
	}
}

static void
printstrbufarg(struct tcb *tcp, long arg, int getting)
{
	struct strbuf buf;

	if (arg == 0)
		tprints("NULL");
	else if (umove(tcp, arg, &buf) < 0)
		tprints("{...}");
	else
		printstrbuf(tcp, &buf, getting);
	tprints(", ");
}

# include "xlat/pmsgflags.h"
# ifdef SYS_putpmsg
SYS_FUNC(putpmsg)
{
	int i;

	if (entering(tcp)) {
		/* fd */
		tprintf("%ld, ", tcp->u_arg[0]);
		/* control and data */
		for (i = 1; i < 3; i++)
			printstrbufarg(tcp, tcp->u_arg[i], 0);
		/* band */
		tprintf("%ld, ", tcp->u_arg[3]);
		/* flags */
		printflags(pmsgflags, tcp->u_arg[4], "MSG_???");
	}
	return 0;
}
# endif
# ifdef SYS_getpmsg
SYS_FUNC(getpmsg)
{
	int i, flags;

	if (entering(tcp)) {
		/* fd */
		tprintf("%lu, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp)) {
			tprintf("%#lx, %#lx, %#lx, %#lx", tcp->u_arg[1],
				tcp->u_arg[2], tcp->u_arg[3], tcp->u_arg[4]);
			return 0;
		}
		/* control and data */
		for (i = 1; i < 3; i++)
			printstrbufarg(tcp, tcp->u_arg[i], 1);
		/* pointer to band */
		printnum_int(tcp, tcp->u_arg[3], "%d");
		tprints(", ");
		/* pointer to flags */
		if (tcp->u_arg[4] == 0)
			tprints("NULL");
		else if (umove(tcp, tcp->u_arg[4], &flags) < 0)
			tprints("[?]");
		else {
			tprints("[");
			printflags(pmsgflags, flags, "MSG_???");
			tprints("]");
		}
		/* decode return value */
		switch (tcp->u_rval) {
		case MORECTL:
			tcp->auxstr = "MORECTL";
			break;
		case MORECTL|MOREDATA:
			tcp->auxstr = "MORECTL|MOREDATA";
			break;
		case MOREDATA:
			tcp->auxstr = "MORECTL";
			break;
		default:
			tcp->auxstr = NULL;
			break;
		}
	}
	return RVAL_HEX | RVAL_STR;
}
# endif
#endif /* STREAMS syscalls support */
