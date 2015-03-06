# Automake input for strace.

if HAVE_M32_RUNTIME
TESTS_M32 = tests-m32
endif
if HAVE_MX32_RUNTIME
TESTS_MX32 = tests-mx32
endif
SUBDIRS = tests $(TESTS_M32) $(TESTS_MX32)

bin_PROGRAMS = strace
man_MANS = strace.1
bin_SCRIPTS = strace-graph strace-log-merge

OS		= linux
# ARCH is `i386', `m68k', `sparc', etc.
ARCH		= @arch@

ACLOCAL_AMFLAGS = -I m4
AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(builddir)/$(OS)/$(ARCH) \
	      -I$(srcdir)/$(OS)/$(ARCH) \
	      -I$(builddir)/$(OS) \
	      -I$(srcdir)/$(OS)

include xlat/Makemodule.am

strace_SOURCES =	\
	access.c	\
	affinity.c	\
	aio.c		\
	bjm.c		\
	block.c		\
	cacheflush.c	\
	capability.c	\
	caps0.h		\
	caps1.h		\
	chdir.c		\
	chmod.c		\
	clone.c		\
	count.c		\
	desc.c		\
	dirent.c	\
	evdev.c		\
	execve.c	\
	exit.c		\
	fadvise.c	\
	fallocate.c	\
	fanotify.c	\
	fchownat.c	\
	file.c		\
	futex.c		\
	get_robust_list.c \
	getcpu.c	\
	getcwd.c	\
	getrandom.c	\
	hostname.c	\
	inotify.c	\
	io.c		\
	ioctl.c		\
	ioprio.c	\
	ipc.c		\
	kexec.c		\
	keyctl.c	\
	ldt.c		\
	link.c		\
	loop.c		\
	lseek.c		\
	mem.c		\
	mknod.c		\
	mount.c		\
	mtd.c		\
	net.c		\
	open.c		\
	or1k_atomic.c	\
	pathtrace.c	\
	personality.c	\
	prctl.c		\
	printmode.c	\
	printstat.h	\
	process.c	\
	process_vm.c	\
	ptp.c		\
	ptrace.h	\
	quota.c		\
	readahead.c	\
	readlink.c	\
	reboot.c	\
	regs.h		\
	renameat.c	\
	resource.c	\
	sched.c		\
	scsi.c		\
	seccomp.c	\
	signal.c	\
	sigreturn.c	\
	sock.c		\
	socketutils.c	\
	sram_alloc.c	\
	statfs.c	\
	strace.c	\
	stream.c	\
	swapon.c	\
	sync_file_range.c \
	syscall.c	\
	sysctl.c	\
	sysinfo.c	\
	syslog.c	\
	sysmips.c	\
	term.c		\
	time.c		\
	truncate.c	\
	uid.c		\
	uid16.c		\
	umask.c		\
	umount.c	\
	uname.c		\
	util.c		\
	utime.c		\
	utimes.c	\
	v4l2.c		\
	vsprintf.c	\
	wait.c		\
	xattr.c

if USE_LIBUNWIND
strace_SOURCES += unwind.c
strace_CPPFLAGS = $(AM_CPPFLAGS) $(libunwind_CPPFLAGS)
strace_LDFLAGS = $(libunwind_LDFLAGS)
strace_LDADD = $(libunwind_LIBS)
else
strace_CPPFLAGS = $(AM_CPPFLAGS)
endif

noinst_HEADERS = defs.h
# Enable this to get link map generated
#strace_CFLAGS = $(AM_CFLAGS) -Wl,-Map=strace.mapfile

