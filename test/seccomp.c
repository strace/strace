#include <stddef.h>
#include <unistd.h>
#include <errno.h>
#include <sys/prctl.h>
#include <asm/unistd.h>
#include <linux/audit.h>
#include <linux/filter.h>
#include <linux/seccomp.h>

#if defined __i386__
# define SECCOMP_ARCH AUDIT_ARCH_I386
#elif defined __x86_64__
# define SECCOMP_ARCH AUDIT_ARCH_X86_64
#elif defined __arm__
# define SECCOMP_ARCH AUDIT_ARCH_ARM
#elif defined __arm64__ || defined __aarch64__
# define SECCOMP_ARCH AUDIT_ARCH_AARCH64
#else
# error unsupported architecture
#endif

#define SOCK_FILTER_KILL_PROCESS \
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_KILL)

#define SOCK_FILTER_DENY_SYSCALL(nr, err) \
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_ ## nr, 0, 1), \
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ERRNO | (SECCOMP_RET_DATA & (err)))

#define SOCK_FILTER_ALLOW_SYSCALL(nr) \
		BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, __NR_ ## nr, 0, 1), \
		BPF_STMT(BPF_RET | BPF_K, SECCOMP_RET_ALLOW)

static const struct sock_filter filter[] = {
	/* load architecture */
	BPF_STMT(BPF_LD | BPF_W | BPF_ABS, (offsetof (struct seccomp_data, arch))),
	/* jump forward 1 instruction if architecture matches */
	BPF_JUMP(BPF_JMP | BPF_JEQ | BPF_K, SECCOMP_ARCH, 1, 0),
	/* kill process */
	SOCK_FILTER_KILL_PROCESS,

	/* load syscall number */
	BPF_STMT(BPF_LD | BPF_W | BPF_ABS, offsetof(struct seccomp_data, nr)),

	/* allow syscalls */
	SOCK_FILTER_ALLOW_SYSCALL(close),
	SOCK_FILTER_ALLOW_SYSCALL(exit),
	SOCK_FILTER_ALLOW_SYSCALL(exit_group),

	/* deny syscalls */
	SOCK_FILTER_DENY_SYSCALL(sync, EBUSY),
	SOCK_FILTER_DENY_SYSCALL(setsid, EACCES),
	SOCK_FILTER_DENY_SYSCALL(getpid, EPERM),
	SOCK_FILTER_DENY_SYSCALL(munlockall, SECCOMP_RET_DATA),

	/* kill process */
	SOCK_FILTER_KILL_PROCESS
};

static const struct sock_fprog prog = {
	.len = sizeof(filter) / sizeof(filter[0]),
	.filter = (struct sock_filter *) filter,
};

int
main(void)
{
	int fds[2];

	close(0);
	close(1);
	if (pipe(fds))
		return 77;

	if (prctl(PR_SET_NO_NEW_PRIVS, 1, 0, 0, 0))
		return 77;

	if (prctl(PR_SET_SECCOMP, SECCOMP_MODE_FILTER, &prog))
		return 77;

	if (close(0) || close(1))
		_exit(1);

#define TEST_DENIED_SYSCALL(nr, err, fail) \
	if (errno = 0, syscall(__NR_ ## nr, 0xbad, 0xf00d, 0xdead, 0xbeef, err, fail) != -1 || err != errno) \
		close(-fail)

	TEST_DENIED_SYSCALL(sync, EBUSY, 2);
	TEST_DENIED_SYSCALL(setsid, EACCES, 3);
	TEST_DENIED_SYSCALL(getpid, EPERM, 4);
	TEST_DENIED_SYSCALL(munlockall, SECCOMP_RET_DATA, 5);

	_exit(0);
}
