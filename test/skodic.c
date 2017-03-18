/*
 * This demonstrates races: kernel may actually open other file
 * than you read in the strace output.
 * If you see a successfull open of /etc/shadow,
 * you know you've seen a race.
 *
 * $ gcc -Wall -O0 skodic.c -o skodic
 * $ timeout 0.1 ../strace -yeopen -o'|grep "shadow.*= [0-9]"' ./skodic
 */

#ifndef _GNU_SOURCE
# define  _GNU_SOURCE 1
#endif

#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

int
main(void)
{
	FILE *fp = tmpfile();
	if (!fp)
		error(1, errno, "tmpfile");

	int fd = fileno(fp);
	size_t size = sysconf(_SC_PAGESIZE);

	if (ftruncate(fd, size))
		error(1, errno, "ftruncate");

	char *p = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	if (p == MAP_FAILED)
		error(1, errno, "mmap");
	fclose(fp);
	fp = NULL;
	fd = -1;

	strcpy(p, "/etc/shadow");
	if (open(p, 0) >= 0)
		error(1, 0, p);

	pid_t pid = fork();
	if (pid < 0)
		error(1, errno, "fork");

	if (!pid) {
		for (;;) {
			strcpy(p, "/etc/passwd");
			strcpy(p, "/etc/shadow");
		}
	} else {
		for (;;) {
			if ((fd = open(p, 0)) >= 0)
				close(fd);
		}
	}

	return 0;
}
