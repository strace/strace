/*
 * Check decoding of move_pages syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@strace.io>
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"
#include "pidns.h"

#include <errno.h>
#include <stdio.h>
#include <unistd.h>

#define MAX_STRLEN 3

static void
print_page_array(const void **const pages,
		 const unsigned long count,
		 const unsigned int offset)
{
	if (!count) {
		printf("%s", pages ? "[]" : "NULL");
		return;
	}
	if (count <= offset) {
		printf("%p", pages);
		return;
	}
	printf("[");
	for (unsigned long i = 0; i < count; ++i) {
		if (i)
			printf(", ");
		if (i + offset < count) {
			if (i >= MAX_STRLEN) {
				printf("...");
				break;
			}
		} else {
			printf("... /* %p */", pages + i);
			break;
		}
		const void *const addr = pages[i];
		if (addr)
			printf("%p", addr);
		else
			printf("NULL");
	}
	printf("]");
}

static void
print_node_array(const int *const nodes,
		 const unsigned long count,
		 const unsigned int offset)
{
	if (!count) {
		printf("%s", nodes ? "[]" : "NULL");
		return;
	}
	if (count <= offset) {
		printf("%p", nodes);
		return;
	}
	printf("[");
	for (unsigned long i = 0; i < count; ++i) {
		if (i)
			printf(", ");
		if (i + offset < count) {
			if (i >= MAX_STRLEN) {
				printf("...");
				break;
			}
		} else {
			printf("... /* %p */", nodes + i);
			break;
		}
		printf("%d", nodes[i]);
	}
	printf("]");
}

static void
print_status_array(const int *const status, const unsigned long count)
{
	if (!count) {
		printf("%s", status ? "[]" : "NULL");
		return;
	}
	printf("[");
	for (unsigned long i = 0; i < count; ++i) {
		if (i)
			printf(", ");
		if (i >= MAX_STRLEN) {
			printf("...");
			break;
		}
		if (status[i] >= 0) {
			printf("%d", status[i]);
		} else {
#if !XLAT_RAW
			errno = -status[i];
#endif
#if XLAT_RAW
			printf("%d", status[i]);
#elif XLAT_VERBOSE
			printf("%d /* -%s */", status[i], errno2name());
#else
			printf("-%s", errno2name());
#endif
		}
	}
	printf("]");
}

static void
print_stat_pages(const unsigned long pid,
		 const char *pid_str,
		 const unsigned long count,
		 const void **const pages,
		 int *const status)
{
	const unsigned long flags = (unsigned long) 0xfacefeed00000002ULL;

	long rc = syscall(__NR_move_pages,
			  pid, count, pages, NULL, status, flags);
	const char *errstr = sprintrc(rc);
	pidns_print_leader();
	printf("move_pages(%d%s, %lu, ", (int) pid, pid_str,
		count);
	print_page_array(pages, count, 0);
	printf(", NULL, ");
	if (rc) {
		if (count)
			printf("%p", status);
		else
			printf("[]");
	} else {
		print_status_array(status, count);
	}
#if XLAT_RAW
	printf(", 0x2) = %s\n", errstr);
#elif XLAT_VERBOSE
	printf(", 0x2 /* MPOL_MF_MOVE */) = %s\n", errstr);
#else /* XLAT_ABBREV */
	printf(", MPOL_MF_MOVE) = %s\n", errstr);
#endif
}

static void
print_move_pages(const unsigned long pid,
		 const char *pid_str,
		 unsigned long count,
		 const unsigned int offset,
		 const void **const pages,
		 int *const nodes,
		 int *const status)
{
	const unsigned long flags = (unsigned long) 0xfacefeed00000004ULL;
	count += offset;

	long rc = syscall(__NR_move_pages,
			  pid, count, pages, nodes, status, flags);
	const char *errstr = sprintrc(rc);
	pidns_print_leader();
	printf("move_pages(%d%s, %lu, ", (int) pid, pid_str,
		count);
	print_page_array(pages, count, offset);
	printf(", ");
	print_node_array(nodes, count, offset);
	printf(", ");
	if (count)
		printf("%p", status);
	else
		printf("[]");
#if XLAT_RAW
	printf(", 0x4) = %s\n", errstr);
#elif XLAT_VERBOSE
	printf(", 0x4 /* MPOL_MF_MOVE_ALL */) = %s\n", errstr);
#else /* XLAT_ABBREV */
	printf(", MPOL_MF_MOVE_ALL) = %s\n", errstr);
#endif
}

int
main(void)
{
	PIDNS_TEST_INIT;

	const unsigned long pid =
		(unsigned long) 0xfacefeed00000000ULL | getpid();
	const char *pid_str = pidns_pid2str(PT_TGID);
	unsigned long count = 1;
	const unsigned page_size = get_page_size();
	const void *const page = tail_alloc(page_size);
	const void *const efault = page + page_size;
	TAIL_ALLOC_OBJECT_VAR_PTR(const void *, pages);
	TAIL_ALLOC_OBJECT_VAR_PTR(int, nodes);
	TAIL_ALLOC_OBJECT_VAR_PTR(int, status);

	print_stat_pages(pid, pid_str, 0, pages, status);
	print_move_pages(pid, pid_str, 0, 0, pages, nodes, status);
	print_move_pages(pid, pid_str, 0, 1, pages + 1, nodes + 1, status + 1);

	*pages = page;
	print_stat_pages(pid, pid_str, count, pages, status);
	*nodes = 0xdeadbee1;
	print_move_pages(pid, pid_str, count, 0, pages, nodes, status);
	print_move_pages(pid, pid_str, count, 1, pages, nodes, status);

	++count;
	--status;
	*(--pages) = efault;
	print_stat_pages(pid, pid_str, count, pages, status);
	*(--nodes) = 0xdeadbee2;
	print_move_pages(pid, pid_str, count, 0, pages, nodes, status);
	print_move_pages(pid, pid_str, count, 1, pages, nodes, status);

	++count;
	--status;
	*(--pages) = nodes;
	print_stat_pages(pid, pid_str, count, pages, status);
	*(--nodes) = 0xdeadbee3;
	print_move_pages(pid, pid_str, count, 0, pages, nodes, status);
	print_move_pages(pid, pid_str, count, 1, pages, nodes, status);

	++count;
	--status;
	*(--pages) = status;
	print_stat_pages(pid, pid_str, count, pages, status);
	*(--nodes) = 0xdeadbee4;
	print_move_pages(pid, pid_str, count, 0, pages, nodes, status);
	print_move_pages(pid, pid_str, count, 1, pages, nodes, status);

	pidns_print_leader();
	puts("+++ exited with 0 +++");
	return 0;
}
