# Automake input for strace.

bin_PROGRAMS = strace
man_MANS = strace.1

# OS is one of `linux', `sunos4', `svr4', or `freebsd'.
OS		= @opsys@
# ARCH is `i386', `m68k', `sparc', etc.
ARCH		= @arch@
# OSARCH is OS/ARCH if a makefile exists there, otherwise just OS.
OSARCH		= @osarch@

INCLUDES = -I$(OS)/$(ARCH) -I$(srcdir)/$(OS)/$(ARCH) -I$(OS) -I$(srcdir)/$(OS)

SUBDIRS = $(OSARCH)
DIST_SUBDIRS = test freebsd/i386 sunos4 svr4 linux

strace_SOURCES = strace.c version.c syscall.c util.c desc.c file.c ipc.c \
		 io.c ioctl.c mem.c net.c process.c bjm.c \
		 resource.c signal.c sock.c system.c term.c time.c \
		 proc.c stream.c

EXTRA_DIST = $(man_MANS) errnoent.sh signalent.sh syscallent.sh ioctlsort.c
