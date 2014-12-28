#ifndef DO_PRINTSTAT
# define DO_PRINTSTAT do_printstat
#endif

#ifndef STRUCT_STAT
# define STRUCT_STAT struct stat
#endif

static void
DO_PRINTSTAT(struct tcb *tcp, const STRUCT_STAT *statbuf)
{
	if (!abbrev(tcp)) {
		tprintf("{st_dev=makedev(%u, %u), st_ino=%llu, st_mode=%s, ",
			(unsigned int) major(statbuf->st_dev),
			(unsigned int) minor(statbuf->st_dev),
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
			(unsigned int) major(statbuf->st_rdev),
			(unsigned int) minor(statbuf->st_rdev));
#else /* !HAVE_STRUCT_STAT_ST_RDEV */
		tprintf("st_size=makedev(%u, %u), ",
			(unsigned int) major(statbuf->st_size),
			(unsigned int) minor(statbuf->st_size));
#endif /* !HAVE_STRUCT_STAT_ST_RDEV */
		break;
	default:
		tprintf("st_size=%llu, ",
			(unsigned long long) statbuf->st_size);
		break;
	}

	if (!abbrev(tcp)) {
		tprintf("st_atime=%s, ", sprinttime(statbuf->st_atime));
		tprintf("st_mtime=%s, ", sprinttime(statbuf->st_mtime));
		tprintf("st_ctime=%s", sprinttime(statbuf->st_ctime));
#if HAVE_STRUCT_STAT_ST_FLAGS
		tprintf(", st_flags=%u", (unsigned int) statbuf->st_flags);
#endif
#if HAVE_STRUCT_STAT_ST_FSTYPE
		tprintf(", st_fstype=%.*s",
			(int) sizeof statbuf->st_fstype, statbuf->st_fstype);
#endif
#if HAVE_STRUCT_STAT_ST_GEN
		tprintf(", st_gen=%u", (unsigned int) statbuf->st_gen);
#endif
		tprints("}");
	} else {
		tprints("...}");
	}
}

#undef STRUCT_STAT
#undef DO_PRINTSTAT
