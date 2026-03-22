/*
 * Check decoding of UDMABUF_* ioctl commands.
 *
 * Copyright (c) 2026 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <linux/udmabuf.h>

static const struct {
	struct strval32 memfd;
	struct strval32 flags;
	struct strval64 offset;
	struct strval64 size;
} create_tests[] = {
	{
		{ ARG_STR(-1) },
		{ ARG_STR(0) },
		{ -2, "18446744073709551614" },
		{ -3, "18446744073709551613" },
	}, {
		{ 0, "0</dev/null>" },
		{ ARG_STR(UDMABUF_FLAGS_CLOEXEC) },
		{ -2U, "4294967294" },
		{ -3U, "4294967293" },
	}, {
		{ ARG_STR(-1) },
		{ -1, "UDMABUF_FLAGS_CLOEXEC|0xfffffffe" },
		{ ARG_STR(123456789) },
		{ ARG_STR(234567890) },
	}, {
		{ 0, "0</dev/null>" },
		{ ARG_STR(0xfffffffe) " /* UDMABUF_FLAGS_??? */" },
		{ ARG_STR(345678901) },
		{ ARG_STR(456789012) },
	},
};

static void
test_create(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct udmabuf_create, p_udmabuf_create);

	ioctl(-1, UDMABUF_CREATE, NULL);
	printf("ioctl(-1, UDMABUF_CREATE, NULL)" RVAL_EBADF);

	for (size_t i = 0; i < ARRAY_SIZE(create_tests); i++) {
		p_udmabuf_create->memfd = create_tests[i].memfd.val;
		p_udmabuf_create->flags = create_tests[i].flags.val;
		p_udmabuf_create->offset = create_tests[i].offset.val;
		p_udmabuf_create->size = create_tests[i].size.val;

		ioctl(-1, UDMABUF_CREATE, p_udmabuf_create);
		printf("ioctl(-1, UDMABUF_CREATE"
		       ", {memfd=%s, flags=%s, offset=%s, size=%s})" RVAL_EBADF,
		       create_tests[i].memfd.str,
		       create_tests[i].flags.str,
		       create_tests[i].offset.str,
		       create_tests[i].size.str);
	}
}

static void
test_create_list(void)
{
	ioctl(-1, UDMABUF_CREATE_LIST, NULL);
	printf("ioctl(-1, UDMABUF_CREATE_LIST, NULL)" RVAL_EBADF);

	struct udmabuf_create_list *const p_udmabuf_create_list = tail_alloc(
		offsetof(struct udmabuf_create_list, list)
		+ sizeof(struct udmabuf_create_item) * ARRAY_SIZE(create_tests)
	);

	p_udmabuf_create_list->count = ARRAY_SIZE(create_tests);

	for (size_t i = 0; i < ARRAY_SIZE(create_tests); i++) {
		p_udmabuf_create_list->list[i].memfd = create_tests[i].memfd.val;
		p_udmabuf_create_list->list[i].offset = create_tests[i].offset.val;
		p_udmabuf_create_list->list[i].size = create_tests[i].size.val;
	}

	for (size_t i = 0; i < ARRAY_SIZE(create_tests); i++) {
		p_udmabuf_create_list->flags = create_tests[i].flags.val;

		ioctl(-1, UDMABUF_CREATE_LIST, p_udmabuf_create_list);
		printf("ioctl(-1, UDMABUF_CREATE_LIST"
		       ", {flags=%s, count=%u, list=[",
		       create_tests[i].flags.str,
		       p_udmabuf_create_list->count);

		for (size_t j = 0; j < ARRAY_SIZE(create_tests); j++) {
			if (j > 0)
				printf(", ");

			printf("{memfd=%s, offset=%s, size=%s}",
			       create_tests[j].memfd.str,
			       create_tests[j].offset.str,
			       create_tests[j].size.str);
		}

		printf("]})" RVAL_EBADF);
	}
}

int
main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	test_create();
	test_create_list();

	puts("+++ exited with 0 +++");
	return 0;
}
