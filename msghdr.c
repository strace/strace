/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2000 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <arpa/inet.h>
#include <netinet/in.h>

#include "xlat/msg_flags.h"
#include "xlat/scmvals.h"
#include "xlat/ip_cmsg_types.h"

#ifndef current_wordsize
struct cmsghdr32 {
	uint32_t cmsg_len;
	int cmsg_level;
	int cmsg_type;
};
#endif

typedef union {
	char *ptr;
	struct cmsghdr *cmsg;
#ifndef current_wordsize
	struct cmsghdr32 *cmsg32;
#endif
} union_cmsghdr;

static void
print_scm_rights(struct tcb *tcp, const void *cmsg_data,
		 const unsigned int data_len)
{
	const int *fds = cmsg_data;
	const unsigned int nfds = data_len / sizeof(*fds);
	unsigned int i;

	tprints("[");

	for (i = 0; i < nfds; ++i) {
		if (i)
			tprints(", ");
		if (abbrev(tcp) && i >= max_strlen) {
			tprints("...");
			break;
		}
		printfd(tcp, fds[i]);
	}

	tprints("]");
}

static void
print_scm_creds(struct tcb *tcp, const void *cmsg_data,
		const unsigned int data_len)
{
	const struct ucred *uc = cmsg_data;

	tprintf("{pid=%u, uid=%u, gid=%u}",
		(unsigned) uc->pid, (unsigned) uc->uid, (unsigned) uc->gid);
}

static void
print_scm_security(struct tcb *tcp, const void *cmsg_data,
		   const unsigned int data_len)
{
	print_quoted_string(cmsg_data, data_len, 0);
}

static void
print_cmsg_ip_pktinfo(struct tcb *tcp, const void *cmsg_data,
		      const unsigned int data_len)
{
	const struct in_pktinfo *info = cmsg_data;

	tprints("{ipi_ifindex=");
	print_ifindex(info->ipi_ifindex);
	tprintf(", ipi_spec_dst=inet_addr(\"%s\")",
		inet_ntoa(info->ipi_spec_dst));
	tprintf(", ipi_addr=inet_addr(\"%s\")}",
		inet_ntoa(info->ipi_addr));
}

static void
print_cmsg_uint(struct tcb *tcp, const void *cmsg_data,
		const unsigned int data_len)
{
	const unsigned int *p = cmsg_data;

	tprintf("[%u]", *p);
}

static void
print_cmsg_uint8_t(struct tcb *tcp, const void *cmsg_data,
		   const unsigned int data_len)
{
	const uint8_t *p = cmsg_data;

	tprintf("[%#x]", *p);
}

static void
print_cmsg_ip_opts(struct tcb *tcp, const void *cmsg_data,
		   const unsigned int data_len)
{
	const unsigned char *opts = cmsg_data;
	unsigned int i;

	tprints("[");
	for (i = 0; i < data_len; ++i) {
		if (i)
			tprints(", ");
		if (abbrev(tcp) && i >= max_strlen) {
			tprints("...");
			break;
		}
		tprintf("0x%02x", opts[i]);
	}
	tprints("]");
}

struct sock_ee {
	uint32_t ee_errno;
	uint8_t  ee_origin;
	uint8_t  ee_type;
	uint8_t  ee_code;
	uint8_t  ee_pad;
	uint32_t ee_info;
	uint32_t ee_data;
	struct sockaddr_in offender;
};

static void
print_cmsg_ip_recverr(struct tcb *tcp, const void *cmsg_data,
		      const unsigned int data_len)
{
	const struct sock_ee *const err = cmsg_data;

	tprintf("{ee_errno=%u, ee_origin=%u, ee_type=%u, ee_code=%u"
		", ee_info=%u, ee_data=%u, offender=",
		err->ee_errno, err->ee_origin, err->ee_type,
		err->ee_code, err->ee_info, err->ee_data);
	print_sockaddr(tcp, &err->offender, sizeof(err->offender));
	tprints("}");
}

static void
print_cmsg_ip_origdstaddr(struct tcb *tcp, const void *cmsg_data,
			  const unsigned int data_len)
{
	const unsigned int addr_len =
		data_len > sizeof(struct sockaddr_storage)
		? sizeof(struct sockaddr_storage) : data_len;

	print_sockaddr(tcp, cmsg_data, addr_len);
}

typedef void (* const cmsg_printer)(struct tcb *, const void *, unsigned int);

