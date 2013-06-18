# Automake input for strace tests.

AM_CFLAGS = $(WARN_CFLAGS)

check_PROGRAMS = net-accept-connect set_ptracer_any

TESTS = ptrace_setoptions strace-f qual_syscall stat net \
	detach-sleeping detach-stopped

LOG_COMPILER = $(srcdir)/run.sh

EXTRA_DIST = init.sh run.sh $(TESTS)

CLEANFILES = $(TESTS:=.tmp)