EXTRA_DIST =				\
	$(man_MANS)			\
	.version			\
	COPYING				\
	CREDITS				\
	ChangeLog			\
	ChangeLog-CVS			\
	README-linux-ptrace		\
	debian/changelog		\
	debian/compat			\
	debian/control			\
	debian/copyright		\
	debian/rules			\
	debian/source/format		\
	debian/strace-udeb.install	\
	debian/strace.docs		\
	debian/strace.examples		\
	debian/strace.install		\
	debian/strace.manpages		\
	debian/strace64.install		\
	debian/strace64.manpages	\
	debian/watch			\
	errnoent.sh			\
	ioctlsort.c			\
	linux/32/ioctls_inc.h		\
	linux/32/syscallent.h		\
	linux/64/ioctls_inc.h		\
	linux/64/syscallent.h		\
	linux/aarch64/errnoent1.h	\
	linux/aarch64/ioctls_arch0.h	\
	linux/aarch64/ioctls_arch1.h	\
	linux/aarch64/ioctls_inc0.h	\
	linux/aarch64/ioctls_inc1.h	\
	linux/aarch64/signalent1.h	\
	linux/aarch64/syscallent.h	\
	linux/aarch64/syscallent1.h	\
	linux/alpha/arch_regs.h		\
	linux/alpha/errnoent.h		\
	linux/alpha/ioctls_arch0.h	\
	linux/alpha/ioctls_inc0.h	\
	linux/alpha/signalent.h		\
	linux/alpha/syscallent.h	\
	linux/alpha/userent.h		\
	linux/arc/ioctls_arch0.h	\
	linux/arc/ioctls_inc0.h		\
	linux/arc/syscallent.h		\
	linux/arch_regs.h		\
	linux/arm/arch_regs.h		\
	linux/arm/ioctls_arch0.h	\
	linux/arm/ioctls_inc0.h		\
	linux/arm/syscallent.h		\
	linux/arm/userent.h		\
	linux/avr32/ioctls_arch0.h	\
	linux/avr32/ioctls_inc0.h	\
	linux/avr32/syscallent.h	\
	linux/avr32/userent.h		\
	linux/bfin/ioctls_arch0.h	\
	linux/bfin/ioctls_inc0.h	\
	linux/bfin/syscallent.h		\
	linux/bfin/userent.h		\
	linux/crisv10/userent.h		\
	linux/crisv32/userent.h		\
	linux/dummy.h			\
	linux/errnoent.h		\
	linux/hppa/arch_regs.h		\
	linux/hppa/errnoent.h		\
	linux/hppa/ioctls_arch0.h	\
	linux/hppa/ioctls_inc0.h	\
	linux/hppa/signalent.h		\
	linux/hppa/syscallent.h		\
	linux/i386/arch_regs.h		\
	linux/i386/ioctls_arch0.h	\
	linux/i386/ioctls_inc0.h	\
	linux/i386/syscallent.h		\
	linux/i386/userent.h		\
	linux/i386/userent0.h		\
	linux/ia64/arch_regs.h		\
	linux/ia64/ioctls_arch0.h	\
	linux/ia64/ioctls_inc0.h	\
	linux/ia64/syscallent.h		\
	linux/ia64/userent.h		\
	linux/inet_diag.h		\
	linux/m68k/ioctls_arch0.h	\
	linux/m68k/ioctls_inc0.h	\
	linux/m68k/syscallent.h		\
	linux/m68k/userent.h		\
	linux/metag/ioctls_arch0.h	\
	linux/metag/ioctls_inc0.h	\
	linux/metag/syscallent.h	\
	linux/microblaze/ioctls_arch0.h	\
	linux/microblaze/ioctls_inc0.h	\
	linux/microblaze/syscallent.h	\
	linux/microblaze/userent.h	\
	linux/mips/arch_regs.h		\
	linux/mips/errnoent.h		\
	linux/mips/genstub.sh		\
	linux/mips/ioctls_arch0.h	\
	linux/mips/ioctls_inc0.h	\
	linux/mips/signalent.h		\
	linux/mips/syscallent-compat.h	\
	linux/mips/syscallent-n32.h	\
	linux/mips/syscallent-n64.h	\
	linux/mips/syscallent-o32.h	\
	linux/mips/syscallent.h		\
	linux/mips/userent.h		\
	linux/mtd-abi.h			\
	linux/or1k/ioctls_arch0.h	\
	linux/or1k/ioctls_inc0.h	\
	linux/or1k/syscallent.h		\
	linux/or1k/userent.h		\
	linux/personality.h		\
	linux/powerpc/arch_regs.h	\
	linux/powerpc/errnoent.h	\
	linux/powerpc/ioctls_arch0.h	\
	linux/powerpc/ioctls_inc0.h	\
	linux/powerpc/syscallent.h	\
	linux/powerpc/userent.h		\
	linux/powerpc64/arch_regs.h	\
	linux/powerpc64/errnoent.h	\
	linux/powerpc64/errnoent1.h	\
	linux/powerpc64/ioctls_arch0.h	\
	linux/powerpc64/ioctls_arch1.h	\
	linux/powerpc64/ioctls_inc0.h	\
	linux/powerpc64/ioctls_inc1.h	\
	linux/powerpc64/signalent1.h	\
	linux/powerpc64/syscallent.h	\
	linux/powerpc64/syscallent1.h	\
	linux/powerpc64/userent.h	\
	linux/ptp_clock.h		\
	linux/s390/arch_regs.h		\
	linux/s390/ioctls_arch0.h	\
	linux/s390/ioctls_inc0.h	\
	linux/s390/syscallent.h		\
	linux/s390/userent.h		\
	linux/s390/userent0.h		\
	linux/s390/userent1.h		\
	linux/s390x/arch_regs.h		\
	linux/s390x/ioctls_arch0.h	\
	linux/s390x/ioctls_inc0.h	\
	linux/s390x/syscallent.h	\
	linux/s390x/userent.h		\
	linux/sh/ioctls_arch0.h		\
	linux/sh/ioctls_inc0.h		\
	linux/sh/syscallent.h		\
	linux/sh/userent.h		\
	linux/sh/userent0.h		\
	linux/sh64/arch_regs.h		\
	linux/sh64/ioctls_arch0.h	\
	linux/sh64/ioctls_inc0.h	\
	linux/sh64/syscallent.h		\
	linux/sh64/userent.h		\
	linux/signalent.h		\
	linux/sock_diag.h		\
	linux/sparc/arch_regs.h		\
	linux/sparc/dummy2.h		\
	linux/sparc/errnoent.h		\
	linux/sparc/errnoent1.h		\
	linux/sparc/gen.pl		\
	linux/sparc/ioctlent1.h		\
	linux/sparc/ioctls_arch0.h	\
	linux/sparc/ioctls_inc0.h	\
	linux/sparc/signalent.h		\
	linux/sparc/signalent1.h	\
	linux/sparc/syscall1.h		\
	linux/sparc/syscallent.h	\
	linux/sparc/syscallent1.h	\
	linux/sparc/userent.h		\
	linux/sparc64/arch_regs.h	\
	linux/sparc64/dummy2.h		\
	linux/sparc64/errnoent.h	\
	linux/sparc64/errnoent1.h	\
	linux/sparc64/errnoent2.h	\
	linux/sparc64/ioctlent1.h	\
	linux/sparc64/ioctls_arch0.h	\
	linux/sparc64/ioctls_arch2.h	\
	linux/sparc64/ioctls_inc0.h	\
	linux/sparc64/ioctls_inc2.h	\
	linux/sparc64/signalent.h	\
	linux/sparc64/signalent1.h	\
	linux/sparc64/signalent2.h	\
	linux/sparc64/syscall1.h	\
	linux/sparc64/syscallent.h	\
	linux/sparc64/syscallent1.h	\
	linux/sparc64/syscallent2.h	\
	linux/sparc64/userent.h		\
	linux/subcall.h			\
	linux/syscall.h			\
	linux/tile/arch_regs.h		\
	linux/tile/errnoent1.h		\
	linux/tile/ioctls_arch0.h	\
	linux/tile/ioctls_arch1.h	\
	linux/tile/ioctls_inc0.h	\
	linux/tile/ioctls_inc1.h	\
	linux/tile/signalent1.h		\
	linux/tile/syscallent.h		\
	linux/tile/syscallent1.h	\
	linux/tile/userent.h		\
	linux/ubi-user.h		\
	linux/unix_diag.h		\
	linux/userent.h			\
	linux/userent0.h		\
	linux/x32/arch_regs.h		\
	linux/x32/errnoent1.h		\
	linux/x32/ioctls_arch0.h	\
	linux/x32/ioctls_arch1.h	\
	linux/x32/ioctls_inc0.h		\
	linux/x32/ioctls_inc1.h		\
	linux/x32/signalent1.h		\
	linux/x32/syscallent.h		\
	linux/x32/syscallent1.h		\
	linux/x32/userent.h		\
	linux/x86_64/arch_regs.h	\
	linux/x86_64/errnoent1.h	\
	linux/x86_64/errnoent2.h	\
	linux/x86_64/gentab.pl		\
	linux/x86_64/ioctls_arch0.h	\
	linux/x86_64/ioctls_arch1.h	\
	linux/x86_64/ioctls_arch2.h	\
	linux/x86_64/ioctls_inc0.h	\
	linux/x86_64/ioctls_inc1.h	\
	linux/x86_64/ioctls_inc2.h	\
	linux/x86_64/signalent1.h	\
	linux/x86_64/signalent2.h	\
	linux/x86_64/syscallent.h	\
	linux/x86_64/syscallent1.h	\
	linux/x86_64/syscallent2.h	\
	linux/x86_64/userent.h		\
	linux/xtensa/ioctls_arch0.h	\
	linux/xtensa/ioctls_inc0.h	\
	linux/xtensa/syscallent.h	\
	linux/xtensa/userent.h		\
	maint/ioctls_gen.sh		\
	maint/ioctls_hex.sh		\
	maint/ioctls_sym.sh		\
	maint/print_ioctlent.c		\
	signalent.sh			\
	strace-graph			\
	strace-log-merge		\
	strace.spec			\
	syscallent.sh			\
	$(XLAT_INPUT_FILES)		\
	$(XLAT_HEADER_FILES)		\
	xlat/gen.sh			\
	xlate.el

