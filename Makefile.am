# Automake input for strace.

bin_PROGRAMS = strace
man_MANS = strace.1

# OS is one of `linux', `sunos4', `svr4', or `freebsd'.
OS		= @opsys@
# ARCH is `i386', `m68k', `sparc', etc.
ARCH		= @arch@

INCLUDES = -I$(OS)/$(ARCH) -I$(srcdir)/$(OS)/$(ARCH) -I$(OS) -I$(srcdir)/$(OS)

strace_SOURCES = strace.c version.c syscall.c util.c desc.c file.c ipc.c \
		 io.c ioctl.c mem.c net.c process.c bjm.c \
		 resource.c signal.c sock.c system.c term.c time.c \
		 proc.c stream.c

EXTRA_DIST = $(man_MANS) errnoent.sh signalent.sh syscallent.sh ioctlsort.c \
	     linux/ioctlsort.c \
	     linux/ioctlent.h linux/errnoent.h linux/signalent.h \
	     linux/syscall.h linux/syscallent.h linux/dummy.h \
	     linux/*/errnoent.h linux/*/ioctlent.h \
	     linux/*/signalent.h linux/*/syscallent.h \
	     freebsd/ioctlent.sh \
	     freebsd/syscalls.cat freebsd/syscalls.pl freebsd/syscalls.print \
	     freebsd/*/errnoent.h freebsd/*/ioctlent.h \
	     freebsd/*/signalent.h freebsd/*/syscall.h freebsd/*/syscallent.h
	     sunos4/dummy.h sunos4/errnoent.h \
	     sunos4/ioctlent.h sunos4/ioctlent.sh sunos4/signalent.h \
	     sunos4/syscall.h sunos4/syscallent.h \
	     svr4/dummy.h svr4/errnoent.h svr4/ioctlent.h svr4/ioctlent.sh \
	     svr4/signalent.h svr4/syscall.h svr4/syscallent.h
