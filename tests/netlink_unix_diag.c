#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <linux/netlink.h>
#include <linux/sock_diag.h>
#include <linux/unix_diag.h>

#if !defined NETLINK_SOCK_DIAG && defined NETLINK_INET_DIAG
# define NETLINK_SOCK_DIAG NETLINK_INET_DIAG
#endif

static int
send_query(const int fd, const int family, const int proto)
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
			.sdiag_family = family,
			.sdiag_protocol = proto,
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

	return sendmsg(fd, &msg, 0) > 0;
}

static int
check_responses(const int fd)
{
	static char buf[8192];
	struct sockaddr_nl nladdr = {
		.nl_family = AF_NETLINK
	};
	struct iovec iov = {
		.iov_base = buf,
		.iov_len = sizeof(buf)
	};
	struct msghdr msg = {
		.msg_name = (void*)&nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1
	};

	ssize_t ret = recvmsg(fd, &msg, 0);
	if (ret <= 0)
		return 0;

	struct nlmsghdr *h = (struct nlmsghdr*)buf;
	return (NLMSG_OK(h, ret) &&
		h->nlmsg_type != NLMSG_ERROR &&
		h->nlmsg_type != NLMSG_DONE) ? 1 : 0;
}

#define SUN_PATH "netlink_unix_diag_socket"
int main(void)
{
	struct sockaddr_un addr = {
		.sun_family = AF_UNIX,
		.sun_path = SUN_PATH
	};
	socklen_t len = offsetof(struct sockaddr_un, sun_path) + sizeof(SUN_PATH);

	close(0);
	close(1);

	(void) unlink(SUN_PATH);
	if (socket(PF_LOCAL, SOCK_STREAM, 0) ||
	    bind(0, (struct sockaddr *) &addr, len) ||
	    listen(0, 5))
		return 77;

	assert(unlink(SUN_PATH) == 0);

	if (socket(AF_NETLINK, SOCK_RAW, NETLINK_SOCK_DIAG) != 1)
		return 77;

	return (send_query(1, AF_UNIX, 0) &&
		check_responses(1)) ? 0 : 77;
}
