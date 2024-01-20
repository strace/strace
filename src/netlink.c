/*
 * Copyright (c) 2016 Fabien Siron <fabien.siron@epita.fr>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "netlink.h"
#include "nlattr.h"
#include <linux/audit.h>
#include <linux/rtnetlink.h>
#include <linux/xfrm.h>
#include <linux/cryptouser.h>
#include <linux/netfilter/ipset/ip_set.h>
#include <linux/netfilter/nf_tables.h>
#include <linux/netfilter/nf_tables_compat.h>
#include <linux/netfilter/nfnetlink.h>
#include <linux/netfilter/nfnetlink_acct.h>
#include <linux/netfilter/nfnetlink_conntrack.h>
#include <linux/netfilter/nfnetlink_cthelper.h>
#include <linux/netfilter/nfnetlink_cttimeout.h>
#include <linux/netfilter/nfnetlink_hook.h>
#include <linux/netfilter/nfnetlink_log.h>
#include <linux/netfilter/nfnetlink_queue.h>
#include <linux/netfilter/nfnetlink_osf.h>
#include "xlat/netlink_ack_flags.h"
#include "xlat/netlink_delete_flags.h"
#include "xlat/netlink_flags.h"
#include "xlat/netlink_get_flags.h"
#include "xlat/netlink_new_flags.h"
#include "xlat/netlink_protocols.h"
#include "xlat/netlink_types.h"
#include "xlat/nf_acct_msg_types.h"
#include "xlat/nf_cthelper_msg_types.h"
#include "xlat/nf_ctnetlink_exp_msg_types.h"
#include "xlat/nf_ctnetlink_msg_types.h"
#include "xlat/nf_cttimeout_msg_types.h"
#include "xlat/nf_hook_msg_types.h"
#include "xlat/nf_ipset_msg_types.h"
#include "xlat/nf_nft_compat_msg_types.h"
#include "xlat/nf_nftables_msg_types.h"
#include "xlat/nf_osf_msg_types.h"
#include "xlat/nf_queue_msg_types.h"
#include "xlat/nf_ulog_msg_types.h"
#include "xlat/nl_audit_types.h"
#include "xlat/nl_crypto_types.h"
#include "xlat/nl_netfilter_subsys_ids.h"
#include "xlat/nl_selinux_types.h"
#include "xlat/nl_sock_diag_types.h"
#include "xlat/nl_xfrm_types.h"
#include "xlat/nlmsgerr_attrs.h"

/*
 * Fetch a struct nlmsghdr from the given address.
 */
static bool
fetch_nlmsghdr(struct tcb *const tcp, struct nlmsghdr *const nlmsghdr,
	       const kernel_ulong_t addr, const kernel_ulong_t len,
	       const bool in_array)
{
	if (len < sizeof(struct nlmsghdr)) {
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		return false;
	}

	if (tfetch_obj(tcp, addr, nlmsghdr))
		return true;

	if (in_array) {
		tprint_more_data_follows();
		printaddr_comment(addr);
	} else {
		printaddr(addr);
	}

	return false;
}

static int
get_fd_nl_family(struct tcb *const tcp, const int fd)
{
	const unsigned long inode = getfdinode(tcp, fd);
	if (!inode)
		return -1;

	const char *const details = get_sockaddr_by_inode(tcp, fd, inode);
	if (!details)
		return -1;

	const char *const nl_details = STR_STRIP_PREFIX(details, "NETLINK:[");
	if (nl_details == details)
		return -1;

	const struct xlat_data *xlats = netlink_protocols->data;
	for (uint32_t idx = 0; idx < netlink_protocols->size; idx++) {
		if (!netlink_protocols->data[idx].str)
			continue;

		const char *name = STR_STRIP_PREFIX(xlats[idx].str, "NETLINK_");
		if (!strncmp(nl_details, name, strlen(name)))
			return xlats[idx].val;
	}

	if (*nl_details >= '0' && *nl_details <= '9')
		return atoi(nl_details);

	return -1;
}

static void
decode_nlmsg_type_default(struct tcb *tcp, const struct xlat *const xlat,
			  const uint16_t type,
			  const char *const dflt)
{
	printxval(xlat, type, dflt);
}

