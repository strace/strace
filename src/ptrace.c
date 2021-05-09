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
 * Copyright (c) 1999-2021 The strace developers.
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
		tprint_comment_begin();

	if (base_addr == addr)
		tprints(str);
	else
		tprintf("%s + %" PRI_klu,
			str, addr - (kernel_ulong_t) base_addr);

	if (xlat_verbose(xlat_verbosity) == XLAT_STYLE_VERBOSE)
		tprint_comment_end();
}

static void
decode_peeksiginfo_args(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct {
		uint64_t off;
		uint32_t flags;
		uint32_t nr;
	} psi;

	if (!umove_or_printaddr(tcp, addr, &psi)) {
		tprint_struct_begin();
		PRINT_FIELD_U(psi, off);
		tprint_struct_next();

		PRINT_FIELD_FLAGS(psi, flags, ptrace_peeksiginfo_flags,
				  "PTRACE_PEEKSIGINFO_???");
		tprint_struct_next();

		PRINT_FIELD_U(psi, nr);
		tprint_struct_end();
	}
}

static int
decode_seccomp_metadata(struct tcb *const tcp,
			const kernel_ulong_t addr,
			const kernel_ulong_t size)
{
	struct {
		uint64_t filter_off;
		uint64_t flags;
	} md;

	if (entering(tcp)) {
		if (size < sizeof(md.filter_off)) {
			printaddr(addr);
			return RVAL_DECODED;
		}

		if (umove_or_printaddr(tcp, addr, &md.filter_off)) {
			return RVAL_DECODED;
		}

		tprint_struct_begin();
		PRINT_FIELD_U(md, filter_off);
	} else {
		const size_t offset = sizeof(md.filter_off);

		if (!syserror(tcp) && size > offset) {
			tprint_struct_next();

			if (size < sizeof(md) ||
			    !tfetch_mem(tcp, addr + offset,
					sizeof(md.flags), &md.flags)) {
				tprint_unavailable();
			} else {
				PRINT_FIELD_FLAGS(md, flags,
						  seccomp_filter_flags,
						  "SECCOMP_FILTER_FLAG_???");
				if (size > sizeof(md)) {
					tprint_struct_next();
					tprint_more_data_follows();
				}
			}
		}

		tprint_struct_end();
	}

	return 0;
}

static int
decode_ptrace_entering(struct tcb *const tcp)
{
	const kernel_ulong_t request = tcp->u_arg[0];
	const int pid = tcp->u_arg[1];
	const kernel_ulong_t addr = tcp->u_arg[2];
	const kernel_ulong_t data = tcp->u_arg[3];

	/* request */
	printxval64(ptrace_cmds, request, "PTRACE_???");

	if (request == PTRACE_TRACEME) {
		/* pid, addr, and data are ignored. */
		return RVAL_DECODED;
	}

	/* pid */
	tprint_arg_next();
	printpid(tcp, pid, PT_TGID);

	switch (request) {
	case PTRACE_ATTACH:
	case PTRACE_INTERRUPT:
	case PTRACE_KILL:
	case PTRACE_LISTEN:
		/* addr and data are ignored */
		return RVAL_DECODED;
	}

	/* addr */
	tprint_arg_next();
	switch (request) {
	case PTRACE_PEEKUSER:
	case PTRACE_POKEUSER:
		print_user_offset_addr(addr);
		break;
	case PTRACE_GETREGSET:
	case PTRACE_SETREGSET:
		printxval(nt_descriptor_types, addr, "NT_???");
		break;
	case PTRACE_GETSIGMASK:
	case PTRACE_SETSIGMASK:
	case PTRACE_SECCOMP_GET_FILTER:
	case PTRACE_SECCOMP_GET_METADATA:
	case PTRACE_GET_SYSCALL_INFO:
		PRINT_VAL_U(addr);
		break;
	case PTRACE_PEEKSIGINFO:
		decode_peeksiginfo_args(tcp, addr);
		break;
	default:
		printaddr(addr);
	}

# ifdef IA64
	switch (request) {
	case PTRACE_PEEKDATA:
	case PTRACE_PEEKTEXT:
	case PTRACE_PEEKUSER:
		/* data is ignored */
		return RVAL_DECODED | RVAL_HEX;
	}
# endif /* IA64 */

# if defined SPARC || defined SPARC64
	switch (request) {
	case PTRACE_GETREGS:
	case PTRACE_SETREGS:
	case PTRACE_GETFPREGS:
	case PTRACE_SETFPREGS:
		/* data is ignored */
		return RVAL_DECODED;
	}
# endif /* SPARC || SPARC64 */

	/* data */
	tprint_arg_next();
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
		tprint_iov(tcp, /*len:*/ 1, data, iov_decode_addr);
		break;
	case PTRACE_SECCOMP_GET_METADATA:
		return decode_seccomp_metadata(tcp, data, addr);
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
}

static int
decode_ptrace_exiting(struct tcb *const tcp)
{
	const kernel_ulong_t request = tcp->u_arg[0];
	const kernel_ulong_t addr = tcp->u_arg[2];
	const kernel_ulong_t data = tcp->u_arg[3];

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
		tprint_iov(tcp, /*len:*/ 1, data, iov_decode_addr);
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
	case PTRACE_SECCOMP_GET_METADATA:
		return decode_seccomp_metadata(tcp, data, tcp->u_rval);
	case PTRACE_GET_SYSCALL_INFO:
		print_ptrace_syscall_info(tcp, data, addr);
		break;
	}

	return 0;
}

SYS_FUNC(ptrace)
{
	return entering(tcp) ? decode_ptrace_entering(tcp)
			     : decode_ptrace_exiting(tcp);
}
