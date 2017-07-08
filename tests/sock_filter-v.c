/*
 * Check decoding of socket filters.
 *
 * Copyright (c) 2017 Dmitry V. Levin <ldv@altlinux.org>
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

#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/filter.h>

#define PRINT_STMT_SYM(pfx, code, k)	PRINT_STMT_SYM_(pfx, #code, #k)
#define PRINT_STMT_SYM_(pfx, code, k)	\
	printf("%sBPF_STMT(%s, %s)", pfx, code, k)
#define PRINT_STMT_VAL(pfx, code, k)	PRINT_STMT_VAL_(pfx, #code, k)
#define PRINT_STMT_VAL_(pfx, code, k)	\
	printf("%sBPF_STMT(%s, %#x)", pfx, code, k)

#define PRINT_JUMP(pfx, code, k, jt, jf)	PRINT_JUMP_(pfx, #code, k, jt, jf)
#define PRINT_JUMP_(pfx, code, k, jt, jf)	\
	printf("%sBPF_JUMP(%s, %#x, %#x, %#x)", pfx, code, k, jt, jf)

static const struct sock_filter bpf_filter[] = {
	BPF_STMT(BPF_LD|BPF_B|BPF_ABS, SKF_LL_OFF+4),
	BPF_STMT(BPF_LD|BPF_B|BPF_ABS, SKF_NET_OFF+8),
	BPF_STMT(BPF_LD|BPF_B|BPF_ABS, SKF_AD_OFF+SKF_AD_PROTOCOL),
	BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, IPPROTO_UDP, 0, 5),
	BPF_STMT(BPF_LD|BPF_W|BPF_LEN, 0),
	BPF_JUMP(BPF_JMP|BPF_K|BPF_JGE, 100, 0, 3),
	BPF_STMT(BPF_LD|BPF_B|BPF_ABS, 42),
	BPF_JUMP(BPF_JMP|BPF_K|BPF_JEQ, 'a', 0, 1),
	BPF_STMT(BPF_RET|BPF_K, -1U),
	BPF_STMT(BPF_RET|BPF_K, 0)
};

static void
print_filter(void)
{
	PRINT_STMT_SYM("[", BPF_LD|BPF_B|BPF_ABS, SKF_LL_OFF+4);
	PRINT_STMT_SYM(", ", BPF_LD|BPF_B|BPF_ABS, SKF_NET_OFF+8);
	PRINT_STMT_SYM(", ", BPF_LD|BPF_B|BPF_ABS, SKF_AD_OFF+SKF_AD_PROTOCOL);
	PRINT_JUMP(", ", BPF_JMP|BPF_K|BPF_JEQ, IPPROTO_UDP, 0, 5);
	PRINT_STMT_VAL(", ", BPF_LD|BPF_W|BPF_LEN, 0);
	PRINT_JUMP(", ", BPF_JMP|BPF_K|BPF_JGE, 100, 0, 3);
	PRINT_STMT_VAL(", ", BPF_LD|BPF_B|BPF_ABS, 42);
	PRINT_JUMP(", ", BPF_JMP|BPF_K|BPF_JEQ, 'a', 0, 1);
	PRINT_STMT_VAL(", ", BPF_RET|BPF_K, -1U);
	PRINT_STMT_VAL(", ", BPF_RET|BPF_K, 0);
	putchar(']');
}

static const char *errstr;

static int
get_filter(int fd, void *val, socklen_t *len)
{
	int rc = getsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, val, len);
	errstr = sprintrc(rc);
	return rc;
}

static int
set_filter(int fd, void *val, socklen_t len)
{
	int rc = setsockopt(fd, SOL_SOCKET, SO_ATTACH_FILTER, val, len);
	errstr = sprintrc(rc);
	return rc;
}

int
main(void)
{
	int rc;
	struct sock_filter *const filter =
		tail_memdup(bpf_filter, sizeof(bpf_filter));
	void *const efault = filter + ARRAY_SIZE(bpf_filter);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct sock_fprog, prog);
	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, len);

	prog->len = ARRAY_SIZE(bpf_filter);
	prog->filter = filter;

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0)
		perror_msg_and_skip("socket AF_INET SOCK_DGRAM");

	/* query sock_filter program length -> 0 */
	*len = BPF_MAXINSNS;
	rc = get_filter(fd, NULL, len);
	if (rc)
		perror_msg_and_skip("getsockopt SOL_SOCKET SO_ATTACH_FILTER");
	printf("getsockopt(%d, SOL_SOCKET, SO_ATTACH_FILTER"
	       ", NULL, [%u->0]) = 0\n", fd, BPF_MAXINSNS);

	/* getsockopt NULL optlen - EFAULT */
	rc = get_filter(fd, NULL, NULL);
	printf("getsockopt(%d, SOL_SOCKET, SO_ATTACH_FILTER, NULL, NULL)"
	       " = %s\n", fd, errstr);

	/* attach a filter */
	rc = set_filter(fd, prog, sizeof(*prog));
	if (rc)
		perror_msg_and_skip("setsockopt SOL_SOCKET SO_ATTACH_FILTER");
	printf("setsockopt(%d, SOL_SOCKET, SO_ATTACH_FILTER, {len=%u, filter=",
	       fd, prog->len);
	print_filter();
	printf("}, %u) = 0\n", (unsigned int) sizeof(*prog));

	/* setsockopt optlen is too small - EINVAL */
	rc = set_filter(fd, prog, sizeof(*prog) - 4);
	printf("setsockopt(%d, SOL_SOCKET, SO_ATTACH_FILTER, %p, %u) = %s\n",
	       fd, prog, (unsigned int) sizeof(*prog) - 4, errstr);

#ifdef SO_ATTACH_REUSEPORT_CBPF
	rc = setsockopt(fd, SOL_SOCKET, SO_ATTACH_REUSEPORT_CBPF,
			prog, sizeof(*prog));
	errstr = sprintrc(rc);
	printf("setsockopt(%d, SOL_SOCKET, SO_ATTACH_REUSEPORT_CBPF"
	       ", {len=%u, filter=", fd, prog->len);
	print_filter();
	printf("}, %u) = %s\n", (unsigned int) sizeof(*prog), errstr);
#endif

	/* query sock_filter program length -> ARRAY_SIZE(bpf_filter) */
	*len = 0;
	rc = get_filter(fd, efault, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_ATTACH_FILTER, %p"
	       ", [0->%u]) = %s\n",
	       fd, efault, (unsigned int) ARRAY_SIZE(bpf_filter), errstr);

	/* getsockopt optlen is too small - EINVAL */
	*len = ARRAY_SIZE(bpf_filter) - 1;
	rc = get_filter(fd, efault, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_ATTACH_FILTER, %p"
	       ", [%u]) = %s\n",
	       fd, efault, (unsigned int) ARRAY_SIZE(bpf_filter) - 1, errstr);

	/* getsockopt optval EFAULT */
	*len = ARRAY_SIZE(bpf_filter);
	rc = get_filter(fd, filter + 1, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_ATTACH_FILTER, %p"
	       ", [%u]) = %s\n", fd, filter + 1,
	       (unsigned int) ARRAY_SIZE(bpf_filter), errstr);

	/* getsockopt optlen is too large - truncated */
	*len = ARRAY_SIZE(bpf_filter) + 1;
	rc = get_filter(fd, filter, len);
	printf("getsockopt(%d, SOL_SOCKET, SO_ATTACH_FILTER, ", fd);
	print_filter();
	printf(", [%u->%d]) = %s\n",
	       (unsigned int) ARRAY_SIZE(bpf_filter) + 1, *len, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
