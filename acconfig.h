
/* Define if this is running the Linux operating system.  */
#undef LINUX

/* Define if this is running the SunOS 4.x operating system.  */
#undef SUNOS4

/* Define if this is running the System V release 4 operating system
   or a derivative like Solaris 2.x or Irix 5.x.  */
#undef SVR4

/* Define if this is an i386, i486 or pentium architecture.  */
#undef I386

/* Define if this is an m68k architecture.  */
#undef M68K

/* Define if this is a sparc architecture.  */
#undef SPARC

/* Define if this is a mips architecture.  */
#undef MIPS

/* Define if this is an alpha architecture.  */
#undef ALPHA

/* Define if you have SVR4 and the poll system call works on /proc files.  */
#undef HAVE_POLLABLE_PROCFS

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
