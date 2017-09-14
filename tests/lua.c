#include "tests.h"

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/uio.h>
#include <signal.h>
#include <errno.h>

static volatile int got_sig = 0;

static void
handler(int sig)
{
	got_sig = 1;
}

static void
expect_sigusr1_once(void)
{
	static bool first = true;
	if (first) {
		assert(got_sig);
		got_sig = 0;
		tprintf("--- SIGUSR1 {si_signo=SIGUSR1, si_code=SI_KERNEL} "
			"---\n");
		first = false;
	} else
		assert(!got_sig);
}

int
main(int argc, char **argv)
{
	tprintf("%s", "");

	const struct sigaction act = { .sa_handler = handler };
	if (sigaction(SIGUSR1, &act, NULL))
		perror_msg_and_fail("sigaction");

	sigset_t mask;
	sigemptyset(&mask);
	sigaddset(&mask, SIGUSR1);
	if (sigprocmask(SIG_UNBLOCK, &mask, NULL))
		perror_msg_and_fail("sigprocmask");

	bool expect_sigusr1 = false;
	bool expect_epipe = false;

	int curarg;
	for (curarg = 1; curarg < argc; ++curarg) {
		if (strcmp(argv[curarg], "-EPIPE") == 0)
			expect_epipe = true;
		else if (strcmp(argv[curarg], "-SIGUSR1") == 0)
			expect_sigusr1 = true;
		else
			break;
	}
	assert(argc - curarg == 2);
	char *towrite = argv[curarg++];
	size_t ntowrite = strlen(towrite);
	char *toread = argv[curarg++];
	size_t ntoread = strlen(toread);

	int fds[2];
	if (pipe(fds) < 0)
		perror_msg_and_fail("pipe");

	if (expect_epipe) {
		assert(writev(fds[1], (const struct iovec [1]) {{
				.iov_base = towrite,
				.iov_len = ntowrite,
			}}, 1) == -1 && errno == EPIPE);
		tprintf("writev(%d, [{iov_base=\"%s\", iov_len=%zu}], 1) = "
			"-1 EPIPE (%s) (INJECTED)\n",
			fds[1], towrite, ntowrite, strerror(EPIPE));
		if (expect_sigusr1)
			expect_sigusr1_once();
	}

	assert(writev(fds[1], (const struct iovec [1]) {{
				.iov_base = towrite,
				.iov_len = ntowrite,
			}}, 1) == (ssize_t) ntowrite);
	tprintf("writev(%d, [{iov_base=\"%s\", iov_len=%zu}], 1) = %zu\n",
		fds[1], towrite, ntowrite, ntowrite);
	if (expect_sigusr1)
		expect_sigusr1_once();

	if (close(fds[1]) < 0)
		perror_msg_and_fail("close");

	char *buf = malloc(ntoread + 1);
	if (!buf)
		perror_msg_and_fail("malloc");

	assert(readv(fds[0], (const struct iovec [1]) {{
			.iov_base = buf,
			.iov_len = ntoread + 1,
		}}, 1) == (ssize_t) ntoread);
	if (ntoread && memcmp(buf, toread, ntoread) != 0) {
		buf[ntoread] = '\0';
		error_msg_and_fail("expected to read '%s', got '%s'",
			toread, buf);
		return 1;
	}
	tprintf("readv(%d, [{iov_base=\"%s\", iov_len=%zu}], 1) = %zu%s\n",
		fds[0], toread, ntoread + 1, ntoread,
		ntoread == ntowrite ? "" : " (INJECTED)");

	tprintf("+++ exited with 0 +++\n");
	return 0;
}
