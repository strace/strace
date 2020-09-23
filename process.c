/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 2000 PocketPenguins Inc.  Linux for Hitachi SuperH
 *                    port by Greg Banks <gbanks@pocketpenguins.com>
 * Copyright (c) 1999-2020 The strace developers.
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_ELF_H
# include <elf.h>
#endif

#include "ptrace.h"
#include "ptrace_syscall_info.h"
#include "regs.h"

#include "xlat/nt_descriptor_types.h"
#include "xlat/ptrace_cmds.h"
#include "xlat/ptrace_setoptions_flags.h"
#include "xlat/ptrace_peeksiginfo_flags.h"

#define uoff(member)	offsetof(struct user, member)
#define XLAT_UOFF(member)	{ uoff(member), "offsetof(struct user, " #member ")" }

static const struct xlat_data struct_user_offsets_data[] = {
#include "userent.h"
	{ 0, 0 }
};

static const struct xlat struct_user_offsets = {
	.type = XT_SORTED,
	.size = ARRAY_SIZE(struct_user_offsets_data) - 1,
	.data = struct_user_offsets_data,
};

static void
print_user_offset_addr(const kernel_ulong_t addr)
{
	const uint64_t last_user_offset = struct_user_offsets.size ?
		struct_user_offsets.data[struct_user_offsets.size - 1].val : 0;

	uint64_t base_addr = addr;
	const char *str = xlookup_le(&struct_user_offsets, &base_addr);

	/* We don't want to pretty print addresses beyond struct user */
	if (addr > base_addr && base_addr == last_user_offset)
		str = NULL;

	if (!str || xlat_verbose(xlat_verbosity) != XLAT_STYLE_ABBREV)
		printaddr(addr);
	if (!str || xlat_verbose(xlat_verbosity) == XLAT_STYLE_RAW)
		return;

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprints(" /* ");

	if (base_addr == addr)
		tprints(str);
	else
		tprintf("%s + %" PRI_klu,
			str, addr - (kernel_ulong_t) base_addr);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprints(" */");
}

