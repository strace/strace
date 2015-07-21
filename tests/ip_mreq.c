#ifdef HAVE_CONFIG_H
# include "config.h"
#endif
#include <assert.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int
main(void)
{
#if defined IP_ADD_MEMBERSHIP && defined IPV6_ADD_MEMBERSHIP \
 && defined IPV6_JOIN_ANYCAST && defined HAVE_INET_PTON
	struct ip_mreq m4;
	struct ipv6_mreq m6;

	inet_pton(AF_INET, "224.0.0.3", &m4.imr_multiaddr);
	inet_pton(AF_INET, "127.0.0.1", &m4.imr_interface);
	inet_pton(AF_INET6, "ff01::c", &m6.ipv6mr_multiaddr);
	m6.ipv6mr_interface = 1;

	(void) close(0);
	assert(socket(AF_INET, SOCK_DGRAM, 0) == 0);

	assert(setsockopt(0, SOL_IP, IP_ADD_MEMBERSHIP, &m4, 1) == -1);
	assert(setsockopt(0, SOL_IP, IP_DROP_MEMBERSHIP, &m4, 1) == -1);
	if (setsockopt(0, SOL_IP, IP_ADD_MEMBERSHIP, &m4, sizeof(m4)) ||
	    setsockopt(0, SOL_IP, IP_DROP_MEMBERSHIP, &m4, sizeof(m4)))
		return 77;

	assert(setsockopt(0, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &m6, 1) == -1);
	assert(setsockopt(0, SOL_IPV6, IPV6_DROP_MEMBERSHIP, &m6, 1) == -1);
	assert(setsockopt(0, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &m6, sizeof(m6)) == -1);
	assert(setsockopt(0, SOL_IPV6, IPV6_DROP_MEMBERSHIP, &m6, sizeof(m6)) == -1);

	assert(setsockopt(0, SOL_IPV6, IPV6_JOIN_ANYCAST, &m6, 1) == -1);
	assert(setsockopt(0, SOL_IPV6, IPV6_LEAVE_ANYCAST, &m6, 1) == -1);
	assert(setsockopt(0, SOL_IPV6, IPV6_JOIN_ANYCAST, &m6, sizeof(m6)) == -1);
	assert(setsockopt(0, SOL_IPV6, IPV6_LEAVE_ANYCAST, &m6, sizeof(m6)) == -1);

	return 0;
#else
	return 77;
#endif
}
