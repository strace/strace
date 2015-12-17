/*
 * Copyright (c) 1999-2003 Ulrich Drepper <drepper@redhat.com>
 * Copyright (c) 2004 David S. Miller <davem@nuts.davemloft.net>
 * Copyright (c) 2003-2005 Roland McGrath <roland@redhat.com>
 * Copyright (c) 2007 Jan Kratochvil <jan.kratochvil@redhat.com>
 * Copyright (c) 2009 Denys Vlasenko <dvlasenk@redhat.com>
 * Copyright (c) 2009-2010 Andreas Schwab <schwab@linux-m68k.org>
 * Copyright (c) 2012 H.J. Lu <hongjiu.lu@intel.com>
 * Copyright (c) 2005-2015 Dmitry V. Levin <ldv@altlinux.org>
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

#ifndef DO_PRINTSTAT
# define DO_PRINTSTAT do_printstat
#endif

#ifndef STRUCT_STAT
# define STRUCT_STAT struct stat
#endif

#ifndef STAT_MAJOR
# define STAT_MAJOR(x) major(x)
#endif

#ifndef STAT_MINOR
# define STAT_MINOR(x) minor(x)
#endif

static void
DO_PRINTSTAT(struct tcb *tcp, const STRUCT_STAT *statbuf)
{
	if (!abbrev(tcp)) {
		tprintf("{st_dev=makedev(%u, %u), st_ino=%llu, st_mode=%s, ",
			(unsigned int) STAT_MAJOR(statbuf->st_dev),
			(unsigned int) STAT_MINOR(statbuf->st_dev),
			(unsigned long long) statbuf->st_ino,
			sprintmode(statbuf->st_mode));
		tprintf("st_nlink=%u, st_uid=%u, st_gid=%u, ",
			(unsigned int) statbuf->st_nlink,
			(unsigned int) statbuf->st_uid,
			(unsigned int) statbuf->st_gid);
#ifdef HAVE_STRUCT_STAT_ST_BLKSIZE
		tprintf("st_blksize=%u, ", (unsigned int) statbuf->st_blksize);
#endif
#ifdef HAVE_STRUCT_STAT_ST_BLOCKS
		tprintf("st_blocks=%llu, ",
			(unsigned long long) statbuf->st_blocks);
#endif
	} else {
		tprintf("{st_mode=%s, ", sprintmode(statbuf->st_mode));
	}

	switch (statbuf->st_mode & S_IFMT) {
	case S_IFCHR: case S_IFBLK:
#ifdef HAVE_STRUCT_STAT_ST_RDEV
		tprintf("st_rdev=makedev(%u, %u), ",
			(unsigned int) STAT_MAJOR(statbuf->st_rdev),
			(unsigned int) STAT_MINOR(statbuf->st_rdev));
#else /* !HAVE_STRUCT_STAT_ST_RDEV */
		tprintf("st_size=makedev(%u, %u), ",
			(unsigned int) STAT_MAJOR(statbuf->st_size),
			(unsigned int) STAT_MINOR(statbuf->st_size));
#endif /* !HAVE_STRUCT_STAT_ST_RDEV */
		break;
	default:
		tprintf("st_size=%llu, ",
			(unsigned long long) statbuf->st_size);
		break;
	}

	if (!abbrev(tcp)) {
		const bool cast = sizeof(statbuf->st_atime) == sizeof(int);

		tprints("st_atime=");
		tprints(sprinttime(cast ? (time_t) (int) statbuf->st_atime:
					  (time_t) statbuf->st_atime));
#ifdef HAVE_STRUCT_STAT_ST_ATIME_NSEC
		if (statbuf->st_atime_nsec)
			tprintf(".%09lu", (unsigned long) statbuf->st_atime_nsec);
#endif
		tprints(", st_mtime=");
		tprints(sprinttime(cast ? (time_t) (int) statbuf->st_mtime:
					  (time_t) statbuf->st_mtime));
#ifdef HAVE_STRUCT_STAT_ST_MTIME_NSEC
		if (statbuf->st_mtime_nsec)
			tprintf(".%09lu", (unsigned long) statbuf->st_mtime_nsec);
#endif
		tprints(", st_ctime=");
		tprints(sprinttime(cast ? (time_t) (int) statbuf->st_ctime:
					  (time_t) statbuf->st_ctime));
#ifdef HAVE_STRUCT_STAT_ST_CTIME_NSEC
		if (statbuf->st_ctime_nsec)
			tprintf(".%09lu", (unsigned long) statbuf->st_ctime_nsec);
#endif
#ifdef HAVE_STRUCT_STAT_ST_FLAGS
		tprintf(", st_flags=%u", (unsigned int) statbuf->st_flags);
#endif
#ifdef HAVE_STRUCT_STAT_ST_FSTYPE
		tprintf(", st_fstype=%.*s",
			(int) sizeof statbuf->st_fstype, statbuf->st_fstype);
#endif
#ifdef HAVE_STRUCT_STAT_ST_GEN
		tprintf(", st_gen=%u", (unsigned int) statbuf->st_gen);
#endif
		tprints("}");
	} else {
		tprints("...}");
	}
}

#undef STAT_MINOR
#undef STAT_MAJOR
#undef STRUCT_STAT
#undef DO_PRINTSTAT
