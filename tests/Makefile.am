# Automake input for strace tests.

AM_CFLAGS = $(WARN_CFLAGS)

check_PROGRAMS = net-accept-connect set_ptracer_any sigaction

TESTS = \
	ptrace_setoptions.test \
	strace-f.test \
	qual_syscall.test \
	sigaction.test \
	stat.test \
	net.test \
	detach-sleeping.test \
	detach-stopped.test \
	detach-running.test

TEST_LOG_COMPILER = $(srcdir)/run.sh

EXTRA_DIST = init.sh run.sh sigaction.awk $(TESTS)

CLEANFILES = $(TESTS:=.tmp)
