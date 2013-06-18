# Automake input for strace tests.

AM_CFLAGS = $(WARN_CFLAGS)

check_PROGRAMS = net-accept-connect

TESTS = ptrace_setoptions strace-f qual_syscall stat net

LOG_COMPILER = $(srcdir)/run.sh

EXTRA_DIST = init.sh run.sh $(TESTS)

CLEANFILES = $(TESTS:=.tmp)
