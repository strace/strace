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
 * Copyright (c) 1999-2022 The strace developers.
 *
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_ELF_H
# include <elf.h>
#endif

#include "iovec.h"
#include "ptrace.h"
#include "ptrace_syscall_info.h"
#include "regs.h"

#include "xlat/nt_descriptor_types.h"
#include "xlat/ptrace_cmds.h"
#include "xlat/compat_ptrace_cmds.h"
#include "xlat/ptrace_setoptions_flags.h"
#include "xlat/ptrace_peeksiginfo_flags.h"

#define uoff(member)	offsetof(struct user, member)
#define XLAT_UOFF(member)	{ uoff(member), "offsetof(struct user, " #member ")" }

#ifdef COMPAT_PTRACE_GETREGS
# define HAVE_COMPAT_PTRACE_MACROS 1
#else
# define HAVE_COMPAT_PTRACE_MACROS 0
#endif

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

	tprints_string(str);
	if (base_addr != addr) {
		tprint_plus();
		PRINT_VAL_U(addr - (kernel_ulong_t) base_addr);
	}

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

static void
decode_regset(struct tcb *const tcp, const strace_iovec *iov,
	      const unsigned int nt)
{
	switch (nt) {
		case NT_PRSTATUS:
			decode_prstatus_regset(tcp, iov->iov_base, iov->iov_len);
			break;
		case NT_FPREGSET:
			decode_fpregset(tcp, iov->iov_base, iov->iov_len);
			break;
		default:
			printaddr(iov->iov_base);
			break;
	}
}

static int
decode_getregset(struct tcb *const tcp, const kernel_ulong_t addr,
		 const unsigned int nt)
{
	strace_iovec iov;

	if (!fetch_struct_iovec(tcp, addr, &iov)) {
		printaddr(addr);
		return RVAL_DECODED;
	}

	if (entering(tcp)) {
		set_tcb_priv_ulong(tcp, iov.iov_len);
	} else {
		const unsigned long old_len = get_tcb_priv_ulong(tcp);

		tprint_struct_begin();
		tprints_field_name("iov_base");
		decode_regset(tcp, &iov, nt);
		tprint_struct_next();

		if (old_len == iov.iov_len) {
			PRINT_FIELD_U(iov, iov_len);
		} else {
			tprints_field_name("iov_len");
			PRINT_VAL_U(old_len);
			tprint_value_changed();
			PRINT_VAL_U(iov.iov_len);
		}
		tprint_struct_end();
	}

	return 0;
}

static int
decode_setregset(struct tcb *const tcp, const kernel_ulong_t addr,
		 const unsigned int nt)
{
	strace_iovec iov;

	if (entering(tcp)) {
		if (!fetch_struct_iovec(tcp, addr, &iov)) {
			printaddr(addr);
			return RVAL_DECODED;
		}

		tprint_struct_begin();
		tprints_field_name("iov_base");
		decode_regset(tcp, &iov, nt);
		tprint_struct_next();

		PRINT_FIELD_U(iov, iov_len);

		set_tcb_priv_ulong(tcp, iov.iov_len);
	} else {
		if (fetch_struct_iovec(tcp, addr, &iov) &&
		    get_tcb_priv_ulong(tcp) != iov.iov_len) {
			tprint_value_changed();
			PRINT_VAL_U(iov.iov_len);
		}

		tprint_struct_end();
	}

	return 0;
}

#ifdef PTRACE_GETREGS64
# include "arch_pt_regs64.c"
#endif

