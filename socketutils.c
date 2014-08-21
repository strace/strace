#include "defs.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <linux/sock_diag.h>
#include <linux/inet_diag.h>

static bool
send_query(const int fd, const int family, const int proto)
{
	struct sockaddr_nl nladdr;
	struct {
		struct nlmsghdr nlh;
		struct inet_diag_req_v2 idr;
	} req;
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

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	memset(&req, 0, sizeof(req));
	req.nlh.nlmsg_len = sizeof(req);
	req.nlh.nlmsg_type = SOCK_DIAG_BY_FAMILY;
	req.nlh.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST;
	req.idr.sdiag_family = family;
	req.idr.sdiag_protocol = proto;
	req.idr.idiag_states = -1;

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
parse_response(const struct inet_diag_msg *diag_msg, const unsigned long inode)
{
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

		tprintf("%s:%u->%s:%u",
			src_buf, ntohs(diag_msg->id.idiag_sport),
			dst_buf, ntohs(diag_msg->id.idiag_dport));
	} else {
		tprintf("%s:%u", src_buf, ntohs(diag_msg->id.idiag_sport));
	}

	return true;
}

static bool
receive_responses(const int fd, const unsigned long inode)
{
	static char buf[8192];
	struct sockaddr_nl nladdr;
	struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf)
	};

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	for (;;) {
		ssize_t ret;
		struct nlmsghdr *h;
		struct msghdr msg = {
			.msg_name = (void*)&nladdr,
			.msg_namelen = sizeof(nladdr),
			.msg_iov = &iov,
			.msg_iovlen = 1,
			.msg_control = NULL,
			.msg_controllen = 0,
			.msg_flags = 0
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
			if (parse_response(NLMSG_DATA(h), inode))
				return true;
		}
	}
}

/* Given an inode number of a socket, print out the details
 * of the ip address and port. */
bool
print_sockaddr_by_inode(const unsigned long inode)
{
	const int families[] = {AF_INET, AF_INET6};
	const int protocols[] = {IPPROTO_TCP, IPPROTO_UDP};
	const size_t flen = ARRAY_SIZE(families);
	const size_t plen = ARRAY_SIZE(protocols);
	size_t fi, pi;
	int fd;

	fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_INET_DIAG);
	if (fd < 0)
		return false;

	for (fi = 0; fi < flen; ++fi) {
		for (pi = 0; pi < plen; ++pi) {
			if (!send_query(fd, families[fi], protocols[pi]))
				continue;
			if (receive_responses(fd, inode)) {
				close(fd);
				return true;
			}
		}
	}

	close(fd);
	return false;
}