static void
decode_nlmsg_type_generic(struct tcb *tcp, const struct xlat *const xlat,
			  const uint16_t type,
			  const char *const dflt)
{
	printxval(genl_families_xlat(tcp), type, dflt);
}

static const struct {
	const struct xlat *const xlat;
	const char *const dflt;
} nf_nlmsg_types[] = {
	[NFNL_SUBSYS_CTNETLINK] = {
		nf_ctnetlink_msg_types,
		"IPCTNL_MSG_CT_???"
	},
	[NFNL_SUBSYS_CTNETLINK_EXP] = {
		nf_ctnetlink_exp_msg_types,
		"IPCTNL_MSG_EXP_???"
	},
	[NFNL_SUBSYS_QUEUE] = { nf_queue_msg_types, "NFQNL_MSG_???" },
	[NFNL_SUBSYS_ULOG] = { nf_ulog_msg_types, "NFULNL_MSG_???" },
	[NFNL_SUBSYS_OSF] = { nf_osf_msg_types, "OSF_MSG_???" },
	[NFNL_SUBSYS_IPSET] = { nf_ipset_msg_types, "IPSET_CMD_???" },
	[NFNL_SUBSYS_ACCT] = { nf_acct_msg_types, "NFNL_MSG_ACCT_???" },
	[NFNL_SUBSYS_CTNETLINK_TIMEOUT] = {
		nf_cttimeout_msg_types,
		"IPCTNL_MSG_TIMEOUT_???"
	},
	[NFNL_SUBSYS_CTHELPER] = {
		nf_cthelper_msg_types,
		"NFNL_MSG_CTHELPER_???"
	},
	[NFNL_SUBSYS_NFTABLES] = { nf_nftables_msg_types, "NFT_MSG_???" },
	[NFNL_SUBSYS_NFT_COMPAT] = {
		nf_nft_compat_msg_types,
		"NFNL_MSG_COMPAT_???"
	},
	[NFNL_SUBSYS_HOOK] = { nf_hook_msg_types, "NFT_MSG_HOOK_???" },
};

static void
decode_nlmsg_type_netfilter(struct tcb *tcp, const struct xlat *const xlat,
			    const uint16_t type,
			    const char *const dflt)
{
	/* Reserved control nfnetlink messages first. */
	const char *const text = xlookup(nl_netfilter_msg_types, type);
	if (text) {
		print_xlat_ex(type, text, XLAT_STYLE_DEFAULT);
		return;
	}

	/*
	 * Other netfilter message types are split
	 * in two pieces: 8 bits subsystem and 8 bits type.
	 */
	const uint8_t subsys_id = (uint8_t) (type >> 8);
	const uint8_t msg_type = (uint8_t) type;

	tprint_flags_begin();
	tprint_shift_begin();
	printxval(xlat, subsys_id, dflt);
	tprint_shift();
	PRINT_VAL_U(8);
	tprint_shift_end();

	tprint_flags_or();
	if (subsys_id < ARRAY_SIZE(nf_nlmsg_types))
		printxval(nf_nlmsg_types[subsys_id].xlat,
			  msg_type, nf_nlmsg_types[subsys_id].dflt);
	else
		PRINT_VAL_X(msg_type);
	tprint_flags_end();
}

typedef void (*nlmsg_types_decoder_t)(struct tcb *, const struct xlat *,
				      uint16_t type,
				      const char *dflt);

static const struct {
	const nlmsg_types_decoder_t decoder;
	const struct xlat *const xlat;
	const char *const dflt;
} nlmsg_types[] = {
	[NETLINK_AUDIT] = { NULL, nl_audit_types, "AUDIT_???" },
	[NETLINK_CRYPTO] = { NULL, nl_crypto_types, "CRYPTO_MSG_???" },
	[NETLINK_GENERIC] = {
		decode_nlmsg_type_generic,
		NULL,
		"GENERIC_FAMILY_???"
	},
	[NETLINK_NETFILTER] = {
		decode_nlmsg_type_netfilter,
		nl_netfilter_subsys_ids,
		"NFNL_SUBSYS_???"
	},
	[NETLINK_ROUTE] = { NULL, nl_route_types, "RTM_???" },
	[NETLINK_SELINUX] = { NULL, nl_selinux_types, "SELNL_MSG_???" },
	[NETLINK_SOCK_DIAG] = { NULL, nl_sock_diag_types, "SOCK_DIAG_???" },
	[NETLINK_XFRM] = { NULL, nl_xfrm_types, "XFRM_MSG_???" }
};

