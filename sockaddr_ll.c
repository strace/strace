/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>

#include "xlat/arp_hardware_types.h"
#include "xlat/ethernet_protocols.h"
#include "xlat/af_packet_types.h"

void
print_sockaddr_data_ll(const void *const buf, const int addrlen)
{
	const struct sockaddr_ll *const sa_ll = buf;

	tprints("sll_protocol=htons(");
	printxval(ethernet_protocols, ntohs(sa_ll->sll_protocol), "ETH_P_???");
	tprints("), sll_ifindex=");
	print_ifindex(sa_ll->sll_ifindex);
	tprints(", sll_hatype=");
	printxval(arp_hardware_types, sa_ll->sll_hatype, "ARPHRD_???");
	tprints(", sll_pkttype=");
	printxval(af_packet_types, sa_ll->sll_pkttype, "PACKET_???");
	tprintf(", sll_halen=%u", sa_ll->sll_halen);
	if (sa_ll->sll_halen) {
		const unsigned int oob_halen =
			addrlen - offsetof(struct sockaddr_ll, sll_addr);
		unsigned int i;

		tprints(", sll_addr=[");
		for (i = 0; i < sa_ll->sll_halen; ++i) {
			if (i)
				tprints(", ");
			if (i >= oob_halen) {
				tprints("...");
				break;
			}
			tprintf("%#02x", sa_ll->sll_addr[i]);
		}
		tprints("]");
	}
}
