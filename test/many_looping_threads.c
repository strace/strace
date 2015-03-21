// This testcase, when run with large number of threads
// under stace -f, may never finish because strace does not
// ensure any fairness in thread scheduling:
// it restarts threads as they stop. If daughter threads crowd out
// the "mother" and _they_ get continually restarted by strace,
// the end of spawning loop will never be reached.
//
// Also, it is a testcase which triggers the
// "strace: Exit of unknown pid 32457 seen"
// message when on testcase exit, strace sees deaths of newly-attached
// threads _before_ their first syscall stop.
//
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

static int thd_no;

static void *sub_thd(void *c)
{
	dprintf(1, "sub-thread %d created\n", ++thd_no);
	for (;;)
		getuid();
	return NULL;
}

int main(int argc, char *argv[])
{
	int i;
	pthread_t *thd;
	int num_threads = 1;

	if (argv[1])
		num_threads = atoi(argv[1]);

	thd = malloc(num_threads * sizeof(thd[0]));
	dprintf(1, "test start, num_threads:%d...\n", num_threads);

	for (i = 0; i < num_threads; i++) {
		pthread_create(&thd[i], NULL, sub_thd, NULL);
		dprintf(1, "after pthread_create\n");
	}

	/* Exit. This kills all threads */
	return 0;
}