/*
 * As all valid netlink families are positive integers, use unsigned int
 * for family here to filter out -1.
 */
static void
decode_nlmsg_type(struct tcb *tcp, const uint16_t type,
		  const unsigned int family)
{
	nlmsg_types_decoder_t decoder = decode_nlmsg_type_default;
	const struct xlat *xlat = netlink_types;
	const char *dflt = "NLMSG_???";

	/*
	 * type < NLMSG_MIN_TYPE are reserved control messages
	 * that need no family-specific decoding.
	 */
	if (type >= NLMSG_MIN_TYPE && family < ARRAY_SIZE(nlmsg_types)) {
		if (nlmsg_types[family].decoder)
			decoder = nlmsg_types[family].decoder;
		if (nlmsg_types[family].xlat)
			xlat = nlmsg_types[family].xlat;
		if (nlmsg_types[family].dflt)
			dflt = nlmsg_types[family].dflt;
	}

	decoder(tcp, xlat, type, dflt);
}

static const struct xlat *
decode_nlmsg_flags_crypto(const uint16_t type)
{
	switch (type) {
	case CRYPTO_MSG_NEWALG:
		return netlink_new_flags;
	case CRYPTO_MSG_DELALG:
	case CRYPTO_MSG_DELRNG:
		return netlink_delete_flags;
	case CRYPTO_MSG_GETALG:
		return netlink_get_flags;
	}

	return NULL;
}

static const struct xlat *
decode_nlmsg_flags_netfilter(const uint16_t type)
{
	const uint8_t subsys_id = (uint8_t) (type >> 8);
	const uint8_t msg_type = (uint8_t) type;

	switch (subsys_id) {
	case NFNL_SUBSYS_CTNETLINK:
		switch (msg_type) {
		case IPCTNL_MSG_CT_NEW:
			return netlink_new_flags;
		case IPCTNL_MSG_CT_GET:
		case IPCTNL_MSG_CT_GET_CTRZERO:
		case IPCTNL_MSG_CT_GET_STATS_CPU:
		case IPCTNL_MSG_CT_GET_STATS:
		case IPCTNL_MSG_CT_GET_DYING:
		case IPCTNL_MSG_CT_GET_UNCONFIRMED:
			return netlink_get_flags;
		case IPCTNL_MSG_CT_DELETE:
			return netlink_delete_flags;
		}
		break;
	case NFNL_SUBSYS_CTNETLINK_EXP:
		switch (msg_type) {
		case IPCTNL_MSG_EXP_NEW:
			return netlink_new_flags;
		case IPCTNL_MSG_EXP_GET:
		case IPCTNL_MSG_EXP_GET_STATS_CPU:
			return netlink_get_flags;
		case IPCTNL_MSG_EXP_DELETE:
			return netlink_delete_flags;
		}
		break;
	case NFNL_SUBSYS_ACCT:
		switch (msg_type) {
		case NFNL_MSG_ACCT_NEW:
			return netlink_new_flags;
		case NFNL_MSG_ACCT_GET:
		case NFNL_MSG_ACCT_GET_CTRZERO:
			return netlink_get_flags;
		case NFNL_MSG_ACCT_DEL:
			return netlink_delete_flags;
		}
		break;
	case NFNL_SUBSYS_CTNETLINK_TIMEOUT:
		switch (msg_type) {
		case IPCTNL_MSG_TIMEOUT_NEW:
			return netlink_new_flags;
		case IPCTNL_MSG_TIMEOUT_GET:
			return netlink_get_flags;
		case IPCTNL_MSG_TIMEOUT_DELETE:
			return netlink_delete_flags;
		}
		break;
	case NFNL_SUBSYS_CTHELPER:
		switch (msg_type) {
		case NFNL_MSG_CTHELPER_NEW:
			return netlink_new_flags;
		case NFNL_MSG_CTHELPER_GET:
			return netlink_get_flags;
		case NFNL_MSG_CTHELPER_DEL:
			return netlink_delete_flags;
		}
		break;
	case NFNL_SUBSYS_NFTABLES:
		switch (msg_type) {
		case NFT_MSG_NEWTABLE:
		case NFT_MSG_NEWCHAIN:
		case NFT_MSG_NEWRULE:
		case NFT_MSG_NEWSET:
		case NFT_MSG_NEWSETELEM:
		case NFT_MSG_NEWGEN:
		case NFT_MSG_NEWOBJ:
		case NFT_MSG_NEWFLOWTABLE:
			return netlink_new_flags;
		case NFT_MSG_GETTABLE:
		case NFT_MSG_GETCHAIN:
		case NFT_MSG_GETRULE:
		case NFT_MSG_GETSET:
		case NFT_MSG_GETSETELEM:
		case NFT_MSG_GETGEN:
		case NFT_MSG_GETOBJ:
		case NFT_MSG_GETOBJ_RESET:
		case NFT_MSG_GETFLOWTABLE:
		case NFT_MSG_GETRULE_RESET:
		case NFT_MSG_GETSETELEM_RESET:
			return netlink_get_flags;
		case NFT_MSG_DELTABLE:
		case NFT_MSG_DELCHAIN:
		case NFT_MSG_DELRULE:
		case NFT_MSG_DELSET:
		case NFT_MSG_DELSETELEM:
		case NFT_MSG_DELOBJ:
		case NFT_MSG_DELFLOWTABLE:
		case NFT_MSG_DESTROYTABLE:
		case NFT_MSG_DESTROYCHAIN:
		case NFT_MSG_DESTROYRULE:
		case NFT_MSG_DESTROYSET:
		case NFT_MSG_DESTROYSETELEM:
		case NFT_MSG_DESTROYOBJ:
		case NFT_MSG_DESTROYFLOWTABLE:
			return netlink_delete_flags;
		}
		break;
	case NFNL_SUBSYS_NFT_COMPAT:
		switch (msg_type) {
		case NFNL_MSG_COMPAT_GET:
			return netlink_get_flags;
		}
		break;
	case NFNL_SUBSYS_HOOK:
		switch (msg_type) {
		case NFNL_MSG_HOOK_GET:
			return netlink_get_flags;
		}
		break;
	}

	return NULL;
}