.PHONY: srpm
srpm: dist-xz
	rpmbuild --define '%_srcrpmdir .' -ts $(distdir).tar.xz

BUILT_SOURCES = .version

$(srcdir)/.version:
	$(AM_V_GEN)echo $(VERSION) > $@-t && mv $@-t $@

dist-hook:
	$(AM_V_GEN)echo $(VERSION) > $(distdir)/.tarball-version

today = $(shell date +%Y-%m-%d)
version_regexp = $(subst .,\.,$(VERSION))
news_check_regexp = 'Noteworthy changes in release $(version_regexp) ($(today))'

news-check: NEWS
	$(AM_V_GEN)if head -1 $< |				\
	  grep -x $(news_check_regexp) >/dev/null; then		\
		:;						\
	else							\
		echo >&2 '$<: check failed';			\
		exit 1;						\
	fi

ioctlsort_CC = $(CC_FOR_BUILD)
ioctlsort_DEFS = $(DEFS)
ioctlsort_INCLUDES = $(DEFAULT_INCLUDES) $(INCLUDES)
ioctlsort_CPPFLAGS = $(AM_CPPFLAGS) $(CPPFLAGS_FOR_BUILD)
ioctlsort_CFLAGS = $(AM_CFLAGS) $(CFLAGS_FOR_BUILD)
ioctlsort_LDFLAGS = $(AM_LDFLAGS) $(LDFLAGS_FOR_BUILD)