static int
decode_ptrace_entering(struct tcb *const tcp)
{
	const kernel_ulong_t request = tcp->u_arg[0];
	const int pid = tcp->u_arg[1];
	const kernel_ulong_t addr = tcp->u_arg[2];
	const kernel_ulong_t data = tcp->u_arg[3];

/*
 * SPARC systems have the meaning of data and addr reversed
 * for PTRACE_[GS]ETREGS and PTRACE_[GS]ETFPREGS:
 * data is ignored and the registers are copied from/to the address addr.
 */
#if defined SPARC || defined SPARC64
# define regs_addr	addr
#else
# define regs_addr	data
#endif

	/* COMPAT_PTRACE_* */
#if HAVE_COMPAT_PTRACE_MACROS
	if (current_personality != 1) {
		switch (request) {
		case COMPAT_PTRACE_GETREGS:
		case COMPAT_PTRACE_SETREGS:
		case COMPAT_PTRACE_GETFPREGS:
		case COMPAT_PTRACE_SETFPREGS:
		case COMPAT_PTRACE_GET_THREAD_AREA:
		case COMPAT_PTRACE_SET_SYSCALL:
		case COMPAT_PTRACE_GETVFPREGS:
		case COMPAT_PTRACE_SETVFPREGS:
		case COMPAT_PTRACE_GETHBPREGS:
		case COMPAT_PTRACE_SETHBPREGS:
			printxvals_ex(request, "COMPAT_PTRACE_???",
				      xlat_verbose(xlat_verbosity)
					== XLAT_STYLE_RAW ? XLAT_STYLE_RAW
							  : XLAT_STYLE_VERBOSE,
				      compat_ptrace_cmds, NULL);
			tprint_arg_next();
			printpid(tcp, pid, PT_TGID);
			tprint_arg_next();
			printaddr(addr);
			tprint_arg_next();
			printaddr(data);
			return RVAL_DECODED;
		}
	}
#endif /* HAVE_COMPAT_PTRACE_MACROS */

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
#ifdef IA64
		print_user_offset_addr(addr);
		/* data is ignored */
		return RVAL_DECODED | RVAL_HEX;
#endif
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
#ifdef PTRACE_SETREGS
	case PTRACE_SETREGS:
		decode_pt_regs(tcp, regs_addr);
		return RVAL_DECODED;
#endif
#ifdef PTRACE_SETREGS64
	case PTRACE_SETREGS64:
		decode_pt_regs64(tcp, regs_addr);
		return RVAL_DECODED;
#endif
#ifdef PTRACE_SETFPREGS
	case PTRACE_SETFPREGS:
		decode_pt_fpregs(tcp, regs_addr);
		return RVAL_DECODED;
#endif
#ifdef PTRACE_GETREGS
	case PTRACE_GETREGS:
		/* print regs_addr on exiting syscall */
		return 0;
#endif
#ifdef PTRACE_GETREGS64
	case PTRACE_GETREGS64:
		/* print regs_addr on exiting syscall */
		return 0;
#endif
#ifdef PTRACE_GETFPREGS
	case PTRACE_GETFPREGS:
		/* print regs_addr on exiting syscall */
		return 0;
#endif
#ifdef IA64
	case PTRACE_PEEKDATA:
	case PTRACE_PEEKTEXT:
		printaddr(addr);
		/* data is ignored */
		return RVAL_DECODED | RVAL_HEX;
#endif /* IA64 */
	default:
		printaddr(addr);
	}

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
	case PTRACE_GETREGSET:
		return decode_getregset(tcp, data, addr);
	case PTRACE_SETREGSET:
		return decode_setregset(tcp, data, addr);
		break;
	case PTRACE_SECCOMP_GET_METADATA:
		return decode_seccomp_metadata(tcp, data, addr);
#ifndef IA64
	case PTRACE_PEEKDATA:
	case PTRACE_PEEKTEXT:
	case PTRACE_PEEKUSER:
#endif
	case PTRACE_GETEVENTMSG:
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
		return decode_getregset(tcp, data, addr);
	case PTRACE_SETREGSET:
		return decode_setregset(tcp, data, addr);
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
#ifdef PTRACE_GETREGS
	case PTRACE_GETREGS:
		decode_pt_regs(tcp, regs_addr);
		break;
#endif
#ifdef PTRACE_GETREGS64
	case PTRACE_GETREGS64:
		decode_pt_regs64(tcp, regs_addr);
		break;
#endif
#ifdef PTRACE_GETFPREGS
	case PTRACE_GETFPREGS:
		decode_pt_fpregs(tcp, regs_addr);
		break;
#endif
	}

	return 0;
}

SYS_FUNC(ptrace)
{
	return entering(tcp) ? decode_ptrace_entering(tcp)
			     : decode_ptrace_exiting(tcp);
}
