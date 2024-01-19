/*
 * Copyright (c) 2022 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include DEF_MPERS_TYPE(struct_sysctl_args)
#include <linux/sysctl.h>
typedef struct __sysctl_args struct_sysctl_args;
#include MPERS_DEFS

SYS_FUNC(sysctl)
{
	struct_sysctl_args info;

	if (umove_or_printaddr(tcp, tcp->u_arg[0], &info))
		return RVAL_DECODED;

	size = sizeof(int) * (unsigned long) info.nlen;
	name = (size / sizeof(int) != (unsigned long) info.nlen) ? NULL : malloc(size);
	if (name == NULL ||
		umoven(tcp, (unsigned long) info.name, size, name) < 0) {
		free(name);
		if (entering(tcp))
			tprintf_string("{%p, %d, %p, %p, %p, %lu}",
				info.name, info.nlen, info.oldval,
				info.oldlenp, info.newval, (unsigned long)info.newlen);
		return RVAL_DECODED;
	}

	if (entering(tcp)) {
		unsigned int cnt = 0, max_cnt;

		tprints_string("{{");

		if (info.nlen == 0)
			goto out;
		printxval(sysctl_root, name[0], "CTL_???");
		++cnt;

		if (info.nlen == 1)
			goto out;
		switch (name[0]) {
		case CTL_KERN:
			tprint_array_next();
			printxval(sysctl_kern, name[1], "KERN_???");
			++cnt;
			break;
		case CTL_VM:
			tprint_array_next();
			printxval(sysctl_vm, name[1], "VM_???");
			++cnt;
			break;
		case CTL_NET:
			tprint_array_next();
			printxval(sysctl_net, name[1], "NET_???");
			++cnt;

			if (info.nlen == 2)
				goto out;
			switch (name[1]) {
			case NET_CORE:
				tprint_array_next();
				printxval(sysctl_net_core, name[2],
					"NET_CORE_???");
				break;
			case NET_UNIX:
				tprint_array_next();
				printxval(sysctl_net_unix, name[2],
					"NET_UNIX_???");
				break;
			case NET_IPV4:
				tprint_array_next();
				printxval(sysctl_net_ipv4, name[2],
					"NET_IPV4_???");

				if (info.nlen == 3)
					goto out;
				switch (name[2]) {
				case NET_IPV4_ROUTE:
					tprint_array_next();
					printxval(sysctl_net_ipv4_route,
						name[3],
						"NET_IPV4_ROUTE_???");
					break;
				case NET_IPV4_CONF:
					tprint_array_next();
					printxval(sysctl_net_ipv4_conf,
						name[3],
						"NET_IPV4_CONF_???");
					break;
				default:
					goto out;
				}
				break;
			case NET_IPV6:
				tprint_array_next();
				printxval(sysctl_net_ipv6, name[2],
					"NET_IPV6_???");

				if (info.nlen == 3)
					goto out;
				switch (name[2]) {
				case NET_IPV6_ROUTE:
					tprint_array_next();
					printxval(sysctl_net_ipv6_route,
						name[3],
						"NET_IPV6_ROUTE_???");
					break;
				default:
					goto out;
				}
				break;
			default:
				goto out;
			}
			break;
		default:
			goto out;
		}
out:
		max_cnt = info.nlen;
		if (abbrev(tcp) && max_cnt > max_strlen)
			max_cnt = max_strlen;
		while (cnt < max_cnt)
			tprintf_string(", %x", name[cnt++]);
		if (cnt < (unsigned) info.nlen)
			tprints_string(", ...");
		tprintf_string("}, %d, ", info.nlen);
	} else {
		size_t oldlen = 0;
		if (info.oldval == NULL) {
			tprint_null();
		} else if (umove(tcp, ptr_to_kulong(info.oldlenp), &oldlen) >= 0
				&& info.nlen >= 2
				&& ((name[0] == CTL_KERN
					&& (name[1] == KERN_OSRELEASE
						|| name[1] == KERN_OSTYPE
					)))) {
			printpath(tcp, ptr_to_kulong(info.oldval));
		} else {
			tprintf_string("%p", info.oldval);
		}
		tprintf_string(", %lu, ", (unsigned long)oldlen);
		if (info.newval == NULL)
			tprint_null();
		else if (syserror(tcp))
			tprintf_string("%p", info.newval);
		else
			printpath(tcp, ptr_to_kulong(info.newval));
		tprintf_string(", %lu", (unsigned long)info.newlen);
	}

	free(name);
	return 0;
}
