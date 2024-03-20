/*
 * Copyright (c) 2023 The strace developers.
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.1-or-later
 */

#include "defs.h"
#include "netlink.h"
#include "nlattr.h"
#include <linux/genetlink.h>

#include "netlink_generic.h"
#include "xlat/genl_flags.h"
#include "xlat/genl_nlctrl_types.h"
#include "xlat/genl_nlctrl_attrs.h"
#include "xlat/genl_nlctrl_ops_attrs.h"
#include "xlat/genl_nlctrl_mcast_attrs.h"
#include "xlat/genl_nlctrl_policy_attrs.h"

static void
decode_genl_family(struct tcb *const tcp, const uint8_t cmd,
		   const uint8_t version, const kernel_ulong_t addr,
		   const unsigned int len)
{
	tprint_struct_begin();
	tprints_field_name("cmd");
	printxval(NULL, cmd, "???");
	tprint_struct_next();
	tprints_field_name("version");
	tprintf_string("%d", version);

	if (len > 0) {
		tprint_struct_next();
		tprints_field_name("data");
		printstr_ex(tcp, addr, len, QUOTE_FORCE_HEX);
	}
	tprint_struct_end();
}

static bool
decode_genl_flags(struct tcb *const tcp,
			 const kernel_ulong_t addr,
			 const unsigned int len,
			 const void *const opaque_data)
{
	static const struct decode_nla_xlat_opts opts = {
		genl_flags, "GENL_???", .size = 4,
	};

	return decode_nla_flags(tcp, addr, len, &opts);
}

static const nla_decoder_t nlctrl_attr_ops_decoders[] = {
	[CTRL_ATTR_OP_UNSPEC] = NULL,
	[CTRL_ATTR_OP_ID] = decode_nla_u32,
	[CTRL_ATTR_OP_FLAGS] = decode_genl_flags
};

static bool
decode_nlctrl_op_item(struct tcb *const tcp,
		      const kernel_ulong_t addr,
		      const unsigned int len,
		      const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, genl_nlctrl_ops_attrs,
		      "CTRL_ATTR_OP_???",
		      ARRSZ_PAIR(nlctrl_attr_ops_decoders),
		      NULL);
	return true;
}

static bool
decode_nlctrl_ops(struct tcb *const tcp,
		 const kernel_ulong_t addr,
		 const unsigned int len,
		 const void *const opaque_data)
{
	nla_decoder_t op_decoder = &decode_nlctrl_op_item;
	decode_nlattr(tcp, addr, len, NULL, NULL,
		      &op_decoder, 0, NULL);
	return true;
}

static const nla_decoder_t nlctrl_attr_mcast_decoders[] = {
	[CTRL_ATTR_MCAST_GRP_UNSPEC] = NULL,
	[CTRL_ATTR_MCAST_GRP_NAME] = decode_nla_str,
	[CTRL_ATTR_MCAST_GRP_ID] = decode_nla_u32
};

static bool
decode_nlctrl_mcast_item(struct tcb *const tcp,
		 const kernel_ulong_t addr,
		 const unsigned int len,
		 const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, genl_nlctrl_mcast_attrs,
		      "CTRL_ATTR_MCAST_GRP_???",
		      ARRSZ_PAIR(nlctrl_attr_mcast_decoders),
		      NULL);
	return true;
}

static bool
decode_nlctrl_mcast(struct tcb *const tcp,
		    const kernel_ulong_t addr,
		    const unsigned int len,
		    const void *const opaque_data)
{
	nla_decoder_t mcast_decoder = &decode_nlctrl_mcast_item;
	decode_nlattr(tcp, addr, len, NULL, NULL,
		      &mcast_decoder, 0, NULL);
	return true;
}
static const nla_decoder_t nlctrl_attr_policy_decoders[] = {
	[CTRL_ATTR_POLICY_UNSPEC] = NULL,
	[CTRL_ATTR_POLICY_DO] = decode_nla_u32,
	[CTRL_ATTR_POLICY_DUMP] = decode_nla_u32
};

