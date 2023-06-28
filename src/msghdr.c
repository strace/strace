/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2000 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 2005-2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "msghdr.h"
#include <limits.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define XLAT_MACROS_ONLY
#include "xlat/sock_options.h"
#undef XLAT_MACROS_ONLY
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

	print_local_array_ex(tcp, fds, nfds, sizeof(*fds),
			     print_fd_array_member, NULL, 0, NULL, NULL);

}

static void
print_scm_creds(struct tcb *tcp, const void *cmsg_data,
		const unsigned int data_len)
{
	const struct ucred *uc = cmsg_data;

	tprint_struct_begin();
	PRINT_FIELD_TGID(*uc, pid, tcp);
	tprint_struct_next();
	PRINT_FIELD_ID(*uc, uid);
	tprint_struct_next();
	PRINT_FIELD_ID(*uc, gid);
	tprint_struct_end();
}

static void
print_scm_security(struct tcb *tcp, const void *cmsg_data,
		   const unsigned int data_len)
{
	print_quoted_string(cmsg_data, data_len, 0);
}

static void
print_scm_timestamp_old(struct tcb *tcp, const void *cmsg_data,
			const unsigned int data_len)
{
	print_struct_timeval_data_size(cmsg_data, data_len);
}

#ifdef current_klongsize
# if current_klongsize == 4
#  define PRINT_TIMESPEC_DATA_SIZE print_timespec32_data_size
#  define PRINT_TIMESPEC_ARRAY_DATA_SIZE print_timespec32_array_data_size
# else
#  define PRINT_TIMESPEC_DATA_SIZE print_timespec64_data_size
#  define PRINT_TIMESPEC_ARRAY_DATA_SIZE print_timespec64_array_data_size
# endif
#else
# define PRINT_TIMESPEC_DATA_SIZE			\
	((current_klongsize == 4) ?			\
		print_timespec32_data_size :		\
		print_timespec64_data_size)
# define PRINT_TIMESPEC_ARRAY_DATA_SIZE			\
	((current_klongsize == 4) ?			\
		print_timespec32_array_data_size :	\
		print_timespec64_array_data_size)
#endif

static void
print_scm_timestampns_old(struct tcb *tcp, const void *cmsg_data,
			  const unsigned int data_len)
{
	PRINT_TIMESPEC_DATA_SIZE(cmsg_data, data_len);
}

static void
print_scm_timestamping_old(struct tcb *tcp, const void *cmsg_data,
			   const unsigned int data_len)
{
	PRINT_TIMESPEC_ARRAY_DATA_SIZE(cmsg_data, 3, data_len);
}

static void
print_scm_timestamp_new(struct tcb *tcp, const void *cmsg_data,
			const unsigned int data_len)
{
	print_timeval64_data_size(cmsg_data, data_len);
}

static void
print_scm_timestampns_new(struct tcb *tcp, const void *cmsg_data,
			const unsigned int data_len)
{
	print_timespec64_data_size(cmsg_data, data_len);
}

static void
print_scm_timestamping_new(struct tcb *tcp, const void *cmsg_data,
			   const unsigned int data_len)
{
	print_timespec64_array_data_size(cmsg_data, 3, data_len);
}

static void
print_cmsg_ip_pktinfo(struct tcb *tcp, const void *cmsg_data,
		      const unsigned int data_len)
{
	const struct in_pktinfo *info = cmsg_data;

	tprint_struct_begin();
	PRINT_FIELD_IFINDEX(*info, ipi_ifindex);
	tprint_struct_next();
	PRINT_FIELD_INET_ADDR(*info, ipi_spec_dst, AF_INET);
	tprint_struct_next();
	PRINT_FIELD_INET_ADDR(*info, ipi_addr, AF_INET);
	tprint_struct_end();
}

static void
print_cmsg_uint(struct tcb *tcp, const void *cmsg_data,
		const unsigned int data_len)
{
	const unsigned int *p = cmsg_data;

	print_local_array_ex(tcp, p, data_len / sizeof(*p), sizeof(*p),
			     print_uint_array_member, NULL, 0, NULL, NULL);
}