SYS_FUNC(ptrace)
{
	const kernel_ulong_t request = tcp->u_arg[0];
	const int pid = tcp->u_arg[1];
	const kernel_ulong_t addr = tcp->u_arg[2];
	const kernel_ulong_t data = tcp->u_arg[3];

	if (entering(tcp)) {
		/* request */
		printxval64(ptrace_cmds, request, "PTRACE_???");

		if (request == PTRACE_TRACEME) {
			/* pid, addr, and data are ignored. */
			return RVAL_DECODED;
		}

		/* pid */
		tprints(", ");
		printpid(tcp, pid, PT_TGID);

		/* addr */
		switch (request) {
		case PTRACE_ATTACH:
		case PTRACE_INTERRUPT:
		case PTRACE_KILL:
		case PTRACE_LISTEN:
			/* addr and data are ignored */
			return RVAL_DECODED;
		case PTRACE_PEEKUSER:
		case PTRACE_POKEUSER:
			tprints(", ");
			print_user_offset_addr(addr);
			break;
		case PTRACE_GETREGSET:
		case PTRACE_SETREGSET:
			tprints(", ");
			printxval(nt_descriptor_types, addr, "NT_???");
			break;
		case PTRACE_GETSIGMASK:
		case PTRACE_SETSIGMASK:
		case PTRACE_SECCOMP_GET_FILTER:
		case PTRACE_SECCOMP_GET_METADATA:
		case PTRACE_GET_SYSCALL_INFO:
			tprintf(", %" PRI_klu, addr);
			break;
		case PTRACE_PEEKSIGINFO: {
			tprints(", ");
			struct {
				uint64_t off;
				uint32_t flags;
				uint32_t nr;
			} psi;
			if (umove_or_printaddr(tcp, addr, &psi)) {
				tprints(", ");
				printaddr(data);
				return RVAL_DECODED;
			}
			tprintf("{off=%" PRIu64 ", flags=", psi.off);
			printflags(ptrace_peeksiginfo_flags, psi.flags,
				   "PTRACE_PEEKSIGINFO_???");
			tprintf(", nr=%u}", psi.nr);
			break;
		}
		default:
			tprints(", ");
			printaddr(addr);
		}

#if defined IA64 || defined SPARC || defined SPARC64
		switch (request) {
# ifdef IA64
		case PTRACE_PEEKDATA:
		case PTRACE_PEEKTEXT:
		case PTRACE_PEEKUSER:
			/* data is ignored */
			return RVAL_DECODED | RVAL_HEX;
# endif /* IA64 */
# if defined SPARC || defined SPARC64
		case PTRACE_GETREGS:
		case PTRACE_SETREGS:
		case PTRACE_GETFPREGS:
		case PTRACE_SETFPREGS:
			/* data is ignored */
			return RVAL_DECODED;
# endif /* SPARC || SPARC64 */
		}
#endif /* IA64 || SPARC || SPARC64 */

		tprints(", ");

		/* data */
		switch (request) {
		case PTRACE_CONT:
		case PTRACE_DETACH:
		case PTRACE_SYSCALL:
#ifdef PTRACE_SINGLESTEP
		case PTRACE_SINGLESTEP:
#endif
#ifdef PTRACE_SINGLEBLOCK
		case PTRACE_SINGLEBLOCK:
#endif
#ifdef PTRACE_SYSEMU
		case PTRACE_SYSEMU:
#endif
#ifdef PTRACE_SYSEMU_SINGLESTEP
		case PTRACE_SYSEMU_SINGLESTEP:
#endif
			printsignal(data);
			break;
		case PTRACE_SEIZE:
		case PTRACE_SETOPTIONS:
#ifdef PTRACE_OLDSETOPTIONS
		case PTRACE_OLDSETOPTIONS:
#endif
			printflags64(ptrace_setoptions_flags, data, "PTRACE_O_???");
			break;
		case PTRACE_SETSIGINFO:
			printsiginfo_at(tcp, data);
			break;
		case PTRACE_SETSIGMASK:
			print_sigset_addr_len(tcp, data, addr);
			break;
		case PTRACE_SETREGSET:
			tprint_iov(tcp, /*len:*/ 1, data, IOV_DECODE_ADDR);
			break;
		case PTRACE_SECCOMP_GET_METADATA:
			if (verbose(tcp)) {
				uint64_t filter_off;
				if (addr < sizeof(filter_off) ||
				    umove(tcp, data, &filter_off)) {
					printaddr(data);
					return RVAL_DECODED;
				}

				tprintf("{filter_off=%" PRIu64, filter_off);
				return 0;
			}

			printaddr(data);
			break;
#ifndef IA64
		case PTRACE_PEEKDATA:
		case PTRACE_PEEKTEXT:
		case PTRACE_PEEKUSER:
#endif
		case PTRACE_GETEVENTMSG:
		case PTRACE_GETREGSET:
		case PTRACE_GETSIGINFO:
		case PTRACE_GETSIGMASK:
		case PTRACE_PEEKSIGINFO:
		case PTRACE_SECCOMP_GET_FILTER:
		case PTRACE_GET_SYSCALL_INFO:
			if (verbose(tcp)) {
				/* print data on exiting syscall */
				return 0;
			}
			ATTRIBUTE_FALLTHROUGH;
		default:
			printaddr(data);
			break;
		}

		return RVAL_DECODED;
	} else {
		switch (request) {
#ifndef IA64
		case PTRACE_PEEKDATA:
		case PTRACE_PEEKTEXT:
		case PTRACE_PEEKUSER:
			printnum_ptr(tcp, data);
			break;
#endif
		case PTRACE_GETEVENTMSG:
			printnum_ulong(tcp, data);
			break;
		case PTRACE_GETREGSET:
			tprint_iov(tcp, /*len:*/ 1, data, IOV_DECODE_ADDR);
			break;
		case PTRACE_GETSIGINFO:
			printsiginfo_at(tcp, data);
			break;
		case PTRACE_GETSIGMASK:
			print_sigset_addr_len(tcp, data, addr);
			break;
		case PTRACE_PEEKSIGINFO:
			print_siginfo_array(tcp, data, tcp->u_rval);
			break;
		case PTRACE_SECCOMP_GET_FILTER:
			print_seccomp_fprog(tcp, data, tcp->u_rval);
			break;
		case PTRACE_SECCOMP_GET_METADATA: {
			const size_t offset = sizeof(uint64_t);
			uint64_t flags = 0;
			size_t ret_size = MIN((kernel_ulong_t) tcp->u_rval,
					      offset + sizeof(flags));

			if (syserror(tcp) || ret_size <= offset) {
				tprints("}");
				return 0;
			}

			if (umoven(tcp, data + offset, ret_size - offset,
				   &flags)) {
				tprints(", ...}");
				return 0;
			}

			tprints(", flags=");
			printflags64(seccomp_filter_flags, flags,
				     "SECCOMP_FILTER_FLAG_???");

			if ((kernel_ulong_t) tcp->u_rval > ret_size)
				tprints(", ...");

			tprints("}");
			break;
		}
		case PTRACE_GET_SYSCALL_INFO:
			print_ptrace_syscall_info(tcp, data, addr);
			break;
		}
	}
	return 0;
}
