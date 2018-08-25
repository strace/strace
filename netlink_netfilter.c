/*
 * Copyright (c) 2018 Chen Jingpiao <chenjingpiao@gmail.com>
 * Copyright (c) 2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_LINUX_NETFILTER_NFNETLINK_H

# include "print_fields.h"
# include "nlattr.h"

# include <netinet/in.h>
# include <arpa/inet.h>
# include "netlink.h"
# include <linux/netfilter/nfnetlink.h>

# include "xlat/netfilter_versions.h"
# include "xlat/nf_acct_attr_names.h"
# include "xlat/nf_cthelper_attr_names.h"
# include "xlat/nf_ctnetlink_attr_names.h"
# include "xlat/nf_ctnetlink_exp_attr_names.h"
# include "xlat/nf_ctnetlink_to_attr_names.h"
# include "xlat/nf_ipset_attr_names.h"
# include "xlat/nf_nft_compat_attr_names.h"
# include "xlat/nf_osf_attr_names.h"
# include "xlat/nf_queue_attr_names.h"
# include "xlat/nf_ulog_attr_names.h"
# include "xlat/nft_chain_attr_names.h"
# include "xlat/nft_flow_attr_names.h"
# include "xlat/nft_gen_attr_names.h"
# include "xlat/nft_obj_attr_names.h"
# include "xlat/nft_rule_attr_names.h"
# include "xlat/nft_set_attr_names.h"
# include "xlat/nft_setelem_attr_names.h"
# include "xlat/nft_table_attr_names.h"
# include "xlat/nft_trace_attr_names.h"
# include "xlat/nl_netfilter_msg_types.h"
# include "xlat/nl_netfilter_subsys_ids.h"

# define XLAT_MACROS_ONLY
#  include "xlat/nl_netfilter_subsys_ids.h"
#  include "xlat/nf_nftables_msg_types.h"
# undef XLAT_MACROS_ONLY

struct nfnl_decoder {
	const struct xlat *name_xlat;
	const char *dflt;

	const nla_decoder_t *decoders;
	size_t decoders_sz;

	const struct nfnl_decoder *subdecoder;
	size_t subdecoder_sz;
};

static const struct nfnl_decoder nft_subsystem_decoders[] = {
	[NFT_MSG_NEWTABLE] =
		{ nft_table_attr_names,		"NFTA_TABLE_???", },
	[NFT_MSG_GETTABLE] =
		{ nft_table_attr_names,		"NFTA_TABLE_???", },
	[NFT_MSG_DELTABLE] =
		{ nft_table_attr_names,		"NFTA_TABLE_???", },
	[NFT_MSG_NEWCHAIN] =
		{ nft_chain_attr_names,		"NFTA_CHAIN_???", },
	[NFT_MSG_GETCHAIN] =
		{ nft_chain_attr_names,		"NFTA_CHAIN_???", },
	[NFT_MSG_DELCHAIN] =
		{ nft_chain_attr_names,		"NFTA_CHAIN_???", },
	[NFT_MSG_NEWRULE] =
		{ nft_rule_attr_names,		"NFTA_RULE_???", },
	[NFT_MSG_GETRULE] =
		{ nft_rule_attr_names,		"NFTA_RULE_???", },
	[NFT_MSG_DELRULE] =
		{ nft_rule_attr_names,		"NFTA_RULE_???", },
	[NFT_MSG_NEWSET] =
		{ nft_set_attr_names,		"NFTA_SET_???", },
	[NFT_MSG_GETSET] =
		{ nft_set_attr_names,		"NFTA_SET_???", },
	[NFT_MSG_DELSET] =
		{ nft_set_attr_names,		"NFTA_SET_???", },
	[NFT_MSG_NEWSETELEM] =
		{ nft_setelem_attr_names,	"NFTA_SET_ELEM_???", },
	[NFT_MSG_GETSETELEM] =
		{ nft_setelem_attr_names,	"NFTA_SET_ELEM_???", },
	[NFT_MSG_DELSETELEM] =
		{ nft_setelem_attr_names,	"NFTA_SET_ELEM_???", },
	[NFT_MSG_NEWGEN] =
		{ nft_gen_attr_names,		"NFTA_GEN_???", },
	[NFT_MSG_GETGEN] =
		{ nft_gen_attr_names,		"NFTA_GEN_???", },
	[NFT_MSG_TRACE] =
		{ nft_trace_attr_names,		"NFTA_TRACE_???", },
	[NFT_MSG_NEWOBJ] =
		{ nft_obj_attr_names,		"NFTA_OBJ_???", },
	[NFT_MSG_GETOBJ] =
		{ nft_obj_attr_names,		"NFTA_OBJ_???", },
	[NFT_MSG_DELOBJ] =
		{ nft_obj_attr_names,		"NFTA_OBJ_???", },
	[NFT_MSG_GETOBJ_RESET] =
		{ nft_obj_attr_names,		"NFTA_OBJ_???", },
	[NFT_MSG_NEWFLOWTABLE] =
		{ nft_flow_attr_names,		"NFTA_FLOW_???", },
	[NFT_MSG_GETFLOWTABLE] =
		{ nft_flow_attr_names,		"NFTA_FLOW_???", },
	[NFT_MSG_DELFLOWTABLE] =
		{ nft_flow_attr_names,		"NFTA_FLOW_???", },
};

static const struct nfnl_decoder nfnl_subsystems[] = {
	[NFNL_SUBSYS_CTNETLINK] =
		{ nf_ctnetlink_attr_names,	"CTA_???", },
	[NFNL_SUBSYS_CTNETLINK_EXP] =
		{ nf_ctnetlink_exp_attr_names,	"CTA_EXPECT_???", },
	[NFNL_SUBSYS_QUEUE] =
		{ nf_queue_attr_names,		"NFQA_???", },
	[NFNL_SUBSYS_ULOG] =
		{ nf_ulog_attr_names,		"NFULA_???", },
	[NFNL_SUBSYS_OSF] =
		{ nf_osf_attr_names,		"OSF_???", },
	[NFNL_SUBSYS_IPSET] =
		{ nf_ipset_attr_names,		"IPSET_ATTR_???", },
	[NFNL_SUBSYS_ACCT] =
		{ nf_acct_attr_names,		"NFACCT_???", },
	[NFNL_SUBSYS_CTNETLINK_TIMEOUT] =
		{ nf_ctnetlink_to_attr_names,	"CTA_TIMEOUT_???", },
	[NFNL_SUBSYS_CTHELPER] =
		{ nf_cthelper_attr_names,	"NFCTH_???" },
	[NFNL_SUBSYS_NFTABLES] =
		{ NULL, "NFT_???", NULL, 0,
		  ARRSZ_PAIR(nft_subsystem_decoders) },
	[NFNL_SUBSYS_NFT_COMPAT] =
		{ nf_nft_compat_attr_names,	"NFTA_COMPAT_???" },
};

bool
decode_netlink_netfilter(struct tcb *const tcp,
			 const struct nlmsghdr *const nlmsghdr,
			 const kernel_ulong_t addr,
			 const unsigned int len)
{
	if (nlmsghdr->nlmsg_type == NLMSG_DONE)
		return false;

	struct nfgenmsg nfmsg;

	if (len < sizeof(nfmsg))
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
	else if (!umove_or_printaddr(tcp, addr, &nfmsg)) {
		const uint8_t subsys_id = nlmsghdr->nlmsg_type >> 8;
		const uint8_t msg_type = nlmsghdr->nlmsg_type;
		uint16_t res_id = ntohs(nfmsg.res_id);

		PRINT_FIELD_XVAL("{", nfmsg, nfgen_family, addrfams, "AF_???");
		PRINT_FIELD_XVAL(", ", nfmsg, version, netfilter_versions,
				 "NFNETLINK_???");

		/*
		 * Work around wrong endianness in res_id field,
		 * see linux commit v4.3-rc1~28^2~47^2~1
		 */
		tprints(", res_id=");
		if (subsys_id == NFNL_SUBSYS_NFTABLES
		    && res_id == NFNL_SUBSYS_NFTABLES) {
			print_xlat_ex(nfmsg.res_id,
				      "htons(NFNL_SUBSYS_NFTABLES)",
				      XLAT_STYLE_DEFAULT);
		} else if (subsys_id == NFNL_SUBSYS_NFTABLES
			   && nfmsg.res_id == NFNL_SUBSYS_NFTABLES) {
			print_xlat_ex(nfmsg.res_id, "NFNL_SUBSYS_NFTABLES",
				      XLAT_STYLE_DEFAULT);
		} else {
			tprintf("htons(%d)", res_id);
		}

		const size_t offset = NLMSG_ALIGN(sizeof(nfmsg));
		if (len > offset) {
			tprints(", ");
			if ((nlmsghdr->nlmsg_type >= NFNL_MSG_BATCH_BEGIN
			     && nlmsghdr->nlmsg_type <= NFNL_MSG_BATCH_END)
			    || nlmsghdr->nlmsg_type < NLMSG_MIN_TYPE) {
				printstr_ex(tcp, addr + offset,
					    len - offset, QUOTE_FORCE_HEX);
			} else {
				static const struct nfnl_decoder def;
				const struct nfnl_decoder *subsys = &def;

				if (subsys_id < ARRAY_SIZE(nfnl_subsystems))
					subsys = nfnl_subsystems + subsys_id;
				if (subsys->subdecoder
				    && (msg_type < subsys->subdecoder_sz))
					subsys = subsys->subdecoder + msg_type;

				decode_nlattr(tcp, addr + offset, len - offset,
					      subsys->name_xlat, subsys->dflt,
					      subsys->decoders,
					      subsys->decoders_sz, NULL);
			}
		}
	}

	return true;
}

#endif /* HAVE_LINUX_NETFILTER_NFNETLINK_H */
