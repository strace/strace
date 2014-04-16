# Automake input for strace tests.

AM_CFLAGS = $(WARN_CFLAGS)

check_PROGRAMS = net-accept-connect set_ptracer_any sigaction uio

uio_CFLAGS = $(AM_CFLAGS) -D_FILE_OFFSET_BITS=64

TESTS = \
	ptrace_setoptions.test \
	strace-f.test \
	qual_syscall.test \
	sigaction.test \
	stat.test \
	net.test \
	net-fd.test \
	uio.test \
	detach-sleeping.test \
	detach-stopped.test \
	detach-running.test

net-fd.log: net.log

TEST_LOG_COMPILER = $(srcdir)/run.sh

EXTRA_DIST = init.sh run.sh sigaction.awk $(TESTS)

CLEANFILES = $(TESTS:=.tmp)
