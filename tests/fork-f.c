/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

static int
logit(const char *const str)
{
	return pwrite(-1, str, strlen(str), 0) >= 0;
}

int main(int ac, char **av, char **ep)
{
	if (ac < 1)
		return 1;
	if (ac > 1)
		return logit(av[1]);

	logit("start");

	int fds[2];
	(void) close(0);
	if (pipe(fds)) {
		perror("pipe");
		return 77;
	}

	pid_t pid = fork();

	if (pid < 0) {
		perror("fork");
		return 77;
	}

	if (!pid) {
		close(fds[1]);

		if (read(0, fds, sizeof(int)))
			_exit(2);

		char *const args[] = { av[0], (char *) "exec", NULL };
		if (logit("child") || execve(args[0], args, args + 1))
			_exit(2);
	}

	close(0);

	logit("parent");
	close(fds[1]);

	int status;
	if (wait(&status) != pid) {
		perror("wait");
		return 77;
	}
	if (status) {
		fprintf(stderr, "status = %d\n", status);
		return 77;
	}

	logit("finish");

	pid_t ppid = getpid();
	close(-1);
	printf("%-5d pwrite64(-1, \"start\", 5, 0) = -1 EBADF (%m)\n"
	       "%-5d pwrite64(-1, \"parent\", 6, 0) = -1 EBADF (%m)\n"
	       "%-5d pwrite64(-1, \"child\", 5, 0) = -1 EBADF (%m)\n"
	       "%-5d pwrite64(-1, \"exec\", 4, 0) = -1 EBADF (%m)\n"
	       "%-5d pwrite64(-1, \"finish\", 6, 0) = -1 EBADF (%m)\n",
	       ppid, ppid, pid, pid, ppid);
	return 0;
}