static void
print_cmsg_xint8_t(struct tcb *tcp, const void *cmsg_data,
		   const unsigned int data_len)
{
	print_local_array_ex(tcp, cmsg_data, data_len, 1,
			     print_xint_array_member, NULL, 0, NULL, NULL);
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

	tprint_struct_begin();
	PRINT_FIELD_U(*err, ee_errno);
	tprint_struct_next();
	PRINT_FIELD_U(*err, ee_origin);
	tprint_struct_next();
	PRINT_FIELD_U(*err, ee_type);
	tprint_struct_next();
	PRINT_FIELD_U(*err, ee_code);
	tprint_struct_next();
	PRINT_FIELD_U(*err, ee_info);
	tprint_struct_next();
	PRINT_FIELD_U(*err, ee_data);
	tprint_struct_next();
	PRINT_FIELD_SOCKADDR(*err, offender, tcp);
	tprint_struct_end();
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

static void
print_cmsg_ip_protocol(struct tcb *tcp, const void *cmsg_data,
		       const unsigned int data_len)
{
	const unsigned int *protocol = cmsg_data;

	tprint_indirect_begin();
	printxval(inet_protocols, *protocol, "IP_???");
	tprint_indirect_end();
}

typedef void (* const cmsg_printer)(struct tcb *, const void *, unsigned int);

static const struct {
	const cmsg_printer printer;
	const unsigned int min_len;
} cmsg_socket_printers[] = {
	[SCM_RIGHTS] = { print_scm_rights, sizeof(int) },
	[SCM_CREDENTIALS] = { print_scm_creds, sizeof(struct ucred) },
	[SCM_SECURITY] = { print_scm_security, 1 },
	[SO_TIMESTAMP_OLD] = { print_scm_timestamp_old, 1 },
	[SO_TIMESTAMPNS_OLD] = { print_scm_timestampns_old, 1 },
	[SO_TIMESTAMPING_OLD] = { print_scm_timestamping_old, 1 },
	[SO_TIMESTAMP_NEW] = { print_scm_timestamp_new, 1 },
	[SO_TIMESTAMPNS_NEW] = { print_scm_timestampns_new, 1 },
	[SO_TIMESTAMPING_NEW] = { print_scm_timestamping_new, 1 }
}, cmsg_ip_printers[] = {
	[IP_PKTINFO] = { print_cmsg_ip_pktinfo, sizeof(struct in_pktinfo) },
	[IP_TTL] = { print_cmsg_uint, sizeof(unsigned int) },
	[IP_TOS] = { print_cmsg_xint8_t, 1 },
	[IP_RECVOPTS] = { print_cmsg_xint8_t, 1 },
	[IP_RETOPTS] = { print_cmsg_xint8_t, 1 },
	[IP_RECVERR] = { print_cmsg_ip_recverr, sizeof(struct sock_ee) },
	[IP_ORIGDSTADDR] = { print_cmsg_ip_origdstaddr, sizeof(struct sockaddr_in) },
	[IP_CHECKSUM] = { print_cmsg_uint, sizeof(unsigned int) },
	[IP_PROTOCOL] = { print_cmsg_ip_protocol, sizeof(unsigned int) },
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
			tprint_struct_next();
			tprints_field_name("cmsg_data");
			cmsg_socket_printers[utype].printer(tcp, cmsg_data, data_len);
		}
		break;
	case SOL_IP:
		printxval(ip_cmsg_types, cmsg_type, "IP_???");
		if (utype < ARRAY_SIZE(cmsg_ip_printers)
		    && cmsg_ip_printers[utype].printer
		    && data_len >= cmsg_ip_printers[utype].min_len) {
			tprint_struct_next();
			tprints_field_name("cmsg_data");
			cmsg_ip_printers[utype].printer(tcp, cmsg_data, data_len);
		}
		break;
	default:
		PRINT_VAL_X(cmsg_type);
	}
}

static unsigned int
get_optmem_max(struct tcb *tcp)
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
	tprint_struct_next();
	tprints_field_name("msg_control");

	const unsigned int cmsg_size =
#ifndef current_wordsize
		(current_wordsize < sizeof(long)) ? sizeof(struct cmsghdr32) :
