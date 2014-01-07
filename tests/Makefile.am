# Automake input for strace tests.

AM_CFLAGS = $(WARN_CFLAGS)

check_PROGRAMS = net-accept-connect set_ptracer_any sigaction

TESTS = ptrace_setoptions strace-f qual_syscall sigaction.sh stat net \
	detach-sleeping detach-stopped detach-running

LOG_COMPILER = $(srcdir)/run.sh

EXTRA_DIST = init.sh run.sh sigaction.awk $(TESTS)

CLEANFILES = $(TESTS:=.tmp)
