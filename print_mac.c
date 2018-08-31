/*
 * Copyright (c) 2018 The strace developers.
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

#include "error_prints.h"
#include "xstring.h"

#define XLAT_MACROS_ONLY
# include "xlat/arp_hardware_types.h"
#undef XLAT_MACROS_ONLY

const char *
sprint_mac_addr(const uint8_t addr[], size_t size)
{
	static char res[MAX_ADDR_LEN * 3];

	if (size > MAX_ADDR_LEN) {
		error_func_msg("Address size (%zu) is more than maximum "
			       "supported (%u)", size, MAX_ADDR_LEN);

		return NULL;
	}

	char *ptr = res;

	for (size_t i = 0; i < size; i++)
		ptr = xappendstr(res, ptr, "%s%02x", i ? ":" : "", addr[i]);

	return res;
}

const char *
sprint_hwaddr(const uint8_t hwaddr[], size_t size, uint32_t devtype)
{
	static uint8_t hwaddr_sizes[] = {
		[0 ... ARPHRD_IEEE802_TR] = 255,

		[ARPHRD_NETROM]     =  7 /* AX25_ADDR_LEN */,
		[ARPHRD_ETHER]      =  6 /* ETH_ALEN */,
		/* ARPHRD_EETHER - no actual devices in Linux */
		[ARPHRD_AX25]       =  7 /* AX25_ADDR_LEN */,
		/* ARPHRD_PRONET - no actual devices in Linux */
		/* ARPHRD_CHAOS - no actual devices in Linux */
		[ARPHRD_IEEE802]    =  6 /* FC_ALEN */,
		[ARPHRD_ARCNET]     =  1 /* ARCNET_ALEN */,
		/* ARPHRD_APPLETLK - no actual devices in Linux */
		[ARPHRD_DLCI]       = sizeof(short),
		/* ARPHRD_ATM - no explicit setting */
		/* ARPHRD_METRICOM - no actual devices in Linux */
		[ARPHRD_IEEE1394]   = 16 /* FWNET_ALEN */,
		[ARPHRD_EUI64]      =  8 /* EUI64_ADDR_LEN */,
		[ARPHRD_INFINIBAND] = 20 /* INFINIBAND_ALEN */,
		[ARPHRD_SLIP]       =  0,
		/* ARPHRD_CSLIP - no actual devices in Linux */
		/* ARPHRD_SLIP6 - no actual devices in Linux */
		/* ARPHRD_CSLIP6 - no actual devices in Linux */
		/* ARPHRD_RSRVD - no actual devices in Linux */
		/* ARPHRD_ADAPT - no actual devices in Linux */
		[ARPHRD_ROSE]       =  5 /* ROSE_ADDR_LEN */,
		[ARPHRD_X25]        =  0,
		/* ARPHRD_HWX25 - no actual devices in Linux */
		[ARPHRD_CAN]        =  0,
		[ARPHRD_PPP]        =  0,
		/* ARPHRD_CISCO - no actual devices in Linux */
		/* ARPHRD_LAPB - no actual devices in Linux */
		/* ARPHRD_DDCMP - no actual devices in Linux */
		[ARPHRD_RAWHDLC]    =  0,
		[ARPHRD_RAWIP]      =  0,
		[ARPHRD_TUNNEL]     =  4 /* IPIP */,
		[ARPHRD_TUNNEL6]    = 16 /* sizeof(struct in6_addr) */,
		/* ARPHRD_FRAD - no actual devices in Linux */
		/* ARPHRD_SKIP - no actual devices in Linux */
		[ARPHRD_LOOPBACK]   =  6 /* ETH_ALEN */,
		[ARPHRD_LOCALTLK]   =  1 /* LTALK_ALEN */,
		[ARPHRD_FDDI]       =  6 /* FDDI_K_ALEN */,
		/* ARPHRD_BIF - no actual devices in Linux */
		[ARPHRD_SIT]        =  4,
		[ARPHRD_IPDDP]      =  0,
		[ARPHRD_IPGRE]      =  4,
		[ARPHRD_PIMREG]     =  0,
		[ARPHRD_HIPPI]      =  6 /* HIPPI_ALEN */,
		/* ARPHRD_ASH - no actual devices in Linux */
		/* ARPHRD_ECONET - no actual devices in Linux */
		[ARPHRD_IRDA]       =  4 /* LAP_ALEN */,
		/* ARPHRD_FCPP - no actual devices in Linux */
		/* ARPHRD_FCAL - no actual devices in Linux */
		/* ARPHRD_FCPL - no actual devices in Linux */
		/* ARPHRD_FCFABRIC - no actual devices in Linux */
		/* ARPHRD_IEEE802_TR - no actual devices in Linux */
		[ARPHRD_IEEE80211]  =  6 /* ETH_ALEN */,
		[ARPHRD_IEEE80211_PRISM] = 6 /* ETH_ALEN */,
		[ARPHRD_IEEE80211_RADIOTAP] = 6 /* ETH_ALEN */,
		[ARPHRD_IEEE802154]
			= 8 /* IEEE802154_EXTENDED_ADDR_LEN */,
		[ARPHRD_IEEE802154_MONITOR]
			= 8 /* IEEE802154_EXTENDED_ADDR_LEN */,
		[ARPHRD_PHONET]     =  1,
		[ARPHRD_PHONET_PIPE] = 1,
		[ARPHRD_CAIF]       =  0,
		[ARPHRD_IP6GRE]     = 16 /* sizeof(struct in6_addr) */,
		[ARPHRD_NETLINK]    =  0,
		[ARPHRD_6LOWPAN]    =  8 /* EUI64_ADDR_LEN */
			/* ^ or ETH_ALEN, depending on lltype */,
		[ARPHRD_VSOCKMON]   =  0,
	};

	uint8_t sz = (devtype < ARRAY_SIZE(hwaddr_sizes))
			? hwaddr_sizes[devtype] : 255;

	return sprint_mac_addr(hwaddr, MIN(size, sz));
}
