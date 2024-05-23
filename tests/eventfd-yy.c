/*
 * Test --decode-fds=eventfd option.
 *
 * Copyright (c) 2024 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "tests.h"

#ifdef HAVE_SYS_EVENTFD_H
# include "scno.h"
# include "xmalloc.h"

# include <inttypes.h>
# include <stdio.h>
# include <stdlib.h>
# include <string.h>
# include <unistd.h>
# include <sys/eventfd.h>

static long
k_eventfd(const unsigned int value, const unsigned int flags)
{
	const kernel_ulong_t fill = (kernel_ulong_t) 0xdefaced00000000ULL;
	const kernel_ulong_t bad = (kernel_ulong_t) 0xbadc0dedbadc0dedULL;
	const kernel_ulong_t arg1 = fill | value;
	const kernel_ulong_t arg2 = fill | flags;

	return syscall(__NR_eventfd2, arg1, arg2, bad, bad, bad, bad);
}

struct fdinfo {
	const char *search_pfx;
	size_t search_pfx_len;
	void *data;
	bool avail;
};

static void
parse_fdinfo_efd_id(const char *value, void *data)
{
	int *efd_id = data;
	*efd_id = (int) strtol(value, NULL, 10);

	if (*efd_id < 0)
		*efd_id = -1;
}

static size_t
procfs_check_avail_efd_data(pid_t pid, int fd, struct fdinfo *fdinfo_lines,
                            size_t fdinfo_array_size)
{
	char *fdinfo_path = xasprintf("/proc/%u/fdinfo/%u", pid, fd);
	FILE *f = fopen(fdinfo_path, "r");
	if (!f)
		perror_msg_and_skip("fopen: %s", fdinfo_path);
	free(fdinfo_path);

	char *line = NULL;
	size_t sz = 0;
	size_t matches = 0;

	while (getline(&line, &sz, f) > 0) {
		for (size_t i = 0; i < fdinfo_array_size; i++) {
			const char *search_pfx = fdinfo_lines[i].search_pfx;
			size_t search_pfx_len = fdinfo_lines[i].search_pfx_len;
			if (strncmp(line, search_pfx, search_pfx_len))
				continue;

			if (fdinfo_lines[i].data) {
				const char *value = line + search_pfx_len;
				parse_fdinfo_efd_id(value, fdinfo_lines[i].data);
			}
			fdinfo_lines[i].avail = true;
			matches++;
			break;
		}
	}

	free(line);
	fclose(f);

	return matches;
}

static void
print_eventfd_details(struct fdinfo *fdinfo_lines,
                      uint64_t efd_counter, int efd_semaphore)
{
	if (fdinfo_lines[0].avail) {
		printf("<{eventfd-count=%#" PRIx64, efd_counter);

		int efd_id = *(int *) fdinfo_lines[1].data;
		if (efd_id != -1) {
			printf(", eventfd-id=%d", efd_id);

			if (fdinfo_lines[2].avail)
				printf(", eventfd-semaphore=%d", efd_semaphore);
		}

		printf("}>");
	}
}

int
main(void)
{
	static const char efd_counter_pfx[] = "eventfd-count:";
	static const char efd_id_pfx[] = "eventfd-id:";
	static const char efd_semaphore_pfx[] = "eventfd-semaphore:";

	const pid_t pid = getpid();
	const unsigned int u = 5;

	uint64_t efd_counter = u;
	int efd_id = -1;
	int efd_semaphore = 0;

	struct fdinfo fdinfo_lines[] = {
		{
			.search_pfx = efd_counter_pfx,
			.search_pfx_len = sizeof(efd_counter_pfx) - 1,
		},
		{
			.search_pfx = efd_id_pfx,
			.search_pfx_len = sizeof(efd_id_pfx) - 1,
			.data = (void *) &efd_id,
		},
		{
			.search_pfx = efd_semaphore_pfx,
			.search_pfx_len = sizeof(efd_semaphore_pfx) - 1,
		}
	};
	size_t fdinfo_array_size = ARRAY_SIZE(fdinfo_lines);

	int efd0 = k_eventfd(u, 0);
	if (efd0 < 0)
		perror_msg_and_skip("eventfd");

	procfs_check_avail_efd_data(pid, efd0, fdinfo_lines, fdinfo_array_size);

	printf("eventfd2(%u, 0) = %d", u, efd0);
	print_eventfd_details(fdinfo_lines, efd_counter, efd_semaphore);
	printf("\n");

	uint64_t read_efd_count;
	if (read(efd0, &read_efd_count, sizeof(read_efd_count))
	    != sizeof(read_efd_count))
		perror_msg_and_fail("read");
	efd_counter -= read_efd_count;

	printf("fchdir(%d", efd0);
	print_eventfd_details(fdinfo_lines, efd_counter, efd_semaphore);
	printf(") = %s\n", sprintrc(fchdir(efd0)));

	efd_counter = u;
	efd_id = -1;
	efd_semaphore = 1;
	fdinfo_lines[0].avail = false;
	fdinfo_lines[1].avail = false;
	fdinfo_lines[2].avail = false;

	int efd1 = k_eventfd(u, EFD_SEMAPHORE);
	if (efd1 < 0)
		perror_msg_and_skip("eventfd");

	procfs_check_avail_efd_data(pid, efd1, fdinfo_lines, fdinfo_array_size);

	printf("eventfd2(%u, EFD_SEMAPHORE) = %d", u, efd1);
	print_eventfd_details(fdinfo_lines, efd_counter, efd_semaphore);
	printf("\n");

	if (read(efd1, &read_efd_count, sizeof(read_efd_count))
	    != sizeof(read_efd_count))
		perror_msg_and_fail("read");
	efd_counter -= read_efd_count;

	printf("fchdir(%d", efd1);
	print_eventfd_details(fdinfo_lines, efd_counter, efd_semaphore);
	printf(") = %s\n", sprintrc(fchdir(efd1)));

	close(efd1);
	close(efd0);

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_SYS_EVENTFD_H")

#endif