static bool
decode_nlctrl_policy_item(struct tcb *const tcp,
		 const kernel_ulong_t addr,
		 const unsigned int len,
		 const void *const opaque_data)
{
	decode_nlattr(tcp, addr, len, genl_nlctrl_policy_attrs,
		      "CTRL_ATTR_POLICY_???",
		      ARRSZ_PAIR(nlctrl_attr_policy_decoders),
		      NULL);
	return true;
}

static bool
decode_nlctrl_policy(struct tcb *const tcp,
		     const kernel_ulong_t addr,
		     const unsigned int len,
		     const void *const opaque_data)
{
	nla_decoder_t policy_decoder = &decode_nlctrl_policy_item;
	decode_nlattr(tcp, addr, len, NULL, NULL,
		      &policy_decoder, 0, NULL);
	return true;
}

static const nla_decoder_t nlctrl_attr_decoders[] = {
	[CTRL_ATTR_UNSPEC] = NULL,
	[CTRL_ATTR_FAMILY_ID] = decode_nla_u16,
	[CTRL_ATTR_FAMILY_NAME] = decode_nla_str,
	[CTRL_ATTR_VERSION] = decode_nla_u32,
	[CTRL_ATTR_HDRSIZE] = decode_nla_u32,
	[CTRL_ATTR_MAXATTR] = decode_nla_u32,
	[CTRL_ATTR_OPS] = decode_nlctrl_ops,
	[CTRL_ATTR_MCAST_GROUPS] = decode_nlctrl_mcast,
	[CTRL_ATTR_POLICY] = decode_nlctrl_policy,
	[CTRL_ATTR_OP_POLICY] = decode_nlctrl_policy,
	[CTRL_ATTR_OP] = decode_nla_u32,
};

DECL_NETLINK_GENERIC_DECODER(decode_nlctrl_msg) {
	tprint_struct_begin();
	PRINT_FIELD_XVAL(*genl, cmd, genl_nlctrl_types, "CTRL_CMD_???");
	tprint_struct_next();
	PRINT_FIELD_U(*genl, version);

	if (len > 0) {
		tprint_struct_next();
		decode_nlattr(tcp, addr, len,
			      genl_nlctrl_attrs,
			      "CTRL_ATTR_???",
			      ARRSZ_PAIR(nlctrl_attr_decoders),
			      NULL);
	}

	tprint_struct_end();
}

typedef DECL_NETLINK_GENERIC_DECODER((*netlink_generic_decoder_t));

struct genl_decoder_entry_t {
	char name[GENL_NAMSIZ];
	netlink_generic_decoder_t decoder;
} genl_decoders[] = {
	{ "nlctrl", decode_nlctrl_msg },
};

static netlink_generic_decoder_t
get_genl_decoder(const char *name)
{
	unsigned int i;

	if (!name)
		return NULL;

	for (i = 0; i < ARRAY_SIZE(genl_decoders); i++) {
		if (strcmp(genl_decoders[i].name, name) == 0) {
			return genl_decoders[i].decoder;
		}
	}
	return NULL;
}

bool
decode_netlink_generic(struct tcb *const tcp,
		       const struct nlmsghdr *const nlmsghdr,
		       const kernel_ulong_t addr,
		       const unsigned int len)
{
        struct genlmsghdr genl;

	if (nlmsghdr->nlmsg_type == NLMSG_DONE)
		return false;

	if (!umove_or_printaddr(tcp, addr, &genl)) {
		size_t offset =  sizeof(struct genlmsghdr);
		const unsigned int index = nlmsghdr->nlmsg_type;
		const struct xlat *families = genl_families_xlat(tcp);
		const char* name = xlookup(families, index);
		netlink_generic_decoder_t decoder = get_genl_decoder(name);
		if (decoder) {
			decoder(tcp, &genl, addr + offset, len - offset);
                } else {
			decode_genl_family(tcp, genl.cmd, genl.version,
					   addr + offset, len - offset);
                }
        }

	return true;
}
