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
 *
 *	$Id$
 */

#include "defs.h"

#ifdef HAVE_POLL_H
#include <poll.h>
#endif
#ifdef HAVE_SYS_POLL_H
#include <sys/poll.h>
#endif
#ifdef HAVE_STROPTS_H
#include <stropts.h>
#endif
#ifdef HAVE_SYS_CONF_H
#include <sys/conf.h>
#endif
#ifdef HAVE_SYS_STREAM_H
#include <sys/stream.h>
#endif
#ifdef HAVE_SYS_TIHDR_H
#include <sys/tihdr.h>
#endif

#if defined(HAVE_SYS_STREAM_H) || defined(linux) || defined(FREEBSD)

#ifndef HAVE_STROPTS_H
#define RS_HIPRI 1
struct strbuf {
        int     maxlen;                 /* no. of bytes in buffer */
        int     len;                    /* no. of bytes returned */
        char    *buf;                   /* pointer to data */
};
#define MORECTL 1
#define MOREDATA 2
#endif /* !HAVE_STROPTS_H */

#ifdef HAVE_SYS_TIUSER_H
#include <sys/tiuser.h>
#include <sys/sockmod.h>
#include <sys/timod.h>
#endif /* HAVE_SYS_TIUSER_H */

#ifndef FREEBSD
static struct xlat msgflags[] = {
	{ RS_HIPRI,	"RS_HIPRI"	},
	{ 0,		NULL		},
};


static void
printstrbuf(tcp, sbp, getting)
struct tcb *tcp;
struct strbuf *sbp;
int getting;
{
	if (sbp->maxlen == -1 && getting)
		tprintf("{maxlen=-1}");
	else {
		tprintf("{");
		if (getting)
			tprintf("maxlen=%d, ", sbp->maxlen);
		tprintf("len=%d, buf=", sbp->len);
		printstr(tcp, (unsigned long) sbp->buf, sbp->len);
		tprintf("}");
	}
}

static void
printstrbufarg(tcp, arg, getting)
struct tcb *tcp;
int arg;
int getting;
{
	struct strbuf buf;

	if (arg == 0)
		tprintf("NULL");
	else if (umove(tcp, arg, &buf) < 0)
		tprintf("{...}");
	else
		printstrbuf(tcp, &buf, getting);
	tprintf(", ");
}

int
sys_putmsg(tcp)
struct tcb *tcp;
{
	int i;

	if (entering(tcp)) {
		/* fd */
		tprintf("%ld, ", tcp->u_arg[0]);
		/* control and data */
		for (i = 1; i < 3; i++)
			printstrbufarg(tcp, tcp->u_arg[i], 0);
		/* flags */
		if (!printflags(msgflags, tcp->u_arg[3]))
			tprintf("0");
	}
	return 0;
}

int
sys_getmsg(tcp)
struct tcb *tcp;
{
	int i, flags;

