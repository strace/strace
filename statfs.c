#include "defs.h"
#ifdef HAVE_SYS_VFS_H
# include <sys/vfs.h>
#endif
#include "xlat/fsmagic.h"

static const char *
sprintfstype(const unsigned int magic)
{
	static char buf[32];
	const char *s;

	s = xlat_search(fsmagic, ARRAY_SIZE(fsmagic), magic);
	if (s) {
		sprintf(buf, "\"%s\"", s);
		return buf;
	}
	sprintf(buf, "%#x", magic);
	return buf;
}

static void
printstatfs(struct tcb *tcp, const long addr)
{
	struct statfs statbuf;

	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}
	tprintf("{f_type=%s, f_bsize=%lu, f_blocks=%lu, f_bfree=%lu, ",
		sprintfstype(statbuf.f_type),
		(unsigned long)statbuf.f_bsize,
		(unsigned long)statbuf.f_blocks,
		(unsigned long)statbuf.f_bfree);
	tprintf("f_bavail=%lu, f_files=%lu, f_ffree=%lu, f_fsid={%d, %d}",
		(unsigned long)statbuf.f_bavail,
		(unsigned long)statbuf.f_files,
		(unsigned long)statbuf.f_ffree,
		statbuf.f_fsid.__val[0], statbuf.f_fsid.__val[1]);
	tprintf(", f_namelen=%lu", (unsigned long)statbuf.f_namelen);
#ifdef _STATFS_F_FRSIZE
	tprintf(", f_frsize=%lu", (unsigned long)statbuf.f_frsize);
#endif
#ifdef _STATFS_F_FLAGS
	tprintf(", f_flags=%lu", (unsigned long)statbuf.f_flags);
#endif
	tprints("}");
}

SYS_FUNC(statfs)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printstatfs(tcp, tcp->u_arg[1]);
	}
	return 0;
}

SYS_FUNC(fstatfs)
{
	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printstatfs(tcp, tcp->u_arg[1]);
	}
	return 0;
}

#ifdef HAVE_STRUCT_STATFS64
static void
printstatfs64(struct tcb *tcp, long addr)
{
	struct statfs64 statbuf;

	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}
	tprintf("{f_type=%s, f_bsize=%llu, f_blocks=%llu, f_bfree=%llu, ",
		sprintfstype(statbuf.f_type),
		(unsigned long long)statbuf.f_bsize,
		(unsigned long long)statbuf.f_blocks,
		(unsigned long long)statbuf.f_bfree);
	tprintf("f_bavail=%llu, f_files=%llu, f_ffree=%llu, f_fsid={%d, %d}",
		(unsigned long long)statbuf.f_bavail,
		(unsigned long long)statbuf.f_files,
		(unsigned long long)statbuf.f_ffree,
		statbuf.f_fsid.__val[0], statbuf.f_fsid.__val[1]);
	tprintf(", f_namelen=%lu", (unsigned long)statbuf.f_namelen);
#ifdef _STATFS_F_FRSIZE
	tprintf(", f_frsize=%llu", (unsigned long long)statbuf.f_frsize);
#endif
#ifdef _STATFS_F_FLAGS
	tprintf(", f_flags=%llu", (unsigned long long)statbuf.f_flags);
#endif
	tprints("}");
}

struct compat_statfs64 {
	uint32_t f_type;
	uint32_t f_bsize;
	uint64_t f_blocks;
	uint64_t f_bfree;
	uint64_t f_bavail;
	uint64_t f_files;
	uint64_t f_ffree;
	fsid_t f_fsid;
	uint32_t f_namelen;
	uint32_t f_frsize;
	uint32_t f_flags;
	uint32_t f_spare[4];
}
#if defined AARCH64 || defined X86_64 || defined X32 || defined IA64
  ATTRIBUTE_PACKED ATTRIBUTE_ALIGNED(4)
#endif
;
#if defined AARCH64 || defined ARM
/* See arch/arm/kernel/sys_oabi-compat.c for details. */
# define COMPAT_STATFS64_PADDED_SIZE (sizeof(struct compat_statfs64) + 4)
#endif

static void
printcompat_statfs64(struct tcb *tcp, const long addr)
{
	struct compat_statfs64 statbuf;

	if (syserror(tcp) || !verbose(tcp)) {
		tprintf("%#lx", addr);
		return;
	}
	if (umove(tcp, addr, &statbuf) < 0) {
		tprints("{...}");
		return;
	}
	tprintf("{f_type=%s, f_bsize=%lu, f_blocks=%llu, f_bfree=%llu, ",
		sprintfstype(statbuf.f_type),
		(unsigned long)statbuf.f_bsize,
		(unsigned long long)statbuf.f_blocks,
		(unsigned long long)statbuf.f_bfree);
	tprintf("f_bavail=%llu, f_files=%llu, f_ffree=%llu, f_fsid={%d, %d}",
		(unsigned long long)statbuf.f_bavail,
		(unsigned long long)statbuf.f_files,
		(unsigned long long)statbuf.f_ffree,
		statbuf.f_fsid.__val[0], statbuf.f_fsid.__val[1]);
	tprintf(", f_namelen=%lu", (unsigned long)statbuf.f_namelen);
	tprintf(", f_frsize=%lu", (unsigned long)statbuf.f_frsize);
	tprintf(", f_flags=%lu}", (unsigned long)statbuf.f_frsize);
}

static int
do_statfs64_fstatfs64(struct tcb *tcp)
{
	if (entering(tcp)) {
		tprintf(", %lu, ", tcp->u_arg[1]);
	} else {
		if (tcp->u_arg[1] == sizeof(struct statfs64))
			printstatfs64(tcp, tcp->u_arg[2]);
		else if (tcp->u_arg[1] == sizeof(struct compat_statfs64)
#ifdef COMPAT_STATFS64_PADDED_SIZE
			 || tcp->u_arg[1] == COMPAT_STATFS64_PADDED_SIZE
#endif
									)
			printcompat_statfs64(tcp, tcp->u_arg[2]);
		else
			tprints("{???}");
	}
	return 0;
}

SYS_FUNC(statfs64)
{
	if (entering(tcp))
		printpath(tcp, tcp->u_arg[0]);
	return do_statfs64_fstatfs64(tcp);
}

SYS_FUNC(fstatfs64)
{
	if (entering(tcp))
		printfd(tcp, tcp->u_arg[0]);
	return do_statfs64_fstatfs64(tcp);
}
#endif /* HAVE_STRUCT_STATFS64 */

#ifdef ALPHA
SYS_FUNC(osf_statfs)
{
	if (entering(tcp)) {
		printpath(tcp, tcp->u_arg[0]);
		tprints(", ");
	} else {
		printstatfs(tcp, tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}

SYS_FUNC(osf_fstatfs)
{
	if (entering(tcp)) {
		tprintf("%lu, ", tcp->u_arg[0]);
	} else {
		printstatfs(tcp, tcp->u_arg[1]);
		tprintf(", %lu", tcp->u_arg[2]);
	}
	return 0;
}
#endif /* ALPHA */