ioctls_inc_h = $(wildcard $(srcdir)/$(OS)/$(ARCH)/ioctls_inc*.h)
ioctlent_h = $(patsubst $(srcdir)/$(OS)/$(ARCH)/ioctls_inc%,ioctlent%,$(ioctls_inc_h))
BUILT_SOURCES += $(ioctlent_h)
CLEANFILES = $(ioctlent_h)

ioctlent%.h: ioctlsort%
	./$< > $@

ioctlsort%$(BUILD_EXEEXT): ioctlsort%.o
	$(ioctlsort_CC) $(ioctlsort_CFLAGS) $(ioctlsort_LDFLAGS) $< -o $@

ioctlsort%.o: ioctls_all%.h $(srcdir)/ioctlsort.c
	$(ioctlsort_CC) $(ioctlsort_DEFS) $(ioctlsort_INCLUDES) $(ioctlsort_CPPFLAGS) $(ioctlsort_CFLAGS) -DIOCTLSORT_INC=\"$<\" -c -o $@ $(srcdir)/ioctlsort.c

ioctls_all%.h: $(srcdir)/$(OS)/$(ARCH)/ioctls_inc%.h $(srcdir)/$(OS)/$(ARCH)/ioctls_arch%.h
	cat $^ > $@

if MAINTAINER_MODE

gen_changelog_start_date = 2009-07-08 20:00
$(srcdir)/ChangeLog: $(srcdir)/gitlog-to-changelog $(srcdir)/Makefile.in \
		     $(srcdir)/.version
	@rm -f $@.new
	(cd $(srcdir); \
	 ./gitlog-to-changelog --append-dot \
	   --since='$(gen_changelog_start_date)'; \
	 echo; echo; echo 'See ChangeLog-CVS for older changes.' \
	) > $@.new
	chmod 444 $@.new
	mv -f $@.new $@

$(srcdir)/CREDITS: $(srcdir)/CREDITS.in $(srcdir)/.mailmap \
		   $(srcdir)/Makefile.in $(srcdir)/.version
	$(AM_V_GEN)							\
	(								\
	  cd $(srcdir);							\
	  sed '/^##/,$$d' CREDITS.in;					\
	  { sed -n '1,/^##>/d; s/  \+/\t/; s/^./&/p' CREDITS.in;	\
	    git log --pretty=format:'%aN	%aE';			\
	  } | LC_ALL=C sort -u						\
	    | awk -F'\t' '{printf("\t%s <%s>\n",$$1,$$2)}';		\
	) > $@-t && mv $@-t $@

export TAR_OPTIONS = --owner=0 --group=0 --numeric-owner --mode=go-w,go+rX

endif