static const struct {
	const cmsg_printer printer;
	const unsigned int min_len;
} cmsg_socket_printers[] = {
	[SCM_RIGHTS] = { print_scm_rights, sizeof(int) },
	[SCM_CREDENTIALS] = { print_scm_creds, sizeof(struct ucred) },
	[SCM_SECURITY] = { print_scm_security, 1 }
}, cmsg_ip_printers[] = {
	[IP_PKTINFO] = { print_cmsg_ip_pktinfo, sizeof(struct in_pktinfo) },
	[IP_TTL] = { print_cmsg_uint, sizeof(unsigned int) },
	[IP_TOS] = { print_cmsg_uint8_t, 1 },
	[IP_RECVOPTS] = { print_cmsg_ip_opts, 1 },
	[IP_RETOPTS] = { print_cmsg_ip_opts, 1 },
	[IP_RECVERR] = { print_cmsg_ip_recverr, sizeof(struct sock_ee) },
	[IP_ORIGDSTADDR] = { print_cmsg_ip_origdstaddr, sizeof(struct sockaddr_in) },
	[IP_CHECKSUM] = { print_cmsg_uint, sizeof(unsigned int) },
	[SCM_SECURITY] = { print_scm_security, 1 }
};

static void
print_cmsg_type_data(struct tcb *tcp, const int cmsg_level, const int cmsg_type,
		     const void *cmsg_data, const unsigned int data_len)
{
	const unsigned int utype = cmsg_type;
	switch (cmsg_level) {
	case SOL_SOCKET:
		printxval(scmvals, cmsg_type, "SCM_???");
		if (utype < ARRAY_SIZE(cmsg_socket_printers)
		    && cmsg_socket_printers[utype].printer
		    && data_len >= cmsg_socket_printers[utype].min_len) {
			tprints(", cmsg_data=");
			cmsg_socket_printers[utype].printer(tcp, cmsg_data, data_len);
		}
		break;
	case SOL_IP:
		printxval(ip_cmsg_types, cmsg_type, "IP_???");
		if (utype < ARRAY_SIZE(cmsg_ip_printers)
		    && cmsg_ip_printers[utype].printer
		    && data_len >= cmsg_ip_printers[utype].min_len) {
			tprints(", cmsg_data=");
			cmsg_ip_printers[utype].printer(tcp, cmsg_data, data_len);
		}
		break;
	default:
		tprintf("%#x", cmsg_type);
	}
}

static unsigned int
get_optmem_max(void)
{
	static int optmem_max;

	if (!optmem_max) {
		if (read_int_from_file("/proc/sys/net/core/optmem_max",
				       &optmem_max) || optmem_max <= 0) {
			optmem_max = sizeof(long long) * (2 * IOV_MAX + 512);
		} else {
			optmem_max = (optmem_max + sizeof(long long) - 1)
				     & ~(sizeof(long long) - 1);
		}
	}

	return optmem_max;
}

static void
decode_msg_control(struct tcb *const tcp, const kernel_ulong_t addr,
		   const kernel_ulong_t in_control_len)
{
	if (!in_control_len)
		return;
	tprints(", msg_control=");

	const unsigned int cmsg_size =
#ifndef current_wordsize
		(current_wordsize < sizeof(long)) ? sizeof(struct cmsghdr32) :
#endif
			sizeof(struct cmsghdr);

	unsigned int control_len = in_control_len > get_optmem_max()
				   ? get_optmem_max() : in_control_len;
	unsigned int buf_len = control_len;
	char *buf = buf_len < cmsg_size ? NULL : malloc(buf_len);
	if (!buf || umoven(tcp, addr, buf_len, buf) < 0) {
		printaddr(addr);
		free(buf);
		return;
	}

	union_cmsghdr u = { .ptr = buf };

	tprints("[");
	while (buf_len >= cmsg_size) {
		const kernel_ulong_t cmsg_len =
#ifndef current_wordsize
			(current_wordsize < sizeof(long)) ? u.cmsg32->cmsg_len :
#endif
				u.cmsg->cmsg_len;
		const int cmsg_level =
#ifndef current_wordsize
			(current_wordsize < sizeof(long)) ? u.cmsg32->cmsg_level :
#endif
				u.cmsg->cmsg_level;
		const int cmsg_type =
#ifndef current_wordsize
			(current_wordsize < sizeof(long)) ? u.cmsg32->cmsg_type :
#endif
				u.cmsg->cmsg_type;

		if (u.ptr != buf)
			tprints(", ");
		tprintf("{cmsg_len=%" PRI_klu ", cmsg_level=", cmsg_len);
		printxval(socketlayers, cmsg_level, "SOL_???");
		tprints(", cmsg_type=");

		kernel_ulong_t len = cmsg_len > buf_len ? buf_len : cmsg_len;

		print_cmsg_type_data(tcp, cmsg_level, cmsg_type,
				     (const void *) (u.ptr + cmsg_size),
				     len > cmsg_size ? len - cmsg_size: 0);
		tprints("}");

		if (len < cmsg_size) {
			buf_len -= cmsg_size;
			break;
		}
		len = (cmsg_len + current_wordsize - 1) &
			~((kernel_ulong_t) current_wordsize - 1);
		if (len >= buf_len) {
			buf_len = 0;
			break;
		}
		u.ptr += len;
		buf_len -= len;
	}
	if (buf_len) {
		tprints(", ");
		printaddr(addr + (control_len - buf_len));
	} else if (control_len < in_control_len) {
		tprints(", ...");
	}
	tprints("]");
	free(buf);
}

