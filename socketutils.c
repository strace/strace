/*
 * Copyright (c) 2014 Zubin Mithra <zubin.mithra@gmail.com>
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
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
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/sock_diag.h>
#include <linux/inet_diag.h>
#include <linux/unix_diag.h>
#include <linux/netlink_diag.h>
#include <linux/rtnetlink.h>

#include <sys/un.h>
#ifndef UNIX_PATH_MAX
# define UNIX_PATH_MAX sizeof(((struct sockaddr_un *) 0)->sun_path)
#endif

#ifndef NETLINK_SOCK_DIAG
# define NETLINK_SOCK_DIAG 4
#endif

typedef struct {
	unsigned long inode;
	char *details;
} cache_entry;

#define CACHE_SIZE 1024U
static cache_entry cache[CACHE_SIZE];
#define CACHE_MASK (CACHE_SIZE - 1)

static int
cache_and_print_inode_details(const unsigned long inode, char *const details)
{
	cache_entry *e = &cache[inode & CACHE_MASK];
	free(e->details);
	e->inode = inode;
	e->details = details;

	tprints(details);
	return 1;
}

bool
print_sockaddr_by_inode_cached(const unsigned long inode)
{
	const cache_entry *const e = &cache[inode & CACHE_MASK];
	if (e && inode == e->inode) {
		tprints(e->details);
		return true;
	}
	return false;
}

static bool
send_query(const int fd, void *req, size_t req_size)
{
	struct sockaddr_nl nladdr = {
		.nl_family = AF_NETLINK
	};
	struct iovec iov = {
		.iov_base = req,
		.iov_len = req_size
	};
	const struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1
	};

	for (;;) {
		if (sendmsg(fd, &msg, 0) < 0) {
			if (errno == EINTR)
				continue;
			return false;
		}
		return true;
	}
}

static bool
inet_send_query(const int fd, const int family, const int proto)
{
	struct {
		const struct nlmsghdr nlh;
		const struct inet_diag_req_v2 idr;
	} req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = SOCK_DIAG_BY_FAMILY,
			.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST
		},
		.idr = {
			.sdiag_family = family,
			.sdiag_protocol = proto,
			.idiag_states = -1
		}
	};
	return send_query(fd, &req, sizeof(req));
}

static int
inet_parse_response(const char *const proto_name, const void *const data,
		    const int data_len, const unsigned long inode)
{
	const struct inet_diag_msg *const diag_msg = data;
	static const char zero_addr[sizeof(struct in6_addr)];
	socklen_t addr_size, text_size;

	if (data_len < (int) NLMSG_LENGTH(sizeof(*diag_msg)))
		return -1;
	if (diag_msg->idiag_inode != inode)
		return 0;

	switch(diag_msg->idiag_family) {
		case AF_INET:
			addr_size = sizeof(struct in_addr);
			text_size = INET_ADDRSTRLEN;
			break;
		case AF_INET6:
			addr_size = sizeof(struct in6_addr);
			text_size = INET6_ADDRSTRLEN;
			break;
		default:
			return -1;
	}

	char src_buf[text_size];
	char *details;

	if (!inet_ntop(diag_msg->idiag_family, diag_msg->id.idiag_src,
		       src_buf, text_size))
		return -1;

	if (diag_msg->id.idiag_dport ||
	    memcmp(zero_addr, diag_msg->id.idiag_dst, addr_size)) {
		char dst_buf[text_size];

		if (!inet_ntop(diag_msg->idiag_family, diag_msg->id.idiag_dst,
			       dst_buf, text_size))
			return -1;

		if (asprintf(&details, "%s:[%s:%u->%s:%u]", proto_name,
			     src_buf, ntohs(diag_msg->id.idiag_sport),
			     dst_buf, ntohs(diag_msg->id.idiag_dport)) < 0)
			return false;
	} else {
		if (asprintf(&details, "%s:[%s:%u]", proto_name, src_buf,
			     ntohs(diag_msg->id.idiag_sport)) < 0)
			return false;
	}

	return cache_and_print_inode_details(inode, details);
}

static bool
receive_responses(const int fd, const unsigned long inode,
		  const char *proto_name,
		  int (* parser) (const char *, const void *,
				  int, unsigned long))
{
	static union {
		struct nlmsghdr hdr;
		long buf[8192 / sizeof(long)];
	} hdr_buf;

	struct sockaddr_nl nladdr = {
		.nl_family = AF_NETLINK
	};
	struct iovec iov = {
		.iov_base = hdr_buf.buf,
		.iov_len = sizeof(hdr_buf.buf)
	};
	int flags = 0;

	for (;;) {
		struct msghdr msg = {
			.msg_name = &nladdr,
			.msg_namelen = sizeof(nladdr),
			.msg_iov = &iov,
			.msg_iovlen = 1
		};

		ssize_t ret = recvmsg(fd, &msg, flags);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			return false;
		}

		const struct nlmsghdr *h = &hdr_buf.hdr;
		if (!NLMSG_OK(h, ret))
			return false;
		for (; NLMSG_OK(h, ret); h = NLMSG_NEXT(h, ret)) {
			if (h->nlmsg_type != SOCK_DIAG_BY_FAMILY)
				return false;
			const int rc = parser(proto_name, NLMSG_DATA(h),
					      h->nlmsg_len, inode);
			if (rc > 0)
				return true;
			if (rc < 0)
				return false;
		}
		flags = MSG_DONTWAIT;
	}
}

static bool
inet_print(const int fd, const int family, const int protocol,
	   const unsigned long inode, const char *proto_name)
{
	return inet_send_query(fd, family, protocol)
		&& receive_responses(fd, inode, proto_name, inet_parse_response);
}

static bool
unix_send_query(const int fd, const unsigned long inode)
{
	struct {
		const struct nlmsghdr nlh;
		const struct unix_diag_req udr;
	} req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = SOCK_DIAG_BY_FAMILY,
			.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST
		},
		.udr = {
			.sdiag_family = AF_UNIX,
			.udiag_ino = inode,
			.udiag_states = -1,
			.udiag_show = UDIAG_SHOW_NAME | UDIAG_SHOW_PEER
		}
	};
	return send_query(fd, &req, sizeof(req));
}

static int
unix_parse_response(const char *proto_name, const void *data,
		    const int data_len, const unsigned long inode)
{
	const struct unix_diag_msg *diag_msg = data;
	struct rtattr *attr;
	int rta_len = data_len - NLMSG_LENGTH(sizeof(*diag_msg));
	uint32_t peer = 0;
	size_t path_len = 0;
	char path[UNIX_PATH_MAX + 1];

	if (rta_len < 0)
		return -1;
	if (diag_msg->udiag_ino != inode)
		return 0;
	if (diag_msg->udiag_family != AF_UNIX)
		return -1;

	for (attr = (struct rtattr *) (diag_msg + 1);
	     RTA_OK(attr, rta_len);
	     attr = RTA_NEXT(attr, rta_len)) {
		switch (attr->rta_type) {
		case UNIX_DIAG_NAME:
			if (!path_len) {
				path_len = RTA_PAYLOAD(attr);
				if (path_len > UNIX_PATH_MAX)
					path_len = UNIX_PATH_MAX;
				memcpy(path, RTA_DATA(attr), path_len);
				path[path_len] = '\0';
			}
			break;
		case UNIX_DIAG_PEER:
			if (RTA_PAYLOAD(attr) >= 4)
				peer = *(uint32_t *) RTA_DATA(attr);
			break;
		}
	}

	/*
	 * print obtained information in the following format:
	 * "UNIX:[" SELF_INODE [ "->" PEER_INODE ][ "," SOCKET_FILE ] "]"
	 */
	if (!peer && !path_len)
		return -1;

	char peer_str[3 + sizeof(peer) * 3];
	if (peer)
		snprintf(peer_str, sizeof(peer_str), "->%u", peer);
	else
		peer_str[0] = '\0';

	const char *path_str;
	if (path_len) {
		char *outstr = alloca(4 * path_len + 4);

		outstr[0] = ',';
		if (path[0] == '\0') {
			outstr[1] = '@';
			string_quote(path + 1, outstr + 2,
				     path_len - 1, QUOTE_0_TERMINATED);
		} else {
			string_quote(path, outstr + 1,
				     path_len, QUOTE_0_TERMINATED);
		}
		path_str = outstr;
	} else {
		path_str = "";
	}

	char *details;
	if (asprintf(&details, "%s:[%lu%s%s]", proto_name, inode,
		     peer_str, path_str) < 0)
		return -1;

	return cache_and_print_inode_details(inode, details);
}

