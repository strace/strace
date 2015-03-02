#include "defs.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/sock_diag.h>
#include <linux/inet_diag.h>
#include <linux/unix_diag.h>
#include <linux/rtnetlink.h>

#if !defined NETLINK_SOCK_DIAG && defined NETLINK_INET_DIAG
# define NETLINK_SOCK_DIAG NETLINK_INET_DIAG
#endif

#include <sys/un.h>
#ifndef UNIX_PATH_MAX
# define UNIX_PATH_MAX sizeof(((struct sockaddr_un *) 0)->sun_path)
#endif

static bool
inet_send_query(const int fd, const int family, const int proto)
{
	struct sockaddr_nl nladdr = {
		.nl_family = AF_NETLINK
	};
	struct {
		struct nlmsghdr nlh;
		struct inet_diag_req_v2 idr;
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
	struct iovec iov = {
		.iov_base = &req,
		.iov_len = sizeof(req)
	};
	struct msghdr msg = {
		.msg_name = (void*)&nladdr,
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
inet_parse_response(const char *proto_name, const void *data, int data_len,
		    const unsigned long inode)
{
	const struct inet_diag_msg *diag_msg = data;
	static const char zero_addr[sizeof(struct in6_addr)];
	socklen_t addr_size, text_size;

	if (diag_msg->idiag_inode != inode)
		return false;

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
			return false;
	}

	char src_buf[text_size];

	if (!inet_ntop(diag_msg->idiag_family, diag_msg->id.idiag_src,
		       src_buf, text_size))
		return false;

	if (diag_msg->id.idiag_dport ||
	    memcmp(zero_addr, diag_msg->id.idiag_dst, addr_size)) {
		char dst_buf[text_size];

		if (!inet_ntop(diag_msg->idiag_family, diag_msg->id.idiag_dst,
			       dst_buf, text_size))
			return false;

		tprintf("%s:[%s:%u->%s:%u]",
			proto_name,
			src_buf, ntohs(diag_msg->id.idiag_sport),
			dst_buf, ntohs(diag_msg->id.idiag_dport));
	} else {
		tprintf("%s:[%s:%u]", proto_name, src_buf,
			ntohs(diag_msg->id.idiag_sport));
	}

	return true;
}

static bool
receive_responses(const int fd, const unsigned long inode,
		  const char *proto_name,
		  bool (* parser) (const char *, const void *, int, const unsigned long))
{
	static long buf[8192 / sizeof(long)];
	struct sockaddr_nl nladdr = {
		.nl_family = AF_NETLINK
	};
	struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf)
	};

	for (;;) {
		ssize_t ret;
		struct nlmsghdr *h;
		struct msghdr msg = {
			.msg_name = (void*)&nladdr,
			.msg_namelen = sizeof(nladdr),
			.msg_iov = &iov,
			.msg_iovlen = 1
		};

		ret = recvmsg(fd, &msg, 0);
		if (ret < 0) {
			if (errno == EINTR)
				continue;
			return false;
		}
		if (!ret)
			return false;
		for (h = (struct nlmsghdr*)buf;
		     NLMSG_OK(h, ret);
		     h = NLMSG_NEXT(h, ret)) {
			switch (h->nlmsg_type) {
				case NLMSG_DONE:
				case NLMSG_ERROR:
					return false;
			}
			if (parser(proto_name, NLMSG_DATA(h), h->nlmsg_len, inode))
				return true;
		}
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
	struct sockaddr_nl nladdr = {
		.nl_family = AF_NETLINK
	};
	struct {
		struct nlmsghdr nlh;
		struct unix_diag_req udr;
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
	struct iovec iov = {
		.iov_base = &req,
		.iov_len = sizeof(req)
	};
	struct msghdr msg = {
		.msg_name = (void*)&nladdr,
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
unix_parse_response(const char *proto_name, const void *data, int data_len,
		    const unsigned long inode)
{
	const struct unix_diag_msg *diag_msg = data;
	struct rtattr *attr;
	int rta_len = data_len - NLMSG_LENGTH(sizeof(*diag_msg));
	uint32_t peer = 0;
	size_t path_len = 0;
	char path[UNIX_PATH_MAX + 1];

	if (diag_msg->udiag_ino != inode)
		return false;
	if (diag_msg->udiag_family != AF_UNIX)
		return false;

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
				peer = *(uint32_t *)RTA_DATA(attr);
			break;
		}
	}

	/*
	 * print obtained information in the following format:
	 * "UNIX:[" SELF_INODE [ "->" PEER_INODE ][ "," SOCKET_FILE ] "]"
	 */
	if (peer || path_len) {
		tprintf("%s:[%lu", proto_name, inode);
		if (peer)
			tprintf("->%u", peer);
		if (path_len) {
			if (path[0] == '\0') {
				tprints(",@");
				print_quoted_string(path + 1, path_len,
						    QUOTE_0_TERMINATED);
			} else {
				tprints(",");
				print_quoted_string(path, path_len + 1,
						    QUOTE_0_TERMINATED);
			}
		}
		tprints("]");
		return true;
	}
	else
		return false;
}

static bool
unix_print(int fd, const unsigned long inode)
{
	return unix_send_query(fd, inode)
		&& receive_responses(fd, inode, "UNIX", unix_parse_response);
}

/* Given an inode number of a socket, print out the details
 * of the ip address and port. */
bool
print_sockaddr_by_inode(const unsigned long inode, const char *proto_name)
{
	int fd;
	bool r = false;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG);
	if (fd < 0)
		return false;

	if (proto_name) {
		if (strcmp(proto_name, "TCP") == 0)
			r = inet_print(fd, AF_INET, IPPROTO_TCP, inode, "TCP");
		else if (strcmp(proto_name, "UDP") == 0)
			r = inet_print(fd, AF_INET, IPPROTO_UDP, inode, "UDP");
		else if (strcmp(proto_name, "TCPv6") == 0)
			r = inet_print(fd, AF_INET6, IPPROTO_TCP, inode, "TCPv6");
		else if (strcmp(proto_name, "UDPv6") == 0)
			r = inet_print(fd, AF_INET6, IPPROTO_UDP, inode, "UDPv6");
		else if (strcmp(proto_name, "UNIX") == 0)
			r = unix_print(fd, inode);
	} else {
		const struct {
			const int family;
			const int protocol;
			const char *name;
		} protocols[] = {
			{ AF_INET, IPPROTO_TCP, "TCP" },
			{ AF_INET, IPPROTO_UDP, "UDP" },
			{ AF_INET6, IPPROTO_TCP, "TCPv6" },
			{ AF_INET6, IPPROTO_UDP, "UDPv6" }
		};
		size_t i;

		for (i = 0; i < ARRAY_SIZE(protocols); ++i) {
			if ((r = inet_print(fd, protocols[i].family,
					    protocols[i].protocol, inode,
					    protocols[i].name)))
				break;
		}
	}

	close(fd);
	return r;
}
