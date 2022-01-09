# Automake input for strace.
#
# Copyright (c) 2002-2009 Roland McGrath <roland@redhat.com>
# Copyright (c) 2006-2016 Dmitry V. Levin <ldv@strace.io>
# Copyright (c) 2008-2015 Mike Frysinger <vapier@gentoo.org>
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
# Copyright (c) 2002-2022 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

if HAVE_M32_RUNTIME
TESTS_M32 = tests-m32
endif
if HAVE_MX32_RUNTIME
TESTS_MX32 = tests-mx32
endif
SUBDIRS = bundled src tests $(TESTS_M32) $(TESTS_MX32)

man_MANS = doc/strace.1 doc/strace-log-merge.1

ACLOCAL_AMFLAGS = -I m4 -I src/xlat

@CODE_COVERAGE_RULES@
CODE_COVERAGE_BRANCH_COVERAGE = 1
CODE_COVERAGE_GENHTML_OPTIONS = $(CODE_COVERAGE_GENHTML_OPTIONS_DEFAULT) \
	--title "$(PACKAGE_NAME)-$(PACKAGE_VERSION)" \
	--precision 2 --missed \
	--prefix $(abs_top_srcdir) \
	--prefix $(abspath $(abs_top_srcdir))
CODE_COVERAGE_IGNORE_PATTERN = '/usr/include/*' '*/tests/*' '*/tests-m32/*' '*/tests-mx32/*'

EXTRA_DIST =				\
	.version			\
	COPYING				\
	CREDITS				\
	ChangeLog			\
	ChangeLog-CVS			\
	LGPL-2.1-or-later		\
	README-configure		\
	build-aux/copyright-year-gen	\
	build-aux/file-date-gen		\
	build-aux/git-version-gen	\
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
	doc/README-linux-ptrace		\
	strace.spec			\
	# end of EXTRA_DIST

.PHONY: check-valgrind-local
check-valgrind-local:

.PHONY: srpm
srpm: dist-xz
	rpmbuild --define '%_srcrpmdir .' -ts $(distdir).tar.xz

$(srcdir)/.version:
	$(AM_V_GEN)echo $(VERSION) > $@-t && mv $@-t $@

dist-hook:
	$(AM_V_GEN)echo $(VERSION) > $(distdir)/.tarball-version
	${AM_V_GEN}echo $(COPYRIGHT_YEAR) > $(distdir)/.year
	${AM_V_GEN}echo $(STRACE_MANPAGE_DATE) \
		> $(distdir)/doc/.strace.1.in.date
	${AM_V_GEN}echo $(SLM_MANPAGE_DATE) \
		> $(distdir)/doc/.strace-log-merge.1.in.date

today = $(shell LC_TIME=C date -u +%Y-%m-%d)
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

BUILT_SOURCES = .version

clean-local:
	-rm -rf mpers-m32 mpers-mx32

if MAINTAINER_MODE

gen_changelog_start_date = 2009-07-08 20:00
$(srcdir)/ChangeLog: $(srcdir)/build-aux/gitlog-to-changelog $(srcdir)/Makefile.in \
		     $(srcdir)/.version
	@rm -f $@.new
	(cd $(srcdir); \
	 ./build-aux/gitlog-to-changelog --append-dot \
	   --since='$(gen_changelog_start_date)'; \
	 echo; echo; echo 'See ChangeLog-CVS for older changes.' \
	) > $@.new
	chmod 444 $@.new
	mv -f $@.new $@

$(srcdir)/CREDITS: $(srcdir)/CREDITS.in $(srcdir)/.mailmap \
		   $(srcdir)/Makefile.in $(srcdir)/.version
	$(AM_V_GEN)							\
	(								\
	  D="$(D)";							\
	  export D;							\
	  cd $(srcdir);							\
	  sed '/^##/,$$d' CREDITS.in;					\
	  sed -n '1,/^##>/d; s/   */\t/; s/^./&/p' CREDITS.in		\
	    | maint/gen-contributors-list.sh - -e HEAD --initial	\
	    | sed 's/\t/ /g; s/^/\t/'					\
	) > $@-t && mv $@-t $@

export TAR_OPTIONS = --owner=0 --group=0 --numeric-owner --mode=go-w,go+rX

endif
