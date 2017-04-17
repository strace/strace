/*
 * Check decoding of move_pages syscall.
 *
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include "tests.h"
#include <asm/unistd.h>

#ifdef __NR_move_pages

# include <errno.h>
# include <stdio.h>
# include <unistd.h>

# define MAX_STRLEN 3

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
	unsigned long i;
	for (i = 0; i < count; ++i) {
		if (i)
			printf(", ");
		if (i + offset < count) {
			if (i >= MAX_STRLEN) {
				printf("...");
				break;
			}
		} else {
			printf("%p", pages + i);
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
	unsigned long i;
	for (i = 0; i < count; ++i) {
		if (i)
			printf(", ");
		if (i + offset < count) {
			if (i >= MAX_STRLEN) {
				printf("...");
				break;
			}
		} else {
			printf("%p", nodes + i);
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
	unsigned long i;
	for (i = 0; i < count; ++i) {
		if (i)
			printf(", ");
		if (i >= MAX_STRLEN) {
			printf("...");
			break;
		}
		if (status[i] >= 0) {
			printf("%d", status[i]);
		} else {
			errno = -status[i];
			printf("-%s", errno2name());
		}
	}
	printf("]");
}

static void
print_stat_pages(const unsigned long pid, const unsigned long count,
		 const void **const pages, int *const status)
{
	const unsigned long flags = (unsigned long) 0xfacefeed00000002ULL;

	long rc = syscall(__NR_move_pages,
			  pid, count, pages, NULL, status, flags);
	const char *errstr = sprintrc(rc);
	printf("move_pages(%d, %lu, ", (int) pid, count);
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
	printf(", MPOL_MF_MOVE) = %s\n", errstr);
}

static void
print_move_pages(const unsigned long pid,
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
	printf("move_pages(%d, %lu, ", (int) pid, count);
	print_page_array(pages, count, offset);
	printf(", ");
	print_node_array(nodes, count, offset);
	printf(", ");
	if (count)
		printf("%p", status);
	else
		printf("[]");
	printf(", MPOL_MF_MOVE_ALL) = %s\n", errstr);
}

int
main(void)
{
	const unsigned long pid =
		(unsigned long) 0xfacefeed00000000ULL | getpid();
	unsigned long count = 1;
	const unsigned page_size = get_page_size();
	const void *const page = tail_alloc(page_size);
	const void *const efault = page + page_size;
	TAIL_ALLOC_OBJECT_VAR_PTR(const void *, pages);
	TAIL_ALLOC_OBJECT_VAR_PTR(int, nodes);
	TAIL_ALLOC_OBJECT_VAR_PTR(int, status);

	print_stat_pages(pid, 0, pages, status);
	print_move_pages(pid, 0, 0, pages, nodes, status);
	print_move_pages(pid, 0, 1, pages + 1, nodes + 1, status + 1);

	*pages = page;
	print_stat_pages(pid, count, pages, status);
	*nodes = 0xdeadbee1;
	print_move_pages(pid, count, 0, pages, nodes, status);
	print_move_pages(pid, count, 1, pages, nodes, status);

	++count;
	--status;
	*(--pages) = efault;
	print_stat_pages(pid, count, pages, status);
	*(--nodes) = 0xdeadbee2;
	print_move_pages(pid, count, 0, pages, nodes, status);
	print_move_pages(pid, count, 1, pages, nodes, status);

	++count;
	--status;
	*(--pages) = nodes;
	print_stat_pages(pid, count, pages, status);
	*(--nodes) = 0xdeadbee3;
	print_move_pages(pid, count, 0, pages, nodes, status);
	print_move_pages(pid, count, 1, pages, nodes, status);

	++count;
	--status;
	*(--pages) = status;
	print_stat_pages(pid, count, pages, status);
	*(--nodes) = 0xdeadbee4;
	print_move_pages(pid, count, 0, pages, nodes, status);
	print_move_pages(pid, count, 1, pages, nodes, status);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_move_pages")

#endif