#endif
			sizeof(struct cmsghdr);

	unsigned int control_len = in_control_len > get_optmem_max(tcp)
				   ? get_optmem_max(tcp) : in_control_len;
	unsigned int buf_len = control_len;
	char *buf = buf_len < cmsg_size ? NULL : malloc(buf_len);
	if (!buf || umoven(tcp, addr, buf_len, buf) < 0) {
		printaddr(addr);
		free(buf);
		return;
	}

	union_cmsghdr u = { .ptr = buf };

	tprint_array_begin();
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
			tprint_array_next();
		tprint_struct_begin();
		tprints_field_name("cmsg_len");
		PRINT_VAL_U(cmsg_len);
		tprint_struct_next();
		tprints_field_name("cmsg_level");
		printxval(socketlayers, cmsg_level, "SOL_???");
		tprint_struct_next();
		tprints_field_name("cmsg_type");

		kernel_ulong_t len = cmsg_len > buf_len ? buf_len : cmsg_len;

		print_cmsg_type_data(tcp, cmsg_level, cmsg_type,
				     (const void *) (u.ptr + cmsg_size),
				     len > cmsg_size ? len - cmsg_size : 0);
		tprint_struct_end();

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
		tprint_array_next();
		tprint_more_data_follows();
		printaddr_comment(addr + (control_len - buf_len));
	} else if (control_len < in_control_len) {
		tprint_array_next();
		tprint_more_data_follows();
	}
	tprint_array_end();
	free(buf);
}

static void
iov_decode_netlink(struct tcb *tcp, kernel_ulong_t addr, kernel_ulong_t size,
		   void *opaque_data)
{
	const int *const fdp = opaque_data;
	decode_netlink(tcp, *fdp, addr, size);
}

void
print_struct_msghdr(struct tcb *tcp, const struct msghdr *msg,
		    const int *const p_user_msg_namelen,
		    const kernel_ulong_t data_size)
{
	const int msg_namelen =
		p_user_msg_namelen && (int) msg->msg_namelen > *p_user_msg_namelen
		? *p_user_msg_namelen : (int) msg->msg_namelen;

	tprint_struct_begin();
	tprints_field_name("msg_name");
	const int family =
		decode_sockaddr(tcp, ptr_to_kulong(msg->msg_name), msg_namelen);
	/* Assume that the descriptor is the 1st syscall argument. */
	int fd = tcp->u_arg[0];
	const enum sock_proto proto =
		(family == -1) ? getfdproto(tcp, fd) : SOCK_PROTO_UNKNOWN;
	const print_obj_by_addr_size_fn print_func =
		(family == AF_NETLINK || proto == SOCK_PROTO_NETLINK)
		? iov_decode_netlink : iov_decode_str;

	tprint_struct_next();
	tprints_field_name("msg_namelen");
	if (p_user_msg_namelen && *p_user_msg_namelen != (int) msg->msg_namelen) {
		PRINT_VAL_D(*p_user_msg_namelen);
		tprint_value_changed();
	}
	PRINT_VAL_D(msg->msg_namelen);

	tprint_struct_next();
	tprints_field_name("msg_iov");
	tprint_iov_upto(tcp, msg->msg_iovlen,
			ptr_to_kulong(msg->msg_iov), data_size, print_func, &fd);
	tprint_struct_next();
	PRINT_FIELD_U(*msg, msg_iovlen);

	decode_msg_control(tcp, ptr_to_kulong(msg->msg_control),
			   msg->msg_controllen);
	tprint_struct_next();
	PRINT_FIELD_U(*msg, msg_controllen);

	tprint_struct_next();
	PRINT_FIELD_FLAGS(*msg, msg_flags, msg_flags, "MSG_???");
	tprint_struct_end();
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
	/* sockfd */
	printfd(tcp, tcp->u_arg[0]);
	tprint_arg_next();

	/* msg */
	decode_msghdr(tcp, 0, tcp->u_arg[1], -1);
	tprint_arg_next();

	/* flags */
	printflags(msg_flags, tcp->u_arg[2], "MSG_???");

	return RVAL_DECODED;
}

SYS_FUNC(recvmsg)
{
	int msg_namelen;

	if (entering(tcp)) {
		/* sockfd */
		printfd(tcp, tcp->u_arg[0]);
		tprint_arg_next();

		if (fetch_msghdr_namelen(tcp, tcp->u_arg[1], &msg_namelen)) {
			set_tcb_priv_ulong(tcp, msg_namelen);
			return 0;
		}
		/* msg */
		printaddr(tcp->u_arg[1]);
	} else {
		msg_namelen = get_tcb_priv_ulong(tcp);

		/* msg */
		if (syserror(tcp)) {
			tprint_struct_begin();
			tprints_field_name("msg_namelen");
			PRINT_VAL_D(msg_namelen);
			tprint_struct_end();
		} else {
			decode_msghdr(tcp, &msg_namelen, tcp->u_arg[1],
				      tcp->u_rval);
		}
	}
	tprint_arg_next();

	/* flags */
	printflags(msg_flags, tcp->u_arg[2], "MSG_???");

	return RVAL_DECODED;
}
