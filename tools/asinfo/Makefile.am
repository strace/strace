# Automake input for asinfo.
#
# Copyright (c) 2017 Edgar Kaziakhmedov <edgar.kaziakhmedov@virtuozzo.com>
# Copyright (c) 2017-2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

SUBDIRS = . tests

bin_PROGRAMS = asinfo
man_MANS = asinfo.1

OS = linux

AUTOMAKE_OPTIONS = subdir-objects

AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = -I$(builddir) \
	      -I$(top_builddir)/$(OS) \
	      -I$(top_srcdir)/$(OS) \
	      -I$(top_builddir) \
	      -I$(top_srcdir)

include Makemodule.am

asinfo_CPPFLAGS = $(AM_CPPFLAGS)
asinfo_CFLAGS = $(AM_CFLAGS)

asinfo_SOURCES =		\
	arch_definitions.h	\
	arch_interface.c	\
	$(ARCH_AUX_FILES)	\
	arch_interface.h	\
	asinfo.c		\
	../../basic_filters.c	\
	dispatchers.c		\
	dispatchers.h		\
	error_interface.c	\
	error_interface.h	\
	../../error_prints.c	\
	../../error_prints.h	\
	../../filter.h		\
	../../macros.h		\
	../../number_set.c	\
	../../number_set.h	\
	request_msgs.h		\
	../../string_to_uint.h	\
	../../string_to_uint.c	\
	syscall_interface.c	\
	syscall_interface.h	\
	../../sysent_shorthand_defs.h \
	../../sysent_shorthand_undefs.h \
	../../xmalloc.c		\
	../../xmalloc.h		\
	#end of asinfo_SOURCES

EXTRA_DIST = gen_asinfo_files.sh README-arch
