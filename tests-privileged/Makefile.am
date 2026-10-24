# Automake input for privileged strace tests.
#
# Copyright (c) 2026 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

ARCH = @arch@
NATIVE_ARCH = @arch_native@
SIZEOF_KERNEL_LONG_T = @SIZEOF_KERNEL_LONG_T@
SIZEOF_LONG = @SIZEOF_LONG@

TESTS = \
	bpf-obj_get_info_by_fd.test \
	bpf-obj_get_info_by_fd-link.test \
	bpf-obj_get_info_by_fd-prog.test \
	bpf-obj_get_info_by_fd-prog-v.test \
	bpf-obj_get_info_by_fd-v.test

privileged_programs = $(TESTS:.test=)

$(privileged_programs):
	$(MAKE) -C ../tests $@
	ln -s ../tests/$@ $@

$(TESTS:.test=.log): $(privileged_programs)

TEST_LOG_COMPILER = $(SHELL) $(srcdir)/run-test.sh $(abs_top_srcdir)/tests
AM_TEST_LOG_FLAGS = STRACE_ARCH=$(ARCH) STRACE_NATIVE_ARCH=$(NATIVE_ARCH) \
		    SIZEOF_KERNEL_LONG_T=$(SIZEOF_KERNEL_LONG_T) \
		    SIZEOF_LONG=$(SIZEOF_LONG) MIPS_ABI=$(MIPS_ABI) \
		    $(abs_top_srcdir)/tests/run.sh

check_SCRIPTS = run-test.sh
EXTRA_DIST = $(TESTS) $(check_SCRIPTS)
CLEANFILES = $(privileged_programs)

clean-local:
	-rm -rf -- $(privileged_programs:=.dir)