static bool
netlink_send_query(const int fd, const unsigned long inode)
{
	struct {
		const struct nlmsghdr nlh;
		const struct netlink_diag_req ndr;
	} req = {
		.nlh = {
			.nlmsg_len = sizeof(req),
			.nlmsg_type = SOCK_DIAG_BY_FAMILY,
			.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST
		},
		.ndr = {
			.sdiag_family = AF_NETLINK,
			.sdiag_protocol = NDIAG_PROTO_ALL,
			.ndiag_show = NDIAG_SHOW_MEMINFO
		}
	};
	return send_query(fd, &req, sizeof(req));
}

static int
netlink_parse_response(const char *proto_name, const void *data,
		    const int data_len, const unsigned long inode)
{
	const struct netlink_diag_msg *const diag_msg = data;
	const char *netlink_proto;
	char *details;

	if (data_len < (int) NLMSG_LENGTH(sizeof(*diag_msg)))
		return -1;
	if (diag_msg->ndiag_ino != inode)
		return 0;

	if (diag_msg->ndiag_family != AF_NETLINK)
		return -1;

	netlink_proto = xlookup(netlink_protocols,
				diag_msg->ndiag_protocol);

	if (netlink_proto) {
		static const char netlink_prefix[] = "NETLINK_";
		const size_t netlink_prefix_len =
			sizeof(netlink_prefix) -1;
		if (strncmp(netlink_proto, netlink_prefix,
			    netlink_prefix_len) == 0)
			netlink_proto += netlink_prefix_len;
		if (asprintf(&details, "%s:[%s:%u]", proto_name,
			     netlink_proto, diag_msg->ndiag_portid) < 0)
			return -1;
	} else {
		if (asprintf(&details, "%s:[%u]", proto_name,
			     (unsigned) diag_msg->ndiag_protocol) < 0)
			return -1;
	}

	return cache_and_print_inode_details(inode, details);
}

