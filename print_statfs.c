/*
 * Copyright (c) 2014-2018 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include "statfs.h"
#include "xlat/fsmagic.h"
#include "xlat/statfs_flags.h"

static void
print_statfs_type(const char *const prefix, const unsigned long long magic)
{
	tprints(prefix);
	printxval(fsmagic, magic, NULL);
}

#if defined HAVE_STRUCT_STATFS_F_FLAGS || defined HAVE_STRUCT_STATFS64_F_FLAGS
static void
print_statfs_flags(const char *const prefix, const unsigned long long flags)
{
	if (flags & ST_VALID) {
		tprints(prefix);
		printflags64(statfs_flags, flags, "ST_???");
	}
}
#endif /* HAVE_STRUCT_STATFS_F_FLAGS || HAVE_STRUCT_STATFS64_F_FLAGS */

static void
print_statfs_number(const char *const prefix, const unsigned long long number)
{
	tprints(prefix);
	tprintf("%llu",  number);
}

void
print_struct_statfs(struct tcb *const tcp, const kernel_ulong_t addr)
{
#ifdef HAVE_STRUCT_STATFS
	struct strace_statfs b;

	if (!fetch_struct_statfs(tcp, addr, &b))
		return;

	print_statfs_type("{f_type=", b.f_type);
	print_statfs_number(", f_bsize=", b.f_bsize);
	print_statfs_number(", f_blocks=", b.f_blocks);
	print_statfs_number(", f_bfree=", b.f_bfree);
	print_statfs_number(", f_bavail=", b.f_bavail);
	print_statfs_number(", f_files=", b.f_files);
	print_statfs_number(", f_ffree=", b.f_ffree);
# if defined HAVE_STRUCT_STATFS_F_FSID_VAL \
  || defined HAVE_STRUCT_STATFS_F_FSID___VAL
	print_statfs_number(", f_fsid={val=[", b.f_fsid[0]);
	print_statfs_number(", ", b.f_fsid[1]);
	tprints("]}");
# endif
	print_statfs_number(", f_namelen=", b.f_namelen);
# ifdef HAVE_STRUCT_STATFS_F_FRSIZE
	print_statfs_number(", f_frsize=", b.f_frsize);
# endif
# ifdef HAVE_STRUCT_STATFS_F_FLAGS
	print_statfs_flags(", f_flags=", b.f_flags);
# endif
	tprints("}");
#else
	printaddr(addr);
#endif
}

void
print_struct_statfs64(struct tcb *const tcp, const kernel_ulong_t addr,
		      const kernel_ulong_t size)
{
#ifdef HAVE_STRUCT_STATFS64
	struct strace_statfs b;

	if (!fetch_struct_statfs64(tcp, addr, size, &b))
		return;

	print_statfs_type("{f_type=", b.f_type);
	print_statfs_number(", f_bsize=", b.f_bsize);
	print_statfs_number(", f_blocks=", b.f_blocks);
	print_statfs_number(", f_bfree=", b.f_bfree);
	print_statfs_number(", f_bavail=", b.f_bavail);
	print_statfs_number(", f_files=", b.f_files);
	print_statfs_number(", f_ffree=", b.f_ffree);
# if defined HAVE_STRUCT_STATFS64_F_FSID_VAL \
  || defined HAVE_STRUCT_STATFS64_F_FSID___VAL
	print_statfs_number(", f_fsid={val=[", b.f_fsid[0]);
	print_statfs_number(", ", b.f_fsid[1]);
	tprints("]}");
# endif
	print_statfs_number(", f_namelen=", b.f_namelen);
# ifdef HAVE_STRUCT_STATFS64_F_FRSIZE
	print_statfs_number(", f_frsize=", b.f_frsize);
# endif
# ifdef HAVE_STRUCT_STATFS64_F_FLAGS
	print_statfs_flags(", f_flags=", b.f_flags);
# endif
	tprints("}");
#else
	printaddr(addr);
#endif
}
