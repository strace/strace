# Automake input for asinfo tests.
#
# Copyright (c) 2017 Edgar Kaziakhmedov <edgar.kaziakhmedov@virtuozzo.com>
# Copyright (c) 2017-2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

OS = linux
AM_COLOR_TESTS = always
AM_CFLAGS = $(WARN_CFLAGS)
AM_CPPFLAGS = $(ARCH_MFLAGS) \
	      -I$(builddir) \
	      -I$(builddir)/.. \
	      -I$(top_builddir) \
	      -I$(top_srcdir)
AM_LDFLAGS = $(ARCH_MFLAGS)

check_PROGRAMS = set_arch set_mult_arch list_arch set_abi set_mult_abi \
		 get_arch_abi get_sname get_snum com_disp_wrong_keys \
		 multiarch_get_sname multiarch_get_snum format_output
TESTS = set_arch.test set_mult_arch.test list_arch.test set_abi.test \
	set_mult_abi.test get_arch_abi.test get_sname.test get_snum.test \
	com_disp_wrong_keys.test multiarch_get_sname.test \
	multiarch_get_snum.test format_output.test

set_arch_SOURCES = set_arch.c
set_mult_arch_SOURCES = set_mult_arch.c
list_arch_SOURCES = list_arch.c
set_abi_SOURCES = set_abi.c
set_mult_abi_SOURCES = set_mult_abi.c
get_arch_abi_SOURCES = get_arch_abi.c
get_sname_SOURCES = get_sname.c
get_snum_SOURCES = get_snum.c
com_disp_wrong_keys_SOURCES = com_disp_wrong_keys.c
multiarch_get_sname_SOURCES = multiarch_get_sname.c
multiarch_get_snum_SOURCES = multiarch_get_snum.c
format_output_SOURCES = format_output.c

EXTRA_DIST = $(TESTS) init.sh ref_asinfo_output.h

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	-rm -rf -- $(TESTS:.test=.dir)
