/*
 * Copyright (c) 2020-2021 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "xgetdents.h"
#include "kernel_dirent.h"

static void
decode_dents(struct tcb *const tcp, kernel_ulong_t addr, unsigned int len,
	     const unsigned int header_size,
	     const decode_dentry_head_fn decode_dentry_head,
	     const decode_dentry_tail_fn decode_dentry_tail)
{
	union {
		kernel_dirent_t ent;
		kernel_dirent64_t ent64;
	} dent;
	unsigned int count = 0;

	if (abbrev(tcp))
		printaddr(addr);

	for (;;) {
		if (len < header_size) {
			if (!abbrev(tcp)) {
				if (!len) {
					tprint_array_begin();
					++count;
				} else {
					printstr_ex(tcp, addr, len,
						    QUOTE_FORCE_HEX);
				}
			}
			break;
		}

		/* len >= header_size after this point.  */
		if (!tfetch_mem(tcp, addr, header_size, &dent)) {
			if (!abbrev(tcp)) {
				if (count) {
					tprint_more_data_follows();
					printaddr_comment(addr);
				} else {
					printaddr(addr);
				}
			}

			break;
		}

		if (!abbrev(tcp)) {
			if (!count)
				tprint_array_begin();
		}
		++count;

		kernel_ulong_t next_addr = 0;
		unsigned int next_len = 0;
		unsigned int d_reclen = decode_dentry_head(tcp, &dent);

		if (d_reclen > len) {
			/* cannot happen? */
			tprintf_comment("%s%u bytes overflow",
					(abbrev(tcp) ? "d_reclen " : ""),
					d_reclen - len);
			d_reclen = len;
		} else if (d_reclen < header_size) {
			/* cannot happen? */
			tprintf_comment("%s%u bytes underflow",
					(abbrev(tcp) ? "d_reclen " : ""),
					header_size - d_reclen);
			d_reclen = header_size;
			next_len = len - header_size;
		} else {
			next_len = len - d_reclen;
			if (next_len) {
				if (addr + d_reclen > addr) {
					next_addr = addr + d_reclen;
				} else {
					/* cannot happen? */
					tprints_comment("address overflow");
				}
			}
		}

		len = next_len;
		/* Do not use len inside the loop after this point.  */

		if (!abbrev(tcp)) {
			int rc = decode_dentry_tail(tcp, addr + header_size,
						   &dent,
						   d_reclen - header_size);
			if (next_addr) {
				tprint_array_next();
				if (rc < 0) {
					tprint_more_data_follows();
					break;
				}
			}
		}

		if (!next_addr)
			break;
		addr = next_addr;
	}

	if (!abbrev(tcp)) {
		if (count)
			tprint_array_end();
	} else {
		tprintf_comment("%u%s entries", count, len ? "+" : "");
	}
}

int
xgetdents(struct tcb *const tcp, const unsigned int header_size,
	  const decode_dentry_head_fn decode_dentry_head,
	  const decode_dentry_tail_fn decode_dentry_tail)
{
	if (entering(tcp)) {
		/* fd */
		printfd(tcp, tcp->u_arg[0]);
#ifdef ENABLE_SECONTEXT
		tcp->last_dirfd = (int) tcp->u_arg[0];
#endif
		tprint_arg_next();
		return 0;
	}

	const unsigned int count = tcp->u_arg[2];

	if (syserror(tcp) || !verbose(tcp) ||
	    (kernel_ulong_t) tcp->u_rval > count /* kernel gone bananas? */) {
		printaddr(tcp->u_arg[1]);
	} else {
		decode_dents(tcp, tcp->u_arg[1], tcp->u_rval, header_size,
			     decode_dentry_head, decode_dentry_tail);
	}
	tprint_arg_next();

	PRINT_VAL_U(count);
	return 0;
}
