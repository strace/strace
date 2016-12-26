/*
 * Copyright (c) 2010 Andreas Schwab <schwab@linux-m68k.org>
 * Copyright (c) 2012-2013 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2014 Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2010-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include "msghdr.h"
#include <limits.h>

static int
fetch_struct_mmsghdr_or_printaddr(struct tcb *const tcp,
				  const kernel_ulong_t addr,
				  const unsigned int len, void *const mh)
{
	if ((entering(tcp) || !syserror(tcp))
	    && fetch_struct_mmsghdr(tcp, addr, mh)) {
		return 0;
	} else {
		printaddr(addr);
		return -1;
	}
}

struct print_struct_mmsghdr_config {
	const int *p_user_msg_namelen;
	unsigned int msg_len_vlen;
	unsigned int count;
	bool use_msg_len;
};

static bool
print_struct_mmsghdr(struct tcb *tcp, void *elem_buf,
		     size_t elem_size, void *data)
{
	const struct mmsghdr *const mmsg = elem_buf;
	struct print_struct_mmsghdr_config *const c = data;

	if (!c->count) {
		tprints("...");
		return false;
	}
	--c->count;

	tprints("{msg_hdr=");
	print_struct_msghdr(tcp, &mmsg->msg_hdr, c->p_user_msg_namelen,
			    c->use_msg_len ? mmsg->msg_len : (kernel_ulong_t) -1);
	if (c->msg_len_vlen) {
		tprintf(", msg_len=%u", mmsg->msg_len);
		--c->msg_len_vlen;
	}
	tprints("}");

	if (c->p_user_msg_namelen)
		++c->p_user_msg_namelen;

	return true;
}

static void
free_mmsgvec_data(void *ptr)
{
	char **pstr = ptr;
	free(*pstr);
	*pstr = 0;

	free(ptr);
}

struct mmsgvec_data {
	char *timeout;
	unsigned int count;
	int namelen[IOV_MAX];
};

static void
save_mmsgvec_namelen(struct tcb *const tcp, kernel_ulong_t addr,
		     unsigned int len, const char *const timeout)
{
	if (len > IOV_MAX)
		len = IOV_MAX;

	const size_t data_size = offsetof(struct mmsgvec_data, namelen)
				 + sizeof(int) * len;
	struct mmsgvec_data *const data = xmalloc(data_size);
	data->timeout = xstrdup(timeout);

	unsigned int i, fetched;

	for (i = 0; i < len; ++i, addr += fetched) {
		struct mmsghdr mh;

		fetched = fetch_struct_mmsghdr(tcp, addr, &mh);
		if (!fetched)
			break;
		data->namelen[i] = mh.msg_hdr.msg_namelen;
	}
	data->count = i;

	set_tcb_priv_data(tcp, data, free_mmsgvec_data);
}

static void
decode_mmsgvec(struct tcb *const tcp, const kernel_ulong_t addr,
	       const unsigned int vlen, const unsigned int msg_len_vlen,
	       const bool use_msg_len)
{
	struct mmsghdr mmsg;
	struct print_struct_mmsghdr_config c = {
		.msg_len_vlen = msg_len_vlen,
		.count = IOV_MAX,
		.use_msg_len = use_msg_len
	};
	const struct mmsgvec_data *const data = get_tcb_priv_data(tcp);

	if (data) {
		if (data->count < c.count)
			c.count = data->count;
		c.p_user_msg_namelen = data->namelen;
	}

	print_array(tcp, addr, vlen, &mmsg, sizeof_struct_mmsghdr(),
		    fetch_struct_mmsghdr_or_printaddr,
		    print_struct_mmsghdr, &c);
}

void
dumpiov_in_mmsghdr(struct tcb *const tcp, kernel_ulong_t addr)
{
	unsigned int len = tcp->u_rval;
	unsigned int i, fetched;
	struct mmsghdr mmsg;

	for (i = 0; i < len; ++i, addr += fetched) {
		fetched = fetch_struct_mmsghdr(tcp, addr, &mmsg);
		if (!fetched)
			break;
		tprintf(" = %" PRI_klu " buffers in vector %u\n",
			(kernel_ulong_t) mmsg.msg_hdr.msg_iovlen, i);
		dumpiov_upto(tcp, mmsg.msg_hdr.msg_iovlen,
			     ptr_to_kulong(mmsg.msg_hdr.msg_iov),
			     mmsg.msg_len);
	}
}

SYS_FUNC(sendmmsg)
{
	if (entering(tcp)) {
		/* sockfd */
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		if (!verbose(tcp)) {
			/* msgvec */
			printaddr(tcp->u_arg[1]);
			/* vlen */
			tprintf(", %u, ", (unsigned int) tcp->u_arg[2]);
			/* flags */
			printflags(msg_flags, tcp->u_arg[3], "MSG_???");
			return RVAL_DECODED;
		}
	} else {
		const unsigned int msg_len_vlen =
			syserror(tcp) ? 0 : tcp->u_rval;
		/* msgvec */
		temporarily_clear_syserror(tcp);
		decode_mmsgvec(tcp, tcp->u_arg[1], tcp->u_arg[2],
			       msg_len_vlen, false);
		restore_cleared_syserror(tcp);
		/* vlen */
		tprintf(", %u, ", (unsigned int) tcp->u_arg[2]);
		/* flags */
		printflags(msg_flags, tcp->u_arg[3], "MSG_???");
	}
	return 0;
}

SYS_FUNC(recvmmsg)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		if (verbose(tcp)) {
			save_mmsgvec_namelen(tcp, tcp->u_arg[1], tcp->u_arg[2],
					     sprint_timespec(tcp, tcp->u_arg[4]));
		} else {
			/* msgvec */
			printaddr(tcp->u_arg[1]);
			/* vlen */
			tprintf(", %u, ", (unsigned int) tcp->u_arg[2]);
			/* flags */
			printflags(msg_flags, tcp->u_arg[3], "MSG_???");
			tprints(", ");
			print_timespec(tcp, tcp->u_arg[4]);
		}
		return 0;
	} else {
		if (verbose(tcp)) {
			/* msgvec */
			decode_mmsgvec(tcp, tcp->u_arg[1], tcp->u_rval,
				       tcp->u_rval, true);
			/* vlen */
			tprintf(", %u, ", (unsigned int) tcp->u_arg[2]);
			/* flags */
			printflags(msg_flags, tcp->u_arg[3], "MSG_???");
			tprints(", ");
			/* timeout on entrance */
			tprints(*(const char **) get_tcb_priv_data(tcp));
		}
		if (syserror(tcp))
			return 0;
		if (tcp->u_rval == 0) {
			tcp->auxstr = "Timeout";
			return RVAL_STR;
		}
		if (!verbose(tcp) || !tcp->u_arg[4])
			return 0;
		/* timeout on exit */
		static char str[sizeof("left") + TIMESPEC_TEXT_BUFSIZE];
		snprintf(str, sizeof(str), "left %s",
			 sprint_timespec(tcp, tcp->u_arg[4]));
		tcp->auxstr = str;
		return RVAL_STR;
	}
}
