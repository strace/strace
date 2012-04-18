# Automake input for strace.

SUBDIRS = tests

bin_PROGRAMS = strace
man_MANS = strace.1
bin_SCRIPTS = strace-graph strace-log-merge

OS		= linux
# ARCH is `i386', `m68k', `sparc', etc.
ARCH		= @arch@

ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(srcdir)/$(OS)/$(ARCH) -I$(srcdir)/$(OS) -I$(builddir)/$(OS)

strace_SOURCES = strace.c syscall.c count.c util.c desc.c file.c ipc.c \
		 io.c ioctl.c mem.c net.c process.c bjm.c quota.c \
		 resource.c signal.c sock.c system.c term.c time.c \
		 scsi.c stream.c block.c pathtrace.c mtd.c vsprintf.c \
		 loop.c
noinst_HEADERS = defs.h

EXTRA_DIST = $(man_MANS) errnoent.sh signalent.sh syscallent.sh ioctlsort.c \
	     debian/changelog debian/compat debian/control debian/copyright \
	     debian/rules debian/source/format debian/watch \
	     debian/strace64.install debian/strace64.manpages \
	     debian/strace.docs debian/strace.examples debian/strace.install \
	     debian/strace.manpages debian/strace-udeb.install \
	     strace.spec \
	     strace-graph strace-log-merge \
	     ChangeLog ChangeLog-CVS COPYRIGHT CREDITS \
	     README-linux README-linux-ptrace \
	     linux/ioctlsort.c linux/ioctlent.sh \
	     linux/ioctlent.h.in linux/errnoent.h linux/signalent.h \
	     linux/syscall.h linux/dummy.h linux/mtd-abi.h \
	     linux/i386/ioctlent.h.in linux/i386/syscallent.h \
	     linux/alpha/errnoent.h linux/alpha/ioctlent.h.in \
	     linux/alpha/signalent.h linux/alpha/syscallent.h \
	     linux/arm/ioctlent.h.in linux/arm/syscallent.h \
	     linux/arm/syscallent1.h \
	     linux/arm/signalent1.h linux/arm/ioctlent1.h \
	     linux/arm/errnoent1.h \
	     linux/avr32/ioctlent.h.in linux/avr32/syscallent.h \
	     linux/bfin/ioctlent.h.in linux/bfin/syscallent.h \
	     linux/hppa/errnoent.h linux/hppa/ioctlent.h.in \
	     linux/hppa/signalent.h linux/hppa/syscallent.h \
	     linux/ia64/ioctlent.h.in \
	     linux/ia64/signalent.h \
	     linux/ia64/syscallent.h \
	     linux/m68k/ioctlent.h.in linux/m68k/syscallent.h \
	     linux/microblaze/ioctlent.h.in linux/microblaze/syscallent.h \
	     linux/mips/ioctlent.sh linux/mips/errnoent.h \
	     linux/mips/ioctlent.h.in linux/mips/signalent.h \
	     linux/mips/syscallent.h \
	     linux/powerpc/ioctlent.h.in linux/powerpc/ioctlent1.h \
	     linux/powerpc/syscallent.h linux/powerpc/syscallent1.h \
	     linux/powerpc/errnoent1.h linux/powerpc/signalent1.h \
	     linux/s390/ioctlent.h.in \
	     linux/s390/syscallent.h \
	     linux/s390x/ioctlent.h.in \
	     linux/s390x/syscallent.h \
	     linux/sh/syscallent.h \
	     linux/sh/ioctlent.h.in \
	     linux/sh64/ioctlent.h.in linux/sh64/syscallent.h \
	     linux/sparc/dummy2.h \
	     linux/sparc/errnoent.h linux/sparc/errnoent1.h \
	     linux/sparc/ioctlent.h.in linux/sparc/ioctlent1.h \
	     linux/sparc/signalent.h linux/sparc/signalent1.h \
	     linux/sparc/syscall1.h \
	     linux/sparc/syscallent.h linux/sparc/syscallent1.h \
	     linux/sparc/gen.pl \
	     linux/sparc64/dummy2.h linux/sparc64/errnoent.h \
	     linux/sparc64/errnoent1.h linux/sparc64/errnoent2.h \
	     linux/sparc64/ioctlent.h.in linux/sparc64/ioctlent1.h \
	     linux/sparc64/ioctlent2.h linux/sparc64/signalent.h \
	     linux/sparc64/signalent1.h linux/sparc64/signalent2.h \
	     linux/sparc64/syscall1.h \
	     linux/sparc64/syscallent.h linux/sparc64/syscallent1.h \
	     linux/sparc64/syscallent2.h \
	     linux/tile/ioctlent.h.in linux/tile/syscallent.h \
	     linux/x32/errnoent.h linux/x32/ioctlent.h.in \
	     linux/x32/signalent.h linux/x32/syscallent.h \
	     linux/x32/errnoent1.h linux/x32/ioctlent1.h \
	     linux/x32/signalent1.h linux/x32/syscallent1.h \
	     linux/x86_64/ioctlent.h.in linux/x86_64/syscallent.h \
	     linux/x86_64/gentab.pl \
	     linux/x86_64/errnoent1.h linux/x86_64/ioctlent1.h \
	     linux/x86_64/signalent1.h linux/x86_64/syscallent1.h \
	     linux/x86_64/errnoent2.h linux/x86_64/ioctlent2.h \
	     linux/x86_64/signalent2.h linux/x86_64/syscallent2.h \
	     xlate.el