static const struct xlat *
decode_nlmsg_flags_route(const uint16_t type)
{
	/* RTM_DELACTION uses NLM_F_ROOT flags */
	if (type == RTM_DELACTION)
		return netlink_get_flags;
	switch (type & 3) {
	case  0:
		return netlink_new_flags;
	case  1:
		return netlink_delete_flags;
	case  2:
		return netlink_get_flags;
	}

	return NULL;
}

static const struct xlat *
decode_nlmsg_flags_sock_diag(const uint16_t type)
{
	return netlink_get_flags;
}

static const struct xlat *
decode_nlmsg_flags_xfrm(const uint16_t type)
{
	switch (type) {
	case XFRM_MSG_NEWSA:
	case XFRM_MSG_NEWPOLICY:
	case XFRM_MSG_NEWAE:
	case XFRM_MSG_NEWSADINFO:
	case XFRM_MSG_NEWSPDINFO:
		return netlink_new_flags;
	case XFRM_MSG_DELSA:
	case XFRM_MSG_DELPOLICY:
		return netlink_delete_flags;
	case XFRM_MSG_GETSA:
	case XFRM_MSG_GETPOLICY:
	case XFRM_MSG_GETAE:
	case XFRM_MSG_GETSADINFO:
	case XFRM_MSG_GETSPDINFO:
		return netlink_get_flags;
	}

	return NULL;
}

typedef const struct xlat *(*nlmsg_flags_decoder_t)(const uint16_t type);

static const nlmsg_flags_decoder_t nlmsg_flags[] = {
	[NETLINK_CRYPTO] = decode_nlmsg_flags_crypto,
	[NETLINK_NETFILTER] = decode_nlmsg_flags_netfilter,
	[NETLINK_ROUTE] = decode_nlmsg_flags_route,
	[NETLINK_SOCK_DIAG] = decode_nlmsg_flags_sock_diag,
	[NETLINK_XFRM] = decode_nlmsg_flags_xfrm
};

/*
 * As all valid netlink families are positive integers, use unsigned int
 * for family here to filter out -1.
 */