	if (entering(tcp)) {
		/* fd */
		tprintf("%lu, ", tcp->u_arg[0]);
	} else {
		if (syserror(tcp)) {
			tprintf("%#lx, %#lx, %#lx",
				tcp->u_arg[1], tcp->u_arg[2], tcp->u_arg[3]);
			return 0;
		}
		/* control and data */
		for (i = 1; i < 3; i++)
			printstrbufarg(tcp, tcp->u_arg[i], 1);
		/* pointer to flags */
		if (tcp->u_arg[3] == 0)
			tprintf("NULL");
		else if (umove(tcp, tcp->u_arg[3], &flags) < 0)
			tprintf("[?]");
		else {
			tprintf("[");
			if (!printflags(msgflags, flags))
				tprintf("0");
			tprintf("]");
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

#ifdef HAVE_PUTPMSG

static struct xlat pmsgflags[] = {
#ifdef MSG_HIPRI
	{ MSG_HIPRI,	"MSG_HIPRI"	},
#endif
#ifdef MSG_AND
	{ MSG_ANY,	"MSG_ANY"	},
#endif
#ifdef MSG_BAND
	{ MSG_BAND,	"MSG_BAND"	},
#endif
	{ 0,		NULL		},
};

int
sys_putpmsg(tcp)
struct tcb *tcp;
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
		if (!printflags(pmsgflags, tcp->u_arg[4]))
			tprintf("0");
	}
	return 0;
}

int
sys_getpmsg(tcp)
struct tcb *tcp;
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
		printnum(tcp, tcp->u_arg[3], "%d");
		tprintf(", ");
		/* pointer to flags */
		if (tcp->u_arg[4] == 0)
			tprintf("NULL");
		else if (umove(tcp, tcp->u_arg[4], &flags) < 0)
			tprintf("[?]");
		else {
			tprintf("[");
			if (!printflags(pmsgflags, flags))
				tprintf("0");
			tprintf("]");
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

#endif /* HAVE_PUTPMSG */
#endif /* !FREEBSD */


#ifdef HAVE_SYS_POLL_H

static struct xlat pollflags[] = {
#ifdef POLLIN
	{ POLLIN,	"POLLIN"	},
	{ POLLPRI,	"POLLPRI"	},
	{ POLLOUT,	"POLLOUT"	},
#ifdef POLLRDNORM
	{ POLLRDNORM,	"POLLRDNORM"	},
#endif
#ifdef POLLWRNORM
	{ POLLWRNORM,	"POLLWRNORM"	},
#endif
#ifdef POLLRDBAND
	{ POLLRDBAND,	"POLLRDBAND"	},
#endif
#ifdef POLLWRBAND
	{ POLLWRBAND,	"POLLWRBAND"	},
#endif
	{ POLLERR,	"POLLERR"	},
	{ POLLHUP,	"POLLHUP"	},
	{ POLLNVAL,	"POLLNVAL"	},
#endif
	{ 0,		NULL		},
};

int
sys_poll(tcp)
struct tcb *tcp;
{
	struct pollfd *pollp;

	if (exiting(tcp)) {
		int i;
		int nfds = tcp->u_arg[1];

		if (nfds <= 0) {
			tprintf("%#lx, %d, %ld\n",
				tcp->u_arg[0], nfds, tcp->u_arg[2]);
			return 0;
		}
		pollp = (struct pollfd *) malloc(nfds * sizeof(*pollp));
		if (pollp == NULL) {
			fprintf(stderr, "sys_poll: no memory\n");
			tprintf("%#lx, %d, %ld",
				tcp->u_arg[0], nfds, tcp->u_arg[2]);
			return 0;
		}
		if (umoven(tcp, tcp->u_arg[0],
			   (nfds * sizeof(*pollp)), (char *) pollp) < 0) {
			tprintf("%#lx", tcp->u_arg[0]);
		}
		else {
			tprintf("[");
			for (i = 0; i < nfds; i++) {
				if (i)
					tprintf(", ");
				if (pollp[i].fd < 0) {
					tprintf("{fd=%d}", pollp[i].fd);
					continue;
				}
				tprintf("{fd=%d, events=", pollp[i].fd);
				if (!printflags(pollflags, pollp[i].events))
					tprintf("0");
				if (!syserror(tcp) && pollp[i].revents) {
					tprintf(", revents=");
					if (!printflags(pollflags,
							pollp[i].revents))
						tprintf("0");
				}
				tprintf("}");
			}
			tprintf("]");
		}
		tprintf(", %d, ", nfds);
#ifdef INFTIM
		if (tcp->u_arg[2] == INFTIM)
			tprintf("INFTIM");
		else
#endif
			tprintf("%ld", tcp->u_arg[2]);
		free(pollp);
	}
	return 0;
}

#else /* !HAVE_SYS_POLL_H */
int
sys_poll(tcp)
struct tcb *tcp;
{
    	return 0;
}
#endif

#if !defined(linux) && !defined(FREEBSD)

static struct xlat stream_flush_options[] = {
	{ FLUSHR,	"FLUSHR"	},
	{ FLUSHW,	"FLUSHW"	},
	{ FLUSHRW,	"FLUSHRW"	},
#ifdef FLUSHBAND
	{ FLUSHBAND,	"FLUSHBAND"	},
#endif
	{ 0,		NULL		},
};

static struct xlat stream_setsig_flags[] = {
	{ S_INPUT,	"S_INPUT"	},
	{ S_HIPRI,	"S_HIPRI"	},
	{ S_OUTPUT,	"S_OUTPUT"	},
	{ S_MSG,	"S_MSG"		},
#ifdef S_ERROR
	{ S_ERROR,	"S_ERROR"	},
#endif
#ifdef S_HANGUP
	{ S_HANGUP,	"S_HANGUP"	},
#endif
#ifdef S_RDNORM
	{ S_RDNORM,	"S_RDNORM"	},
#endif
#ifdef S_WRNORM
	{ S_WRNORM,	"S_WRNORM"	},
#endif
#ifdef S_RDBAND
	{ S_RDBAND,	"S_RDBAND"	},
#endif
#ifdef S_WRBAND
	{ S_WRBAND,	"S_WRBAND"	},
#endif
#ifdef S_BANDURG
	{ S_BANDURG,	"S_BANDURG"	},
#endif
	{ 0,		NULL		},
};

static struct xlat stream_read_options[] = {
	{ RNORM,	"RNORM"		},
	{ RMSGD,	"RMSGD"		},
	{ RMSGN,	"RMSGN"		},
	{ 0,		NULL		},
};

static struct xlat stream_read_flags[] = {
#ifdef RPROTDAT
	{ RPROTDAT,	"RPROTDAT"	},
#endif
#ifdef RPROTDIS
	{ RPROTDIS,	"RPROTDIS"	},
#endif
#ifdef RPROTNORM
	{ RPROTNORM,	"RPROTNORM"	},
#endif
	{ 0,		NULL		},
};

#ifndef RMODEMASK
#define RMODEMASK (~0)
#endif

#ifdef I_SWROPT
static struct xlat stream_write_flags[] = {
	{ SNDZERO,	"SNDZERO"	},
	{ SNDPIPE,	"SNDPIPE"	},
	{ 0,		NULL		},
};
#endif /* I_SWROPT */

#ifdef I_ATMARK
static struct xlat stream_atmark_options[] = {
	{ ANYMARK,	"ANYMARK"	},
	{ LASTMARK,	"LASTMARK"	},
	{ 0,		NULL		},
};
#endif /* I_ATMARK */

#ifdef TI_BIND
static struct xlat transport_user_options[] = {
	{ T_CONN_REQ,	"T_CONN_REQ"	},
	{ T_CONN_RES,	"T_CONN_RES"	},
	{ T_DISCON_REQ,	"T_DISCON_REQ"	},
	{ T_DATA_REQ,	"T_DATA_REQ"	},
	{ T_EXDATA_REQ,	"T_EXDATA_REQ"	},
	{ T_INFO_REQ,	"T_INFO_REQ"	},
	{ T_BIND_REQ,	"T_BIND_REQ"	},
	{ T_UNBIND_REQ,	"T_UNBIND_REQ"	},
	{ T_UNITDATA_REQ,"T_UNITDATA_REQ"},
	{ T_OPTMGMT_REQ,"T_OPTMGMT_REQ"	},
	{ T_ORDREL_REQ,	"T_ORDREL_REQ"	},
	{ 0,		NULL		},
};

static struct xlat transport_provider_options[] = {
	{ T_CONN_IND,	"T_CONN_IND"	},
	{ T_CONN_CON,	"T_CONN_CON"	},
	{ T_DISCON_IND,	"T_DISCON_IND"	},
	{ T_DATA_IND,	"T_DATA_IND"	},
	{ T_EXDATA_IND,	"T_EXDATA_IND"	},
	{ T_INFO_ACK,	"T_INFO_ACK"	},
	{ T_BIND_ACK,	"T_BIND_ACK"	},
	{ T_ERROR_ACK,	"T_ERROR_ACK"	},
	{ T_OK_ACK,	"T_OK_ACK"	},
	{ T_UNITDATA_IND,"T_UNITDATA_IND"},
	{ T_UDERROR_IND,"T_UDERROR_IND"	},
	{ T_OPTMGMT_ACK,"T_OPTMGMT_ACK"	},
	{ T_ORDREL_IND,	"T_ORDREL_IND"	},
	{ 0,		NULL		},
};
#endif /* TI_BIND */

static int
internal_stream_ioctl(tcp, arg)
struct tcb *tcp;
int arg;
{
	struct strioctl si;
	char *name;
	int in_and_out;
#ifdef SI_GETUDATA
	struct si_udata udata;
#endif /* SI_GETUDATA */

	if (!arg)
		return 0;
	if (umove(tcp, arg, &si) < 0) {
		if (entering(tcp))
			tprintf(", {...}");
		return 1;
	}
	if (entering(tcp)) {
		name = ioctl_lookup(si.ic_cmd);
		if (name)
			tprintf(", {ic_cmd=%s", name);
		else
			tprintf(", {ic_cmd=%#x", si.ic_cmd);
		if (si.ic_timout == INFTIM)
			tprintf(", ic_timout=INFTIM, ");
		else
			tprintf(" ic_timout=%d, ", si.ic_timout);
	}
	in_and_out = 1;
	switch (si.ic_cmd) {
#ifdef SI_GETUDATA
	case SI_GETUDATA:
		in_and_out = 0;
		break;
#endif /* SI_GETUDATA */
	}
	if (in_and_out) {
		if (entering(tcp))
			tprintf("/* in */ ");
		else
			tprintf(", /* out */ ");
	}
	if (in_and_out || entering(tcp))
		tprintf("ic_len=%d, ic_dp=", si.ic_len);
	switch (si.ic_cmd) {
#ifdef TI_BIND
	case TI_BIND:
		/* in T_BIND_REQ, out T_BIND_ACK */
		if (entering(tcp)) {
			struct T_bind_req data;

#if 0
			tprintf("struct T_bind_req ");
#endif
			if (umove(tcp, (int) si.ic_dp, &data) < 0)
				tprintf("{...}");
			else {
				tprintf("{PRIM_type=");
				printxval(transport_user_options,
					  data.PRIM_type, "T_???");
				tprintf(", ADDR_length=%ld, ADDR_offset=%ld",
					data.ADDR_length, data.ADDR_offset);
				tprintf(", CONIND_number=%ld}",
					data.CONIND_number);
			}
		}
		else {
			struct T_bind_ack data;

#if 0
			tprintf("struct T_bind_ack ");
#endif
			if (umove(tcp, (int) si.ic_dp, &data) < 0)
				tprintf("{...}");
			else {
				tprintf("[");
				tprintf("{PRIM_type=");
				printxval(transport_provider_options,
					  data.PRIM_type, "T_???");
				tprintf(", ADDR_length=%ld, ADDR_offset=%ld",
					data.ADDR_length, data.ADDR_offset);
				tprintf(", CONIND_number=%ld}",
					data.CONIND_number);
				tprintf(", ");
				printstr(tcp,
					 (int) si.ic_dp + data.ADDR_offset,
					 data.ADDR_length);
				tprintf("]");
			}
		}
		break;
#endif /* TI_BIND */
#if 0
#ifdef TI_UNBIND
	case TI_UNBIND:
		/* in T_UNBIND_REQ, out T_OK_ACK */
		break;
#endif /* TI_UNBIND */
#ifdef TI_GETINFO
	case TI_GETINFO:
		/* in T_INFO_REQ, out T_INFO_ACK */
		break;
#endif /* TI_GETINFO */
#ifdef TI_OPTMGMT
	case TI_OPTMGMT:
		/* in T_OPTMGMT_REQ, out T_OPTMGMT_ACK */
		break;
#endif /* TI_OPTMGMT */
#endif
#ifdef SI_GETUDATA
	case SI_GETUDATA:
		if (entering(tcp))
			break;
#if 0
		tprintf("struct si_udata ");
#endif
		if (umove(tcp, (int) si.ic_dp, &udata) < 0)
			tprintf("{...}");
		else {
			tprintf("{tidusize=%d, addrsize=%d, ",
				udata.tidusize, udata.addrsize);
			tprintf("optsize=%d, etsdusize=%d, ",
				udata.optsize, udata.etsdusize);
			tprintf("servtype=%d, so_state=%d, ",
				udata.servtype, udata.so_state);
			tprintf("so_options=%d", udata.so_options);
#if 0
			tprintf(", tsdusize=%d", udata.tsdusize);
#endif
			tprintf("}");
		}
		break;
#endif /* SI_GETUDATA */
	default:
		printstr(tcp, (int) si.ic_dp, si.ic_len);
		break;
	}
	if (exiting(tcp))
		tprintf("}");
	return 1;
}

int
stream_ioctl(tcp, code, arg)
struct tcb *tcp;
int code, arg;
{
#ifdef I_LIST
	int i;
#endif
	int val;
#ifdef I_FLUSHBAND
	struct bandinfo bi;
#endif
	struct strpeek sp;
	struct strfdinsert sfi;
	struct strrecvfd srf;
#ifdef I_LIST
	struct str_list sl;
#endif

	/* I_STR is a special case because the data is read & written. */
	if (code == I_STR)
		return internal_stream_ioctl(tcp, arg);
	if (entering(tcp))
		return 0;

	switch (code) {
	case I_PUSH:
	case I_LOOK:
	case I_FIND:
		/* arg is a string */
		tprintf(", ");
		printpath(tcp, arg);
		return 1;
	case I_POP:
		/* doesn't take an argument */
		return 1;
	case I_FLUSH:
		/* argument is an option */
		tprintf(", ");
		printxval(stream_flush_options, arg, "FLUSH???");
		return 1;
#ifdef I_FLUSHBAND
	case I_FLUSHBAND:
		/* argument is a pointer to a bandinfo struct */
		if (umove(tcp, arg, &bi) < 0)
			tprintf(", {...}");
		else {
			tprintf(", {bi_pri=%d, bi_flag=", bi.bi_pri);
			if (!printflags(stream_flush_options, bi.bi_flag))
				tprintf("0");
			tprintf("}");
		}
		return 1;
#endif /* I_FLUSHBAND */
	case I_SETSIG:
		/* argument is a set of flags */
		tprintf(", ");
		if (!printflags(stream_setsig_flags, arg))
			tprintf("0");
		return 1;
	case I_GETSIG:
		/* argument is a pointer to a set of flags */
		if (syserror(tcp))
			return 0;
		tprintf(", [");
		if (umove(tcp, arg, &val) < 0)
			tprintf("?");
		else if (!printflags(stream_setsig_flags, val))
			tprintf("0");
		tprintf("]");
		return 1;
	case I_PEEK:
		/* argument is a pointer to a strpeek structure */
		if (syserror(tcp) || !arg)
			return 0;
		if (umove(tcp, arg, &sp) < 0) {
			tprintf(", {...}");
			return 1;
		}
		tprintf(", {ctlbuf=");
		printstrbuf(tcp, &sp.ctlbuf, 1);
		tprintf(", databuf=");
		printstrbuf(tcp, &sp.databuf, 1);
		if (!printflags(msgflags, sp.flags))
			tprintf("0");
		return 1;
	case I_SRDOPT:
		/* argument is an option with flags */
		tprintf(", ");
		printxval(stream_read_options, arg & RMODEMASK, "R???");
		addflags(stream_read_flags, arg & ~RMODEMASK);
		return 1;
	case I_GRDOPT:
		/* argument is an pointer to an option with flags */
		if (syserror(tcp))
			return 0;
		tprintf(", [");
		if (umove(tcp, arg, &val) < 0)
			tprintf("?");
		else {
			printxval(stream_read_options,
				  arg & RMODEMASK, "R???");
			addflags(stream_read_flags, arg & ~RMODEMASK);
		}
		tprintf("]");
		return 1;
	case I_NREAD:
#ifdef I_GETBAND
	case I_GETBAND:
#endif
#ifdef I_SETCLTIME
	case I_SETCLTIME:
#endif
#ifdef I_GETCLTIME
	case I_GETCLTIME:
#endif
		/* argument is a pointer to a decimal integer */
		if (syserror(tcp))
			return 0;
		tprintf(", ");
		printnum(tcp, arg, "%d");
		return 1;
	case I_FDINSERT:
		/* argument is a pointer to a strfdinsert structure */
		if (syserror(tcp) || !arg)
			return 0;
		if (umove(tcp, arg, &sfi) < 0) {
			tprintf(", {...}");
			return 1;
		}
		tprintf(", {ctlbuf=");
		printstrbuf(tcp, &sfi.ctlbuf, 1);
		tprintf(", databuf=");
		printstrbuf(tcp, &sfi.databuf, 1);
		if (!printflags(msgflags, sfi.flags))
			tprintf("0");
		tprintf(", filedes=%d, offset=%d}", sfi.fildes, sfi.offset);
		return 1;
#ifdef I_SWROPT
	case I_SWROPT:
		/* argument is a set of flags */
		tprintf(", ");
		if (!printflags(stream_write_flags, arg))
			tprintf("0");
		return 1;
#endif /* I_SWROPT */
#ifdef I_GWROPT
	case I_GWROPT:
		/* argument is an pointer to an option with flags */
		if (syserror(tcp))
			return 0;
		tprintf(", [");
		if (umove(tcp, arg, &val) < 0)
			tprintf("?");
		else if (!printflags(stream_write_flags, arg))
			tprintf("0");
		tprintf("]");
		return 1;
#endif /* I_GWROPT */
	case I_SENDFD:
#ifdef I_CKBAND
	case I_CKBAND:
#endif
#ifdef I_CANPUT
	case I_CANPUT:
#endif
	case I_LINK:
	case I_UNLINK:
	case I_PLINK:
	case I_PUNLINK:
		/* argument is a decimal integer */
		tprintf(", %d", arg);
		return 1;
	case I_RECVFD:
		/* argument is a pointer to a strrecvfd structure */
		if (syserror(tcp) || !arg)
			return 0;
		if (umove(tcp, arg, &srf) < 0) {
			tprintf(", {...}");
			return 1;
		}
		tprintf(", {fd=%d, uid=%lu, gid=%lu}", srf.fd,
			(unsigned long) srf.uid, (unsigned long) srf.gid);
		return 1;
#ifdef I_LIST
	case I_LIST:
		if (syserror(tcp))
			return 0;
		if (arg == 0) {
			tprintf(", NULL");
			return 1;
		}
		if (umove(tcp, arg, &sl) < 0) {
			tprintf(", {...}");
			return 1;
		}
		tprintf(", {sl_nmods=%d, sl_modlist=[", sl.sl_nmods);
		for (i = 0; i < tcp->u_rval; i++) {
			if (i)
				tprintf(", ");
			printpath(tcp, (int) sl.sl_modlist[i].l_name);
		}
		tprintf("]}");
		return 1;
#endif /* I_LIST */
#ifdef I_ATMARK
	case I_ATMARK:
		tprintf(", ");
		printxval(stream_atmark_options, arg, "???MARK");
		return 1;
#endif /* I_ATMARK */
	default:
		return 0;
	}
}

#endif /* !linux && !FREEBSD */ 

#endif /* HAVE_SYS_STREAM_H || linux || FREEBSD */