.PHONY: srpm
srpm: dist-xz
	rpmbuild --define '%_srcrpmdir .' -ts $(distdir).tar.xz

BUILT_SOURCES =

if MAINTAINER_MODE

gen_changelog_start_date = 2009-07-08 20:00
$(srcdir)/ChangeLog: $(top_srcdir)/gitlog-to-changelog $(srcdir)/Makefile.in \
		     $(top_srcdir)/.git/refs/heads/*
	@rm -f $@.new
	(cd $(top_srcdir); \
	 ./gitlog-to-changelog --append-dot \
	   --since='$(gen_changelog_start_date)'; \
	 echo; echo; echo 'See ChangeLog-CVS for older changes.' \
	) > $@.new
	chmod 444 $@.new
	mv -f $@.new $@

$(srcdir)/CREDITS: $(top_srcdir)/CREDITS.in $(top_srcdir)/.mailmap \
		   $(srcdir)/Makefile.in $(top_srcdir)/.git/refs/heads/*
	$(AM_V_GEN)							\
	{								\
	  cd $(top_srcdir);						\
	  sed '/^##/,$$d' CREDITS.in;					\
	  { sed -n '1,/^##>/d; s/  \+/\t/; s/^./&/p' CREDITS.in;	\
	    git log --pretty=format:'%aN	%aE';			\
	  } | LC_ALL=C sort -u						\
	    | awk -F'\t' '{printf("\t%s <%s>\n",$$1,$$2)}';		\
	} > $@-t && mv $@-t $@

export TAR_OPTIONS = --owner=0 --group=0 --numeric-owner --mode=go-w,go+rX

IOCTLDIR = /usr/include
IOCTLASM = asm

ioctlent_h_in = linux/ioctlent.h.in

BUILT_SOURCES += $(ioctlent_h_in)

$(srcdir)/$(ioctlent_h_in): ioctlsort
	$(<D)/$(<F) > $@
ioctlsort: $(srcdir)/linux/ioctlsort.c ioctls.h ioctldefs.h
	$(filter-out -I%,$(LINK.c)) -I. -I$(IOCTLDIR) $(filter -I%,$(LINK.c)) \
	  -o $@ $<
ioctls.h: $(srcdir)/linux/ioctlent.sh
	$(SHELL) $< $(IOCTLDIR) $(IOCTLASM)
ioctldefs.h: ioctls.h ;

endif

ioctlent_h = $(builddir)/$(OS)/ioctlent.h
BUILT_SOURCES += $(ioctlent_h)
CLEANFILES = $(ioctlent_h)
ioctlent_h_deps = $(srcdir)/$(OS)/ioctlent.h.in $(srcdir)/$(OS)/$(ARCH)/ioctlent.h.in
$(ioctlent_h): $(top_builddir)/config.status $(ioctlent_h_deps)
	$(MKDIR_P) $(builddir)/$(OS)
	cat $(ioctlent_h_deps) | \
		$(COMPILE) -E -P - | \
		LC_ALL=C sort -u -k3,3 -k2,2 > $@
