
/* Define if this is running the Linux operating system.  */
#undef LINUX

/* Define if this is running the SunOS 4.x operating system.  */
#undef SUNOS4

/* Define if this is running the System V release 4 operating system
   or a derivative like Solaris 2.x or Irix 5.x.  */
#undef SVR4

/* Define if this is running the FreeBSD operating system.  */
#undef FREEBSD

/* Define for UnixWare systems. */
#undef UNIXWARE

/* Define if this is an hppa architecture */
#undef HPPA

/* Define if this is an i386, i486 or pentium architecture.  */
#undef I386

/* Define if this is an ia64 architecture.  */
#undef IA64

/* Define if this is an m68k architecture.  */
#undef M68K

/* Define if this is a sparc architecture.  */
#undef SPARC

/* Define if this is a mips architecture.  */
#undef MIPS

/* Define if this is an alpha architecture.  */
#undef ALPHA

/* Define if this is an arm architecture.  */
#undef ARM

/* Define if this is a powerpc architecture.  */
#undef POWERPC

/* Define if this is a S390 architecture.  */
#undef S390

/* Define if you have a SVR4 MP type procfs.  I.E. /dev/xxx/ctl,
   /dev/xxx/status.  Also implies that you have the pr_lwp
   member in prstatus. */
#undef HAVE_MP_PROCFS

/* Define if you have SVR4 and the poll system call works on /proc files.  */
#undef HAVE_POLLABLE_PROCFS

/* Define if you have SVR4_MP and you need to use the poll hack
   to avoid unfinished system calls. */
#undef POLL_HACK

/* Define if the prstatus structure in sys/procfs.h has a pr_syscall member.  */
#undef HAVE_PR_SYSCALL

/* Define if you are have a SPARC with SUNOS4 and your want a version
   of strace that will work on sun4, sun4c and sun4m kernel architectures.
   Only useful if you have a symbolic link from machine to /usr/include/sun4
   in the compilation directory. */
#undef SUNOS4_KERNEL_ARCH_KLUDGE

/* Define if signal.h defines the type sig_atomic_t.  */
#undef HAVE_SIG_ATOMIC_T

/* Define if sys_errlist is declared.  */
#undef SYS_ERRLIST_DECLARED

/* Define if the msghdr structure has a msg_control member.  */
#undef HAVE_MSG_CONTROL

/* Define if stat64 is available in asm/stat.h.  */
#undef HAVE_STAT64

/* Define if your compiler knows about long long */
#undef HAVE_LONG_LONG

/* Define if off_t is a long long */
#undef HAVE_LONG_LONG_OFF_T

/* Define if rlim_t is a long long */
#undef HAVE_LONG_LONG_RLIM_T

/* Define if struct sockaddr_in6 contains sin6_scope_id field. */
#undef HAVE_SIN6_SCOPE_ID

/* Define if linux struct sockaddr_in6 contains sin6_scope_id fiels. */
#undef HAVE_SIN6_SCOPE_ID_LINUX

/* Define if have st_flags in struct stat */
#undef HAVE_ST_FLAGS

/* Define if have st_aclcnt in struct stat */
#undef HAVE_ST_ACLCNT

/* Define if have st_level in struct stat */
#undef HAVE_ST_LEVEL

/* Define if have st_fstype in struct stat */
#undef HAVE_ST_FSTYPE

/* Define if have st_gen in struct stat */
#undef HAVE_ST_GEN

/* Define if have little endiang long long */
#undef HAVE_LITTLE_ENDIAN_LONG_LONG
