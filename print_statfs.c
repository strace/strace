/*
 * Copyright (c) 2014-2016 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"
#include "statfs.h"
#include "xlat/fsmagic.h"
#include "xlat/statfs_flags.h"

static void
print_statfs_type(const char *const prefix, const unsigned long long magic)
{
	tprints(prefix);
	printxval_search(fsmagic, magic, NULL);
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
