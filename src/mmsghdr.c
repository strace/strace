/*
 * Copyright (c) 2010 Andreas Schwab <schwab@linux-m68k.org>
 * Copyright (c) 2012-2013 Denys Vlasenko <vda.linux@googlemail.com>
 * Copyright (c) 2014 Masatake YAMATO <yamato@redhat.com>
 * Copyright (c) 2010-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "msghdr.h"
#include "xstring.h"
#include <limits.h>

static bool
fetch_struct_mmsghdr_for_print(struct tcb *const tcp,
				  const kernel_ulong_t addr,
				  const unsigned int len, void *const mh)
{
	return (entering(tcp) || !syserror(tcp)) &&
	       fetch_struct_mmsghdr(tcp, addr, mh);
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
		tprint_more_data_follows();
		return false;
	}
	--c->count;

	tprint_struct_begin();
	PRINT_FIELD_OBJ_TCB_PTR(*mmsg, msg_hdr, tcp, print_struct_msghdr,
			c->p_user_msg_namelen,
			c->use_msg_len ? mmsg->msg_len : (kernel_ulong_t) -1);
	if (c->msg_len_vlen) {
		tprint_struct_next();
		PRINT_FIELD_U(*mmsg, msg_len);
		--c->msg_len_vlen;
	}
	tprint_struct_end();

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
	int namelen[0];
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
		    fetch_struct_mmsghdr_for_print,
		    print_struct_mmsghdr, &c);
}

void
dumpiov_in_mmsghdr(struct tcb *const tcp, kernel_ulong_t addr)
{
	unsigned int len = tcp->u_rval;
	unsigned int fetched;

	for (unsigned int i = 0; i < len; ++i, addr += fetched) {
		struct mmsghdr mmsg;
		fetched = fetch_struct_mmsghdr(tcp, addr, &mmsg);
		if (!fetched)
			break;
		tprintf_string(" = %" PRI_klu " buffers in vector %u\n",
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
		tprint_arg_next();

		if (!verbose(tcp)) {
			/* msgvec */
			printaddr(tcp->u_arg[1]);
			tprint_arg_next();

			/* vlen */
			PRINT_VAL_U((unsigned int) tcp->u_arg[2]);
			tprint_arg_next();

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
		tprint_arg_next();

		/* vlen */
		PRINT_VAL_U((unsigned int) tcp->u_arg[2]);
		tprint_arg_next();

		/* flags */
		printflags(msg_flags, tcp->u_arg[3], "MSG_???");
	}
	return 0;
}

static int
do_recvmmsg(struct tcb *const tcp, const print_obj_by_addr_fn print_ts,
	    const sprint_obj_by_addr_fn sprint_ts)
{
	if (entering(tcp)) {
		/* sockfd */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();

		if (verbose(tcp)) {
			save_mmsgvec_namelen(tcp, tcp->u_arg[1], tcp->u_arg[2],
					     sprint_ts(tcp, tcp->u_arg[4]));
		} else {
			/* msgvec */
			printaddr(tcp->u_arg[1]);
			tprint_arg_next();

			/* vlen */
			PRINT_VAL_U((unsigned int) tcp->u_arg[2]);
			tprint_arg_next();

			/* flags */
			printflags(msg_flags, tcp->u_arg[3], "MSG_???");
			tprint_arg_next();

			/* timeout */
			print_ts(tcp, tcp->u_arg[4]);
		}
		return 0;
	} else {
		if (verbose(tcp)) {
			/* msgvec */
			decode_mmsgvec(tcp, tcp->u_arg[1], tcp->u_rval,
				       tcp->u_rval, true);
			tprint_arg_next();

			/* vlen */
			PRINT_VAL_U((unsigned int) tcp->u_arg[2]);
			tprint_arg_next();

			/* flags */
			printflags(msg_flags, tcp->u_arg[3], "MSG_???");
			tprint_arg_next();

			/* timeout on entrance */
			tprints_string(*(const char **) get_tcb_priv_data(tcp));
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
		xsprintf(str, "left %s", sprint_ts(tcp, tcp->u_arg[4]));
		tcp->auxstr = str;
		return RVAL_STR;
	}
}

#if HAVE_ARCH_TIME32_SYSCALLS
SYS_FUNC(recvmmsg_time32)
{
	return do_recvmmsg(tcp, print_timespec32, sprint_timespec32);
}
#endif

SYS_FUNC(recvmmsg_time64)
{
	return do_recvmmsg(tcp, print_timespec64, sprint_timespec64);
}