static void
decode_nlmsg_flags(const uint16_t flags, const uint16_t type,
		   const unsigned int family)
{
	const struct xlat *table = NULL;

	if (type < NLMSG_MIN_TYPE) {
		if (type == NLMSG_ERROR)
			table = netlink_ack_flags;
	} else if (family < ARRAY_SIZE(nlmsg_flags) && nlmsg_flags[family])
		table = nlmsg_flags[family](type);

	tprint_flags_begin();
	printflags_ex(flags, "NLM_F_???", XLAT_STYLE_DEFAULT,
		      netlink_flags, table, NULL);
	tprint_flags_end();
}

static void
print_nlmsghdr(struct tcb *tcp,
	       const int fd,
	       const int family,
	       const struct nlmsghdr *const nlmsghdr)
{
	/* print the whole structure regardless of its nlmsg_len */

	tprint_struct_begin();
	PRINT_FIELD_U(*nlmsghdr, nlmsg_len);

	tprint_struct_next();
	PRINT_FIELD_OBJ_TCB_VAL(*nlmsghdr, nlmsg_type, tcp,
				decode_nlmsg_type, family);

	tprint_struct_next();
	PRINT_FIELD_OBJ_VAL(*nlmsghdr, nlmsg_flags, decode_nlmsg_flags,
			    nlmsghdr->nlmsg_type, family);

	tprint_struct_next();
	PRINT_FIELD_U(*nlmsghdr, nlmsg_seq);

	tprint_struct_next();
	PRINT_FIELD_TGID(*nlmsghdr, nlmsg_pid, tcp);
	tprint_struct_end();
}

static bool
print_cookie(struct tcb *const tcp, void *const elem_buf,
	     const size_t elem_size, void *const opaque_data)
{
	PRINT_VAL_U(*(uint8_t *) elem_buf);

	return true;
}

static bool
decode_nlmsgerr_attr_cookie(struct tcb *const tcp,
			    const kernel_ulong_t addr,
			    const unsigned int len,
			    const void *const opaque_data)
{
	uint8_t cookie;
	const size_t nmemb = len / sizeof(cookie);

	print_array(tcp, addr, nmemb, &cookie, sizeof(cookie),
		    tfetch_mem, print_cookie, 0);

	return true;
}

static const nla_decoder_t nlmsgerr_nla_decoders[] = {
	[NLMSGERR_ATTR_MSG]	= decode_nla_str,
	[NLMSGERR_ATTR_OFFS]	= decode_nla_u32,
	[NLMSGERR_ATTR_COOKIE]	= decode_nlmsgerr_attr_cookie
};

static void
decode_nlmsghdr_with_payload(struct tcb *const tcp,
			     const int fd,
			     const int family,
			     const struct nlmsghdr *const nlmsghdr,
			     const kernel_ulong_t addr,
			     const kernel_ulong_t len);

static void
decode_nlmsgerr(struct tcb *const tcp,
		const int fd,
		const int family,
		kernel_ulong_t addr,
		unsigned int len,
		const bool capped)
{
	struct nlmsgerr err;

	if (len < sizeof(err)) {
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
		return;
	}

	if (umove_or_printaddr(tcp, addr, &err))
		return;

	addr += offsetof(struct nlmsgerr, msg);
	len -= offsetof(struct nlmsgerr, msg);

	/*
	 * If err.msg.nlmsg_len < sizeof(err.msg), then it is
	 * invalid and sizeof(err.msg) would be used instead.
	 */
	const unsigned int nlmsg_len =
		MAX(sizeof(err.msg), err.msg.nlmsg_len);

	/*
	 * A valid nlmsg_len can exceed sizeof(err.msg),
	 * an invalid nlmsg_len can exceed len.
	 */
	const unsigned int payload =
		MIN(len, capped ? sizeof(err.msg) : nlmsg_len);

	if (len > payload)
		tprint_array_begin();

	tprint_struct_begin();

	PRINT_FIELD_ERR_D(err, error);
	tprint_struct_next();

	tprints_field_name("msg");
	decode_nlmsghdr_with_payload(tcp, fd, family,
				     &err.msg, addr, payload);

	tprint_struct_end();

	if (len > payload) {
		tprint_array_next();
		decode_nlattr(tcp, addr + payload,
			      len - payload, nlmsgerr_attrs,
			      "NLMSGERR_ATTR_???",
			      nlmsgerr_nla_decoders,
			      ARRAY_SIZE(nlmsgerr_nla_decoders),
			      NULL);
		tprint_array_end();
	}
}