static bool
unix_print(const int fd, const unsigned long inode)
{
	return unix_send_query(fd, inode)
		&& receive_responses(fd, inode, "UNIX", unix_parse_response);
}

static bool
tcp_v4_print(const int fd, const unsigned long inode)
{
	return inet_print(fd, AF_INET, IPPROTO_TCP, inode, "TCP");
}

static bool
udp_v4_print(const int fd, const unsigned long inode)
{
	return inet_print(fd, AF_INET, IPPROTO_UDP, inode, "UDP");
}

static bool
tcp_v6_print(const int fd, const unsigned long inode)
{
	return inet_print(fd, AF_INET6, IPPROTO_TCP, inode, "TCPv6");
}

static bool
udp_v6_print(const int fd, const unsigned long inode)
{
	return inet_print(fd, AF_INET6, IPPROTO_UDP, inode, "UDPv6");
}

static bool
netlink_print(const int fd, const unsigned long inode)
{
	return netlink_send_query(fd, inode)
		&& receive_responses(fd, inode, "NETLINK",
				     netlink_parse_response);
}

static const struct {
	const char *const name;
	bool (*const print)(int, unsigned long);
} protocols[] = {
	[SOCK_PROTO_UNIX] = { "UNIX", unix_print },
	[SOCK_PROTO_TCP] = { "TCP", tcp_v4_print },
	[SOCK_PROTO_UDP] = { "UDP", udp_v4_print },
	[SOCK_PROTO_TCPv6] = { "TCPv6", tcp_v6_print },
	[SOCK_PROTO_UDPv6] = { "UDPv6", udp_v6_print },
	[SOCK_PROTO_NETLINK] = { "NETLINK", netlink_print }
};

enum sock_proto
get_proto_by_name(const char *const name)
{
	unsigned int i;
	for (i = (unsigned int) SOCK_PROTO_UNKNOWN + 1;
	     i < ARRAY_SIZE(protocols); ++i) {
		if (protocols[i].name && !strcmp(name, protocols[i].name))
			return (enum sock_proto) i;
	}
	return SOCK_PROTO_UNKNOWN;
}

/* Given an inode number of a socket, print out the details
 * of the ip address and port. */

bool
print_sockaddr_by_inode(const unsigned long inode, const enum sock_proto proto)
{
	if ((unsigned int) proto >= ARRAY_SIZE(protocols) ||
	    (proto != SOCK_PROTO_UNKNOWN && !protocols[proto].print))
		return false;

	const int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG);
	if (fd < 0)
		return false;
	bool r = false;

	if (proto != SOCK_PROTO_UNKNOWN) {
		r = protocols[proto].print(fd, inode);
		if (!r) {
			tprintf("%s:[%lu]", protocols[proto].name, inode);
			r = true;
		}
	} else {
		unsigned int i;
		for (i = (unsigned int) SOCK_PROTO_UNKNOWN + 1;
		     i < ARRAY_SIZE(protocols); ++i) {
			if (!protocols[i].print)
				continue;
			r = protocols[i].print(fd, inode);
			if (r)
				break;
		}
	}

	close(fd);
	return r;
}
