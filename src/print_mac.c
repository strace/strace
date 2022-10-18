/*
 * Copyright (c) 2018-2022 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include "error_prints.h"
#include "xstring.h"

#define XLAT_MACROS_ONLY
# include "xlat/arp_hardware_types.h"
#undef XLAT_MACROS_ONLY

DIAG_PUSH_IGNORE_OVERRIDE_INIT

static uint8_t hwaddr_sizes[] = {
	[0 ... ARPHRD_VSOCKMON] = 255,

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
	[ARPHRD_MCTP]       =  1,
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

DIAG_POP_IGNORE_OVERRIDE_INIT

static const char *
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

void
print_mac_addr(const char *prefix, const uint8_t addr[], size_t size)
{
	tprints_string(prefix);
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV
	    || size > MAX_ADDR_LEN)
		print_quoted_string((const char *) addr, size,
				    QUOTE_FORCE_HEX);
	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW
	    || size > MAX_ADDR_LEN)
		return;
	(xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE
		? tprints_comment : tprints_string)(sprint_mac_addr(addr, size));
}

static const char *
sprint_hwaddr(const uint8_t hwaddr[], size_t size, uint32_t devtype)
{
	uint8_t sz = (devtype < ARRAY_SIZE(hwaddr_sizes))
			? hwaddr_sizes[devtype] : 255;

	return sprint_mac_addr(hwaddr, MIN(size, sz));
}

void
print_hwaddr(const char *prefix, const uint8_t addr[], size_t size,
	     uint32_t devtype)
{
	tprints_string(prefix);
	if (xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV
	    || size > MAX_ADDR_LEN)
		print_quoted_string((const char *) addr, size,
				    QUOTE_FORCE_HEX);
	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW
	    || size > MAX_ADDR_LEN)
		return;
	(xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE
		? tprints_comment : tprints_string)(sprint_hwaddr(addr, size,
							   devtype));
}
