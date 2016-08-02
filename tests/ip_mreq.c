/*
 * Copyright (c) 2015-2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include "tests.h"
#include <netinet/in.h>

#if defined IP_ADD_MEMBERSHIP && defined IPV6_ADD_MEMBERSHIP \
 && defined IPV6_JOIN_ANYCAST && defined HAVE_IF_INDEXTONAME

# include <assert.h>
# include <stdio.h>
# include <unistd.h>
# include <sys/socket.h>
# include <arpa/inet.h>
# include <net/if.h>

int
main(void)
{
	static const char multi4addr[] = "224.0.0.3";
	static const char multi6addr[] = "ff01::c";
	static const char interface[] = "127.0.0.1";

	struct ip_mreq m4;
	struct ipv6_mreq m6;

	inet_pton(AF_INET, multi4addr, &m4.imr_multiaddr);
	inet_pton(AF_INET, interface, &m4.imr_interface);
	inet_pton(AF_INET6, multi6addr, &m6.ipv6mr_multiaddr);

	m6.ipv6mr_interface = if_nametoindex("lo");
	if (!m6.ipv6mr_interface)
		perror_msg_and_skip("lo");

	(void) close(0);
	if (socket(AF_INET, SOCK_DGRAM, 0))
		perror_msg_and_skip("socket");

	if (setsockopt(0, SOL_IP, IP_ADD_MEMBERSHIP, &m4, sizeof(m4)) ||
	    setsockopt(0, SOL_IP, IP_DROP_MEMBERSHIP, &m4, sizeof(m4)))
		perror_msg_and_skip("setsockopt");
	printf("setsockopt(0, SOL_IP, IP_ADD_MEMBERSHIP"
	       ", {imr_multiaddr=inet_addr(\"%s\")"
	       ", imr_interface=inet_addr(\"%s\")}, %u) = 0\n",
	       multi4addr, interface, (unsigned) sizeof(m4));
	printf("setsockopt(0, SOL_IP, IP_DROP_MEMBERSHIP"
	       ", {imr_multiaddr=inet_addr(\"%s\")"
	       ", imr_interface=inet_addr(\"%s\")}, %u) = 0\n",
	       multi4addr, interface, (unsigned) sizeof(m4));

	assert(setsockopt(0, SOL_IP, IP_ADD_MEMBERSHIP, &m4, 1) == -1);
	printf("setsockopt(0, SOL_IP, IP_ADD_MEMBERSHIP, \"\\%hho\", 1)"
	       " = -1 %s (%m)\n",
	       * (unsigned char *) (void *) &m4, errno2name());

	assert(setsockopt(0, SOL_IP, IP_DROP_MEMBERSHIP, &m4, 1) == -1);
	printf("setsockopt(0, SOL_IP, IP_DROP_MEMBERSHIP, \"\\%hho\", 1)"
	       " = -1 %s (%m)\n",
	       * (unsigned char *) (void *) &m4, errno2name());

	assert(setsockopt(0, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &m6, 1) == -1);
	printf("setsockopt(0, SOL_IPV6, IPV6_ADD_MEMBERSHIP, \"\\%hho\", 1)"
	       " = -1 %s (%m)\n",
	       * (unsigned char *) (void *) &m6, errno2name());
	assert(setsockopt(0, SOL_IPV6, IPV6_DROP_MEMBERSHIP, &m6, 1) == -1);
	printf("setsockopt(0, SOL_IPV6, IPV6_DROP_MEMBERSHIP, \"\\%hho\", 1)"
	       " = -1 %s (%m)\n",
	       * (unsigned char *) (void *) &m6, errno2name());

	assert(setsockopt(0, SOL_IPV6, IPV6_ADD_MEMBERSHIP, &m6, sizeof(m6)) == -1);
	printf("setsockopt(0, SOL_IPV6, IPV6_ADD_MEMBERSHIP"
	       ", {ipv6mr_multiaddr=inet_pton(\"%s\")"
	       ", ipv6mr_interface=if_nametoindex(\"lo\")}, 20) = -1 %s (%m)\n",
	       multi6addr, errno2name());

	assert(setsockopt(0, SOL_IPV6, IPV6_DROP_MEMBERSHIP, &m6, sizeof(m6)) == -1);
	printf("setsockopt(0, SOL_IPV6, IPV6_DROP_MEMBERSHIP"
	       ", {ipv6mr_multiaddr=inet_pton(\"%s\")"
	       ", ipv6mr_interface=if_nametoindex(\"lo\")}, 20) = -1 %s (%m)\n",
	       multi6addr, errno2name());

	assert(setsockopt(0, SOL_IPV6, IPV6_JOIN_ANYCAST, &m6, 1) == -1);
	printf("setsockopt(0, SOL_IPV6, IPV6_JOIN_ANYCAST, \"\\%hho\", 1)"
	       " = -1 %s (%m)\n",
	       * (unsigned char *) (void *) &m6, errno2name());

	assert(setsockopt(0, SOL_IPV6, IPV6_LEAVE_ANYCAST, &m6, 1) == -1);
	printf("setsockopt(0, SOL_IPV6, IPV6_LEAVE_ANYCAST, \"\\%hho\", 1)"
	       " = -1 %s (%m)\n",
	       * (unsigned char *) (void *) &m6, errno2name());

	assert(setsockopt(0, SOL_IPV6, IPV6_JOIN_ANYCAST, &m6, sizeof(m6)) == -1);
	printf("setsockopt(0, SOL_IPV6, IPV6_JOIN_ANYCAST"
	       ", {ipv6mr_multiaddr=inet_pton(\"%s\")"
	       ", ipv6mr_interface=if_nametoindex(\"lo\")}, 20) = -1 %s (%m)\n",
	       multi6addr, errno2name());

	assert(setsockopt(0, SOL_IPV6, IPV6_LEAVE_ANYCAST, &m6, sizeof(m6)) == -1);
	printf("setsockopt(0, SOL_IPV6, IPV6_LEAVE_ANYCAST"
	       ", {ipv6mr_multiaddr=inet_pton(\"%s\")"
	       ", ipv6mr_interface=if_nametoindex(\"lo\")}, 20) = -1 %s (%m)\n",
	       multi6addr, errno2name());

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("IP_ADD_MEMBERSHIP && IPV6_ADD_MEMBERSHIP"
		    " && IPV6_JOIN_ANYCAST && HAVE_IF_INDEXTONAME")

#endif
