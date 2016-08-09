/*
 * Create NUM_THREADS threads which print "1" and sleep in pause().
 * Then create another thread which prints "2", and re-execs the program.
 * The leader then either sleeps in pause(), or exits if $LEADER_EXIT is set.
 * This triggers "execve'ed thread replaces thread leader" case.
 *
 * gcc -Wall -Os -o threaded_execve threaded_execve.c
 *
 * Try running it under strace like this:
 *
 * # Should not be confused by traced execve-ing thread
 * # replacing traced leader:
 * strace -oLOG -f ./threaded_execve
 *
 * # Same, but different output mode. Output after execve
 * # should go into leader's LOG.<pid> file, not into execve'ed
 * # thread's log file:
 * strace -oLOG -ff ./threaded_execve
 *
 * # Should not be confused by non-traced execve-ing thread
 * # replacing traced leader:
 * strace -oLOG ./threaded_execve
 * ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
 * In Linux 3.2, non-traced execve-ing thread does not
 * become traced after execve, even though it has pid == leader's pid
 * after execve. And yet, strace's waitpid doesn't return ECHILD.
 *
 * # Run for NUM seconds, not just one second.
 * # Watch top to check for memory leaks in strace:
 * strace -oLOG -f ./threaded_execve <NUM>
 *
 */
#define NUM_THREADS 1

#define _GNU_SOURCE 1
#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <sched.h>
#include <signal.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <asm/unistd.h>

/* Define clone2 for all arches */
#ifdef __ia64__
extern int __clone2(int (*fn) (void *), void *child_stack_base,
		size_t stack_size, int flags, void *arg, ...);
#define clone2 __clone2
#elif defined(__metag__)
#define clone2(func, stack_base, size, flags, arg...) \
        clone(func, stack_base, flags, arg)
#else
#define clone2(func, stack_base, size, flags, arg...) \
        clone(func, (stack_base) + (size), flags, arg)
#endif
/* Direct calls to syscalls, avoiding libc wrappers */
#define syscall_tgkill(pid, tid, sig) syscall(__NR_tgkill, (pid), (tid), (sig))
#define syscall_getpid() syscall(__NR_getpid)
#define syscall_gettid() syscall(__NR_gettid)
#define syscall_exit(v) syscall(__NR_exit, (v));

static char my_name[PATH_MAX];
static int leader_final_action;

static int
thread1(void *unused)
{
	write(1, "1", 1);
	for(;;) pause();
	return 0;
}

static int
thread2(void *unused)
{
	char buf[64];
	sprintf(buf, "%d", leader_final_action);
	write(1, "2", 1);
	usleep(20*1000);
	/* This fails with ENOENT if leader has exited by now! :) */
	execl("/proc/self/exe", "exe", "exe", buf, NULL);
	/* So fall back to resolved name */
	execl(my_name, "exe", "exe", buf, NULL);
	for(;;) pause();
	return 0;
}

static void
thread_leader(void)
{
	/* malloc gives sufficiently aligned buffer.
	 * long buf[] does not! (on ia64).
	 */
	int cnt = NUM_THREADS;
	while (--cnt >= 0) {
		/* As seen in pthread_create(): */
		clone2(thread1, malloc(16 * 1024), 16 * 1024, 0
			| CLONE_VM
			| CLONE_FS
			| CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM
			| 0        /* no signal to send on death */
			, NULL);
		usleep(20*1000);
	}
	clone2(thread2, malloc(16 * 1024), 16 * 1024, 0
		| CLONE_VM
		| CLONE_FS
		| CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD | CLONE_SYSVSEM
		| 0        /* no signal to send on death */
		, NULL);

	/* Various states leader can be while other thread execve's: */
	switch (leader_final_action % 3) {
		case 0: syscall_exit(42); /* leader is dead */
		case 1: for(;;) pause(); /* leader is in syscall */
		default: for(;;) continue; /* leader is in userspace */
	}
}

int
main(int argc, char **argv)
{
	if (readlink("/proc/self/exe", my_name, sizeof(my_name)-1) <= 0)
		return 1;

	setbuf(stdout, NULL);

	if (argv[1] && strcmp(argv[1], "exe") == 0) {
		leader_final_action = atoi(argv[2]) + 1;
		thread_leader();
	}

	printf("%d: thread leader\n", getpid());

	alarm(argv[1] ? atoi(argv[1]) : 1);
	thread_leader();

        return 0;
}
