/* This test is not yet added to Makefile */

#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>

static int thd_no;

static void *sub_thd(void *c)
{
	fprintf(stderr, "sub-thread %d created\n", ++thd_no);
	for (;;)
		getuid();
	return NULL;
}

int main(int argc, char **argv)
{
	int i;
	pthread_t *thd;
	int num_threads = 1;

	if (argv[1])
		num_threads = atoi(argv[1]);

	thd = malloc(num_threads * sizeof(thd[0]));
	fprintf(stderr, "test start, num_threads:%d...\n", num_threads);
	for (i = 0; i < num_threads; i++) {
		pthread_create(&thd[i], NULL, sub_thd, NULL);
		fprintf(stderr, "after pthread_create\n");
	}
	/* Exit. This kills all threads */
	return 0;
}
