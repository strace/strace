/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_open

# include <asm/fcntl.h>
# include <stdio.h>
# include <unistd.h>

# include "secontext.h"

int
main(void)
{
	/*
	 * Make sure the current workdir of the tracee
	 * is different from the current workdir of the tracer.
	 */
	create_and_enter_subdir("open_subdir");

	static const char sample[] = "open@json.sample";
	char *my_secontext = SECONTEXT_PID_MY();

	static char se_buf[4096];
	if (my_secontext[0] != '\0')
		snprintf(se_buf, sizeof(se_buf), "\"secontext\": \"%s\", ", my_secontext);

	puts("[");
	long fd = syscall(__NR_open, sample, O_RDONLY|O_CREAT, 0400);

	if (fd != -1) {
		printf("{ %s"
				"\"" SYSCALL_FIELD_NAME "\": \"%s\", "
				"\"args\": ["
					"\"%s\", \"O_RDONLY|O_CREAT\", 0400"
				"], "
				"\"return\": %ld"
				"%s},\n",
	          se_buf,
	          "open",
	          sample,
	          fd,
	          SECONTEXT_FILE(sample));

		close(fd);
		if (unlink(sample))
			perror_msg_and_fail("unlink");

		fd = syscall(__NR_open, sample, O_RDONLY);
		printf("{ %s"
				"\"" SYSCALL_FIELD_NAME "\": \"%s\", "
				"\"args\": ["
					"\"%s\", \"O_RDONLY\""
				"], "
				"\"return\": %ld,"
				"\"error\": \"%s\","
				"\"strerror\": \"%m\""
				"},\n",
	          se_buf,
	          "open",
	          sample,
	          fd,
	          errno2name());

		fd = syscall(__NR_open, sample, O_WRONLY|O_NONBLOCK|0x80000000);
		printf("{ %s"
				"\"" SYSCALL_FIELD_NAME "\": \"%s\", "
				"\"args\": ["
					"\"%s\", \"O_RDONLY|O_NONBLOCK|0x80000000\""
				"], "
				"\"return\": %ld,"
				"\"error\": \"%s\","
				"\"strerror\": \"%m\""
				"},\n",
	          se_buf,
	          "open",
	          sample,
	          fd,
	          errno2name());
	} else {
		printf("{ %s"
				"\"" SYSCALL_FIELD_NAME "\": \"%s\", "
				"\"args\": ["
					"\"%s\", \"O_RDONLY|O_CREAT\", 0400"
				"], "
				"\"return\": %ld,"
				"\"error\": \"%s\","
				"\"strerror\": \"%m\""
				"%s},\n",
	          se_buf,
	          "open",
	          sample,
	          fd,
	          errno2name(),
	          SECONTEXT_FILE(sample));
	}

# ifdef O_TMPFILE
	fd = syscall(__NR_open, sample, O_WRONLY|O_TMPFILE, 0600);
	printf("{ %s"
			"\"" SYSCALL_FIELD_NAME "\": \"%s\", "
			"\"args\": ["
				"\"%s\", \"O_WRONLY|O_TMPFILE\", 0600"
			"], "
			"\"return\": %ld,"
			"\"error\": \"%s\","
			"\"strerror\": \"%m\""
			"},\n",
	        se_buf,
	        "open",
	        sample,
	        fd,
	        errno2name());
# endif /* O_TMPFILE */

	leave_and_remove_subdir();

	puts("{\"exited\": 0}]");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_open")

#endif
