#include <assert.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <linux/netlink.h>
#include <linux/sock_diag.h>
#include <linux/inet_diag.h>

static int
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

	return sendmsg(fd, &msg, 0) > 0;
}

static int
check_responses(const int fd)
{
	static char buf[8192];
	struct sockaddr_nl nladdr;
	struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf)
	};

	memset(&nladdr, 0, sizeof(nladdr));
	nladdr.nl_family = AF_NETLINK;

	struct msghdr msg = {
		.msg_name = (void*)&nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = NULL,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	ssize_t ret = recvmsg(fd, &msg, 0);
	if (ret <= 0)
		return 0;

	struct nlmsghdr *h = (struct nlmsghdr*)buf;
	return (NLMSG_OK(h, ret) &&
		h->nlmsg_type != NLMSG_ERROR &&
		h->nlmsg_type != NLMSG_DONE) ? 1 : 0;
}

int main(void)
{
	struct sockaddr_in addr;
	socklen_t len = sizeof(addr);

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

	close(0);
	close(1);

	assert(socket(PF_INET, SOCK_STREAM, 0) == 0);
	assert(bind(0, (struct sockaddr *) &addr, len) == 0);
	assert(listen(0, 5) == 0);

	if (socket(AF_NETLINK, SOCK_RAW, NETLINK_INET_DIAG) != 1)
		return 77;

	return (send_query(1, AF_INET, IPPROTO_TCP) &&
		check_responses(1)) ? 0 : 77;
}