static const netlink_decoder_t netlink_decoders[] = {
	[NETLINK_CRYPTO] = decode_netlink_crypto,
	[NETLINK_NETFILTER] = decode_netlink_netfilter,
	[NETLINK_ROUTE] = decode_netlink_route,
	[NETLINK_SELINUX] = decode_netlink_selinux,
	[NETLINK_SOCK_DIAG] = decode_netlink_sock_diag
};

static void
decode_payload(struct tcb *const tcp,
	       const int fd,
	       const int family,
	       const struct nlmsghdr *const nlmsghdr,
	       const kernel_ulong_t addr,
	       const unsigned int len)
{
	if (nlmsghdr->nlmsg_type == NLMSG_ERROR) {
		decode_nlmsgerr(tcp, fd, family, addr, len,
				nlmsghdr->nlmsg_flags & NLM_F_CAPPED);
		return;
	}

	/*
	 * While most of NLMSG_DONE messages indeed have payloads
	 * containing just a single integer, there are few exceptions,
	 * so pass payloads of NLMSG_DONE messages to family-specific
	 * netlink payload decoders.
	 *
	 * Other types of reserved control messages need no family-specific
	 * netlink payload decoding.
	 */
	if ((nlmsghdr->nlmsg_type >= NLMSG_MIN_TYPE
	    || nlmsghdr->nlmsg_type == NLMSG_DONE)
	    && (unsigned int) family < ARRAY_SIZE(netlink_decoders)
	    && netlink_decoders[family]
	    && netlink_decoders[family](tcp, nlmsghdr, addr, len)) {
		return;
	}

	if (nlmsghdr->nlmsg_type == NLMSG_DONE && len == sizeof(int)) {
		int num;

		if (!umove_or_printaddr(tcp, addr, &num))
			PRINT_VAL_D(num);
		return;
	}

	printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
}

static void
decode_nlmsghdr_with_payload(struct tcb *const tcp,
			     const int fd,
			     const int family,
			     const struct nlmsghdr *const nlmsghdr,
			     const kernel_ulong_t addr,
			     const kernel_ulong_t len)
{
	const unsigned int nlmsg_len = MIN(nlmsghdr->nlmsg_len, len);

	if (nlmsg_len > NLMSG_HDRLEN)
		tprint_array_begin();

	print_nlmsghdr(tcp, fd, family, nlmsghdr);

	if (nlmsg_len > NLMSG_HDRLEN) {
		tprint_array_next();
		decode_payload(tcp, fd, family, nlmsghdr, addr + NLMSG_HDRLEN,
						     nlmsg_len - NLMSG_HDRLEN);
		tprint_array_end();
	}
}

void
decode_netlink(struct tcb *const tcp,
	       const int fd,
	       kernel_ulong_t addr,
	       kernel_ulong_t len)
{
	const int family = get_fd_nl_family(tcp, fd);

	if (family == NETLINK_KOBJECT_UEVENT) {
		decode_netlink_kobject_uevent(tcp, addr, len);
		return;
	}

	struct nlmsghdr nlmsghdr;
	bool is_array = false;

	for (unsigned int elt = 0;
	     fetch_nlmsghdr(tcp, &nlmsghdr, addr, len, is_array);
	     ++elt) {
		if (abbrev(tcp) && elt == max_strlen) {
			tprint_more_data_follows();
			break;
		}

		unsigned int nlmsg_len = NLMSG_ALIGN(nlmsghdr.nlmsg_len);
		kernel_ulong_t next_addr = 0;
		kernel_ulong_t next_len = 0;

		if (nlmsghdr.nlmsg_len >= NLMSG_HDRLEN) {
			next_len = (len >= nlmsg_len) ? len - nlmsg_len : 0;

			if (next_len && addr + nlmsg_len > addr)
				next_addr = addr + nlmsg_len;
		}

		if (!is_array && next_addr) {
			tprint_array_begin();
			is_array = true;
		}

		decode_nlmsghdr_with_payload(tcp, fd, family,
					     &nlmsghdr, addr, len);

		if (!next_addr)
			break;

		tprint_array_next();
		addr = next_addr;
		len = next_len;
	}

	if (is_array) {
		tprint_array_end();
	}
}