void
print_struct_msghdr(struct tcb *tcp, const struct msghdr *msg,
		    const int *const p_user_msg_namelen,
		    const kernel_ulong_t data_size)
{
	const int msg_namelen =
		p_user_msg_namelen && (int) msg->msg_namelen > *p_user_msg_namelen
		? *p_user_msg_namelen : (int) msg->msg_namelen;

	tprints("{msg_name=");
	const int family =
		decode_sockaddr(tcp, ptr_to_kulong(msg->msg_name), msg_namelen);
	const enum iov_decode decode =
		(family == AF_NETLINK) ? IOV_DECODE_NETLINK : IOV_DECODE_STR;

	tprints(", msg_namelen=");
	if (p_user_msg_namelen && *p_user_msg_namelen != (int) msg->msg_namelen)
		tprintf("%d->", *p_user_msg_namelen);
	tprintf("%d", msg->msg_namelen);

	tprints(", msg_iov=");

	tprint_iov_upto(tcp, msg->msg_iovlen,
			ptr_to_kulong(msg->msg_iov), decode, data_size);
	tprintf(", msg_iovlen=%" PRI_klu, (kernel_ulong_t) msg->msg_iovlen);

	decode_msg_control(tcp, ptr_to_kulong(msg->msg_control),
			   msg->msg_controllen);
	tprintf(", msg_controllen=%" PRI_klu, (kernel_ulong_t) msg->msg_controllen);

	tprints(", msg_flags=");
	printflags(msg_flags, msg->msg_flags, "MSG_???");
	tprints("}");
}

static bool
fetch_msghdr_namelen(struct tcb *const tcp, const kernel_ulong_t addr,
		     int *const p_msg_namelen)
{
	struct msghdr msg;

	if (addr && verbose(tcp) && fetch_struct_msghdr(tcp, addr, &msg)) {
		*p_msg_namelen = msg.msg_namelen;
		return true;
	} else {
		return false;
	}
}

static void
decode_msghdr(struct tcb *const tcp, const int *const p_user_msg_namelen,
	      const kernel_ulong_t addr, const kernel_ulong_t data_size)
{
	struct msghdr msg;

	if (addr && verbose(tcp) && fetch_struct_msghdr(tcp, addr, &msg))
		print_struct_msghdr(tcp, &msg, p_user_msg_namelen, data_size);
	else
		printaddr(addr);
}

void
dumpiov_in_msghdr(struct tcb *const tcp, const kernel_ulong_t addr,
		  const kernel_ulong_t data_size)
{
	struct msghdr msg;

	if (fetch_struct_msghdr(tcp, addr, &msg)) {
		dumpiov_upto(tcp, msg.msg_iovlen,
			     ptr_to_kulong(msg.msg_iov), data_size);
	}
}

SYS_FUNC(sendmsg)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	decode_msghdr(tcp, 0, tcp->u_arg[1], -1);
	/* flags */
	tprints(", ");
	printflags(msg_flags, tcp->u_arg[2], "MSG_???");

	return RVAL_DECODED;
}

SYS_FUNC(recvmsg)
{
	int msg_namelen;

	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
		if (fetch_msghdr_namelen(tcp, tcp->u_arg[1], &msg_namelen)) {
			set_tcb_priv_ulong(tcp, msg_namelen);
			return 0;
		}
		printaddr(tcp->u_arg[1]);
	} else {
		msg_namelen = get_tcb_priv_ulong(tcp);

		if (syserror(tcp))
			tprintf("{msg_namelen=%d}", msg_namelen);
		else
			decode_msghdr(tcp, &msg_namelen, tcp->u_arg[1],
				      tcp->u_rval);
	}

	/* flags */
	tprints(", ");
	printflags(msg_flags, tcp->u_arg[2], "MSG_???");

	return RVAL_DECODED;
}
