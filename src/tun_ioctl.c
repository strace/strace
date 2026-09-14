/*
 * Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <net/if.h>
#include <linux/ioctl.h>
#include <linux/if_tun.h>

#define TUN_IOC(cmd_)	((cmd_) & ~(_IOC_SIZEMASK << _IOC_SIZESHIFT))

#include "xlat/tun_filter_flags.h"
#include "xlat/tun_ifr_flags.h"
#include "xlat/tun_offload_features.h"

static bool
print_tun_filter_addr(struct tcb *tcp, void *elem_buf,
		      size_t elem_size, void *opaque_data)
{
	print_mac_addr("", elem_buf, elem_size);
	return true;
}

static void
print_tun_filter(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct tun_filter tf;

	if (umove_or_printaddr(tcp, arg, &tf))
		return;

	tprint_struct_begin();
	PRINT_FIELD_FLAGS(tf, flags, tun_filter_flags, "TUN_FLT_???");
	tprint_struct_next();
	PRINT_FIELD_U(tf, count);

	if (tf.count) {
		uint8_t addr[ETH_ALEN];

		tprint_struct_next();
		tprints_field_name("addr");
		print_array(tcp, arg + sizeof(tf), tf.count,
			    addr, sizeof(addr), tfetch_mem,
			    print_tun_filter_addr, NULL);
	}

	tprint_struct_end();
}

static void
print_ifreq(struct tcb *const tcp, const kernel_ulong_t arg)
{
	struct ifreq ifr;

	if (umoven_or_printaddr(tcp, arg, get_ifreq_size(), &ifr))
		return;

	tprint_struct_begin();
	PRINT_FIELD_CSTRING(ifr, ifr_name);
	tprint_struct_next();
	PRINT_FIELD_FLAGS(ifr, ifr_flags, tun_ifr_flags, "IFF_???");
	tprint_struct_end();
}

int
tun_ioctl(struct tcb *const tcp, const unsigned int code,
	  kernel_ulong_t arg)
{
	arg = truncate_kulong_to_current_wordsize(arg);

	switch (code) {
	/* no-argument ioctl, returns fd */
	case TUNGETDEVNETNS:
		return RVAL_IOCTL_DECODED | RVAL_FD;

	/* by-value ioctls */
	case TUNSETNOCSUM:
	case TUNSETPERSIST:
		tprints_arg_next_name("argp");
		PRINT_VAL_U(arg);
		return RVAL_IOCTL_DECODED;

	case TUNSETDEBUG:
		tprints_arg_next_name("argp");
		PRINT_VAL_X((unsigned int) arg);
		return RVAL_IOCTL_DECODED;

	case TUNSETOWNER:
	case TUNSETGROUP:
		tprints_arg_next_name("argp");
		printuid(arg);
		return RVAL_IOCTL_DECODED;

	case TUNSETLINK:
		tprints_arg_next_name("argp");
		printxval(arp_hardware_types, arg, "ARPHRD_???");
		return RVAL_IOCTL_DECODED;

	case TUNSETOFFLOAD:
		tprints_arg_next_name("argp");
		printflags(tun_offload_features, arg, "TUN_F_???");
		return RVAL_IOCTL_DECODED;

	/* pointer-to-int write ioctls */
	case TUNSETSNDBUF:
	case TUNSETVNETHDRSZ:
	case TUNSETVNETLE:
	case TUNSETVNETBE:
	case TUNSETCARRIER:
		tprints_arg_next_name("argp");
		printnum_int(tcp, arg, "%d");
		return RVAL_IOCTL_DECODED;

	/* pointer-to-fd write ioctls */
	case TUNSETSTEERINGEBPF:
	case TUNSETFILTEREBPF:
		tprints_arg_next_name("argp");
		printnum_fd(tcp, arg);
		return RVAL_IOCTL_DECODED;

	/* pointer-to-ifindex write ioctl */
	case TUNSETIFINDEX: {
		int ifindex;

		tprints_arg_next_name("argp");
		if (!umove_or_printaddr(tcp, arg, &ifindex)) {
			tprint_indirect_begin();
			print_ifindex(ifindex);
			tprint_indirect_end();
		}
		return RVAL_IOCTL_DECODED;
	}

	/* pointer-to-int read ioctls */
	case TUNGETSNDBUF:
	case TUNGETVNETHDRSZ:
	case TUNGETVNETLE:
	case TUNGETVNETBE:
		if (entering(tcp))
			return 0;
		tprints_arg_next_name("argp");
		printnum_int(tcp, arg, "%d");
		return RVAL_IOCTL_DECODED;

	/* pointer-to-uint read ioctl (flags) */
	case TUNGETFEATURES:
		if (entering(tcp)) {
			return 0;
		} else {
			unsigned int features;

			tprints_arg_next_name("argp");
			if (!umove_or_printaddr(tcp, arg, &features)) {
				tprint_indirect_begin();
				printflags(tun_ifr_flags, features, "IFF_???");
				tprint_indirect_end();
			}
		}
		return RVAL_IOCTL_DECODED;

	/* struct tun_filter ioctl */
	case TUNSETTXFILTER:
		tprints_arg_next_name("argp");
		print_tun_filter(tcp, arg);
		return RVAL_IOCTL_DECODED;

	/* struct ifreq ioctls */
	case TUNGETIFF:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;

	case TUNSETIFF:
	case TUNSETQUEUE:
		tprints_arg_next_name("ifr");
		print_ifreq(tcp, arg);
		return RVAL_IOCTL_DECODED;

	default:
		switch (TUN_IOC(code)) {
		/* sock_fprog ioctls */
		case TUN_IOC(TUNGETFILTER):
			if (entering(tcp))
				return 0;
			ATTRIBUTE_FALLTHROUGH;

		case TUN_IOC(TUNATTACHFILTER):
		case TUN_IOC(TUNDETACHFILTER):
			tprints_arg_next_name("argp");
			decode_sock_fprog(tcp, arg);
			return RVAL_IOCTL_DECODED;

		default:
			return RVAL_DECODED;
		}
	}
}
