/*
 * Check decoding of TUN/TAP ioctl commands.
 *
 * Copyright (c) 2026 Dmitry V. Levin <ldv@strace.io>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "xmalloc.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/filter.h>
#include <linux/if_arp.h>
#include <linux/if_tun.h>

#ifndef PRINT_PATHS
# define PRINT_PATHS 0
#endif
#ifndef VERBOSE
# define VERBOSE 0
#endif
#ifndef SKIP_IF_PROC_IS_UNAVAILABLE
# define SKIP_IF_PROC_IS_UNAVAILABLE
#endif

#if PRINT_PATHS
# define FD_PATH_FMT "<%s>"
#else
# define FD_PATH_FMT "%s"
#endif

static const char *errstr;
static unsigned int lo_ifindex;
static int null_fd;
static const char null_path[] = "/dev/null";

static const unsigned int all_iff =
	IFF_TUN
	| IFF_TAP
	| IFF_NAPI
	| IFF_NAPI_FRAGS
	| IFF_NO_CARRIER
	| IFF_BACKPRESSURE
	| IFF_MULTI_QUEUE
	| IFF_ATTACH_QUEUE
	| IFF_DETACH_QUEUE
	| IFF_PERSIST
	| IFF_NO_PI
	| IFF_ONE_QUEUE
	| IFF_VNET_HDR
	| IFF_TUN_EXCL;
static const char all_iff_str[] =
	"IFF_TUN"
	"|IFF_TAP"
	"|IFF_NAPI"
	"|IFF_NAPI_FRAGS"
	"|IFF_NO_CARRIER"
	"|IFF_BACKPRESSURE"
	"|IFF_MULTI_QUEUE"
	"|IFF_ATTACH_QUEUE"
	"|IFF_DETACH_QUEUE"
	"|IFF_PERSIST"
	"|IFF_NO_PI"
	"|IFF_ONE_QUEUE"
	"|IFF_VNET_HDR"
	"|IFF_TUN_EXCL";

static long
do_ioctl(kernel_ulong_t cmd, kernel_ulong_t arg)
{
	long rc = ioctl(-1, cmd, arg);

	errstr = sprintrc(rc);

#ifdef INJECT_RETVAL
	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Got a return value of %ld != %ld",
				   rc, (long) INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
#endif

	return rc;
}

static long
do_ioctl_ptr(kernel_ulong_t cmd, const void *arg)
{
	return do_ioctl(cmd, (uintptr_t) arg);
}

static void
test_no_arg(void)
{
	do_ioctl(TUNGETDEVNETNS, 0);
#if PRINT_PATHS && defined(INJECT_RETVAL)
	printf("ioctl(-1, TUNGETDEVNETNS) = %d<%s> (INJECTED)\n",
	       INJECT_RETVAL, null_path);
#else
	printf("ioctl(-1, TUNGETDEVNETNS) = %s\n", errstr);
#endif
}

static void
test_by_value(void)
{
	static const unsigned long ul_val = (unsigned long) 0xdeadbeefbadc0dedULL;

	static const struct strval32 ul_cmds[] = {
		{ ARG_STR(TUNSETNOCSUM) },
		{ ARG_STR(TUNSETPERSIST) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(ul_cmds); i++) {
		do_ioctl(ul_cmds[i].val, 0);
		printf("ioctl(-1, %s, 0) = %s\n",
		       ul_cmds[i].str, errstr);

		do_ioctl(ul_cmds[i].val, ul_val);
		printf("ioctl(-1, %s, %lu) = %s\n",
		       ul_cmds[i].str, ul_val, errstr);
	}

	/* TUNSETDEBUG */
	do_ioctl(TUNSETDEBUG, 0);
	printf("ioctl(-1, TUNSETDEBUG, 0) = %s\n", errstr);

	do_ioctl(TUNSETDEBUG, 0xdeadbeef);
	printf("ioctl(-1, TUNSETDEBUG, 0xdeadbeef) = %s\n", errstr);

	static const struct strval32 uid_cmds[] = {
		{ ARG_STR(TUNSETOWNER) },
		{ ARG_STR(TUNSETGROUP) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(uid_cmds); i++) {
		do_ioctl(uid_cmds[i].val, 0);
		printf("ioctl(-1, %s, 0) = %s\n",
		       uid_cmds[i].str, errstr);

		do_ioctl(uid_cmds[i].val, -2U);
		printf("ioctl(-1, %s, %u) = %s\n",
		       uid_cmds[i].str, -2U, errstr);

		do_ioctl(uid_cmds[i].val, -1);
		printf("ioctl(-1, %s, -1) = %s\n",
		       uid_cmds[i].str, errstr);
	}

	/* TUNSETLINK */
	do_ioctl(TUNSETLINK, ARPHRD_ETHER);
	printf("ioctl(-1, TUNSETLINK, ARPHRD_ETHER) = %s\n", errstr);

	do_ioctl(TUNSETLINK, ARPHRD_NONE);
	printf("ioctl(-1, TUNSETLINK, ARPHRD_NONE) = %s\n", errstr);

	do_ioctl(TUNSETLINK, 0xfeed);
	printf("ioctl(-1, TUNSETLINK, 0xfeed /* ARPHRD_??? */) = %s\n", errstr);

	/* TUNSETOFFLOAD */
	do_ioctl(TUNSETOFFLOAD, TUN_F_CSUM | TUN_F_TSO4);
	printf("ioctl(-1, TUNSETOFFLOAD, TUN_F_CSUM|TUN_F_TSO4) = %s\n",
	       errstr);

	do_ioctl(TUNSETOFFLOAD, 0);
	printf("ioctl(-1, TUNSETOFFLOAD, 0) = %s\n", errstr);

	static const unsigned int all_tun_f =
		TUN_F_CSUM
		| TUN_F_TSO4
		| TUN_F_TSO6
		| TUN_F_TSO_ECN
		| TUN_F_UFO
		| TUN_F_USO4
		| TUN_F_USO6
		| TUN_F_UDP_TUNNEL_GSO
		| TUN_F_UDP_TUNNEL_GSO_CSUM;
	static const char all_tun_f_str[] =
		"TUN_F_CSUM"
		"|TUN_F_TSO4"
		"|TUN_F_TSO6"
		"|TUN_F_TSO_ECN"
		"|TUN_F_UFO"
		"|TUN_F_USO4"
		"|TUN_F_USO6"
		"|TUN_F_UDP_TUNNEL_GSO"
		"|TUN_F_UDP_TUNNEL_GSO_CSUM";

	do_ioctl(TUNSETOFFLOAD, all_tun_f);
	printf("ioctl(-1, TUNSETOFFLOAD, %s) = %s\n",
	       all_tun_f_str, errstr);

	do_ioctl(TUNSETOFFLOAD, ~all_tun_f);
	printf("ioctl(-1, TUNSETOFFLOAD, %#x /* TUN_F_??? */) = %s\n",
	       ~all_tun_f, errstr);

	do_ioctl(TUNSETOFFLOAD, (unsigned long) -1);
	printf("ioctl(-1, TUNSETOFFLOAD, %s|%#x) = %s\n",
	       all_tun_f_str, ~all_tun_f, errstr);
}

static void
test_ptr_int_write(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(int, pint);

	static const struct strval32 int_cmds[] = {
		{ ARG_STR(TUNSETSNDBUF) },
		{ ARG_STR(TUNSETVNETHDRSZ) },
		{ ARG_STR(TUNSETVNETLE) },
		{ ARG_STR(TUNSETVNETBE) },
		{ ARG_STR(TUNSETCARRIER) },
	};
	static const int int_val = 0xdeadbeef;

	for (size_t i = 0; i < ARRAY_SIZE(int_cmds); i++) {
		*pint = int_val;
		do_ioctl_ptr(int_cmds[i].val, pint);
		printf("ioctl(-1, %s, [%d]) = %s\n",
		       int_cmds[i].str, int_val, errstr);

		do_ioctl_ptr(int_cmds[i].val, NULL);
		printf("ioctl(-1, %s, NULL) = %s\n",
		       int_cmds[i].str, errstr);

		do_ioctl_ptr(int_cmds[i].val, (char *) pint + 1);
		printf("ioctl(-1, %s, %p) = %s\n",
		       int_cmds[i].str, (char *) pint + 1, errstr);
	}

	static const struct strval32 fd_cmds[] = {
		{ ARG_STR(TUNSETSTEERINGEBPF) },
		{ ARG_STR(TUNSETFILTEREBPF) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(fd_cmds); i++) {
		*pint = null_fd;
		do_ioctl_ptr(fd_cmds[i].val, pint);
		printf("ioctl(-1, %s, [%d" FD_PATH_FMT "]) = %s\n",
		       fd_cmds[i].str,
		       null_fd, PRINT_PATHS ? null_path : "", errstr);

		do_ioctl_ptr(fd_cmds[i].val, NULL);
		printf("ioctl(-1, %s, NULL) = %s\n",
		       fd_cmds[i].str, errstr);

		do_ioctl_ptr(fd_cmds[i].val, (char *) pint + 1);
		printf("ioctl(-1, %s, %p) = %s\n",
		       fd_cmds[i].str, (char *) pint + 1, errstr);
	}

	/* TUNSETIFINDEX uses print_ifindex */
	*pint = lo_ifindex;
	do_ioctl_ptr(TUNSETIFINDEX, pint);
	printf("ioctl(-1, TUNSETIFINDEX, [" IFINDEX_LO_STR "]) = %s\n", errstr);

	*pint = 0;
	do_ioctl_ptr(TUNSETIFINDEX, pint);
	printf("ioctl(-1, TUNSETIFINDEX, [0]) = %s\n", errstr);

	do_ioctl_ptr(TUNSETIFINDEX, NULL);
	printf("ioctl(-1, TUNSETIFINDEX, NULL) = %s\n", errstr);

	do_ioctl_ptr(TUNSETIFINDEX, (char *) pint + 1);
	printf("ioctl(-1, TUNSETIFINDEX, %p) = %s\n",
	       (char *) pint + 1, errstr);
}

static void
test_ptr_int_read(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(int, pint);

	static const struct strval32 int_rd_cmds[] = {
		{ ARG_STR(TUNGETSNDBUF) },
		{ ARG_STR(TUNGETVNETHDRSZ) },
		{ ARG_STR(TUNGETVNETLE) },
		{ ARG_STR(TUNGETVNETBE) },
	};
	static const int int_val = 0xdeadbeef;

	for (size_t i = 0; i < ARRAY_SIZE(int_rd_cmds); i++) {
		long rc;

		*pint = int_val;
		rc = do_ioctl_ptr(int_rd_cmds[i].val, pint);
		printf("ioctl(-1, %s, ", int_rd_cmds[i].str);
		if (rc >= 0)
			printf("[%d]", int_val);
		else
			printf("%p", pint);
		printf(") = %s\n", errstr);

		do_ioctl_ptr(int_rd_cmds[i].val, NULL);
		printf("ioctl(-1, %s, NULL) = %s\n",
		       int_rd_cmds[i].str, errstr);

		do_ioctl_ptr(int_rd_cmds[i].val, (char *) pint + 1);
		printf("ioctl(-1, %s, %p) = %s\n",
		       int_rd_cmds[i].str, (char *) pint + 1, errstr);
	}
}

static void
test_ptr_uint_read(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(unsigned int, puint);

	char *unknown_iff_str = xasprintf("%#x /* IFF_??? */", ~all_iff);
	char *all_bits_iff_str = xasprintf("%s|%#x", all_iff_str, ~all_iff);

	const struct strval32 feat_vals[] = {
		{ 0, "0" },
		{ all_iff, all_iff_str },
		{ ~all_iff, unknown_iff_str },
		{ -1U, all_bits_iff_str },
	};

	for (size_t i = 0; i < ARRAY_SIZE(feat_vals); i++) {
		long rc;

		*puint = feat_vals[i].val;
		rc = do_ioctl_ptr(TUNGETFEATURES, puint);
		printf("ioctl(-1, TUNGETFEATURES, ");
		if (rc >= 0)
			printf("[%s]", feat_vals[i].str);
		else
			printf("%p", puint);
		printf(") = %s\n", errstr);
	}

	do_ioctl_ptr(TUNGETFEATURES, NULL);
	printf("ioctl(-1, TUNGETFEATURES, NULL) = %s\n", errstr);

	do_ioctl_ptr(TUNGETFEATURES, (char *) puint + 1);
	printf("ioctl(-1, TUNGETFEATURES, %p) = %s\n",
	       (char *) puint + 1, errstr);

	free(unknown_iff_str);
	free(all_bits_iff_str);
}

static void
test_tun_filter(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct tun_filter, tf);
	memset(tf, 0, sizeof(*tf));

	static const uint16_t all_tun_flt = TUN_FLT_ALLMULTI;
	static const char all_tun_flt_str[] = "TUN_FLT_ALLMULTI";

	char *unknown_flt_str =
		xasprintf("%#x /* TUN_FLT_??? */", (uint16_t) ~all_tun_flt);
	char *all_bits_flt_str =
		xasprintf("%s|%#x", all_tun_flt_str, (uint16_t) ~all_tun_flt);

	const struct strval32 flt_vals[] = {
		{ 0, "0" },
		{ all_tun_flt, all_tun_flt_str },
		{ (uint16_t) ~all_tun_flt, unknown_flt_str },
		{ (uint16_t) -1, all_bits_flt_str },
	};

	for (size_t i = 0; i < ARRAY_SIZE(flt_vals); i++) {
		tf->flags = flt_vals[i].val;
		do_ioctl_ptr(TUNSETTXFILTER, tf);
		printf("ioctl(-1, TUNSETTXFILTER"
		       ", {flags=%s, count=0}) = %s\n",
		       flt_vals[i].str, errstr);
	}

	struct tun_filter *tf2 =
		tail_alloc(sizeof(*tf2) + 2 * ETH_ALEN);
	memset(tf2, 0, sizeof(*tf2) + 2 * ETH_ALEN);
	tf2->count = 2;
	static const uint8_t mac1[ETH_ALEN] =
		{0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff};
	static const uint8_t mac2[ETH_ALEN] =
		{0x11, 0x22, 0x33, 0x44, 0x55, 0x66};
	memcpy(tf2->addr[0], mac1, ETH_ALEN);
	memcpy(tf2->addr[1], mac2, ETH_ALEN);

	do_ioctl_ptr(TUNSETTXFILTER, tf2);
	printf("ioctl(-1, TUNSETTXFILTER, {flags=0, count=2"
	       ", addr=[aa:bb:cc:dd:ee:ff, 11:22:33:44:55:66]}) = %s\n",
	       errstr);

	do_ioctl_ptr(TUNSETTXFILTER, NULL);
	printf("ioctl(-1, TUNSETTXFILTER, NULL) = %s\n", errstr);

	do_ioctl_ptr(TUNSETTXFILTER, (char *) tf + 1);
	printf("ioctl(-1, TUNSETTXFILTER, %p) = %s\n",
	       (char *) tf + 1, errstr);

	free(unknown_flt_str);
	free(all_bits_flt_str);
}

static void
test_sock_fprog(void)
{
	static const struct sock_filter bpf_filter[] = {
		BPF_STMT(BPF_RET | BPF_K, 0)
	};
	struct sock_filter *filter =
		tail_memdup(bpf_filter, sizeof(bpf_filter));
	TAIL_ALLOC_OBJECT_CONST_PTR(struct sock_fprog, prog);

	prog->len = ARRAY_SIZE(bpf_filter);
	prog->filter = filter;

	static const struct strval32 fprog_cmds[] = {
		{ ARG_STR(TUNGETFILTER) },
		{ ARG_STR(TUNATTACHFILTER) },
		{ ARG_STR(TUNDETACHFILTER) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(fprog_cmds); i++) {
		long rc;

		rc = do_ioctl_ptr(fprog_cmds[i].val, prog);
		printf("ioctl(-1, %s, ", fprog_cmds[i].str);
		if (rc >= 0 || fprog_cmds[i].val != TUNGETFILTER)
#if VERBOSE
			printf("{len=1, filter=[BPF_STMT(BPF_RET|BPF_K, 0)]}");
#else
			printf("{len=1, filter=%p}", filter);
#endif
		else
			printf("%p", prog);
		printf(") = %s\n", errstr);

		do_ioctl_ptr(fprog_cmds[i].val, NULL);
		printf("ioctl(-1, %s, NULL) = %s\n",
		       fprog_cmds[i].str, errstr);

		do_ioctl_ptr(fprog_cmds[i].val, (char *) prog + 1);
		printf("ioctl(-1, %s, %p) = %s\n",
		       fprog_cmds[i].str, (char *) prog + 1, errstr);
	}
}

static void
test_ifreq(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ifreq, ifr);

	static const struct strval32 ifreq_cmds[] = {
		{ ARG_STR(TUNGETIFF) },
		{ ARG_STR(TUNSETIFF) },
		{ ARG_STR(TUNSETQUEUE) },
	};

	char *unknown_iff_short_str =
		xasprintf("%#hx /* IFF_??? */", (unsigned short) ~all_iff);
	char *all_bits_iff_short_str =
		xasprintf("%s|%#hx", all_iff_str, (unsigned short) ~all_iff);

	const struct strval32 flag_vals[] = {
		{ 0, "0" },
		{ (unsigned short) all_iff, all_iff_str },
		{ (unsigned short) ~all_iff, unknown_iff_short_str },
		{ (unsigned short) -1, all_bits_iff_short_str },
	};

	for (size_t i = 0; i < ARRAY_SIZE(ifreq_cmds); i++) {
		for (size_t j = 0; j < ARRAY_SIZE(flag_vals); j++) {
			long rc;

			memset(ifr, 0, sizeof(*ifr));
			strcpy(ifr->ifr_name, "tun0");
			ifr->ifr_flags = flag_vals[j].val;

			rc = do_ioctl_ptr(ifreq_cmds[i].val, ifr);
			printf("ioctl(-1, %s, ", ifreq_cmds[i].str);
			if (rc >= 0 || ifreq_cmds[i].val != TUNGETIFF)
				printf("{ifr_name=\"%s\", ifr_flags=%s}",
				       ifr->ifr_name, flag_vals[j].str);
			else
				printf("%p", ifr);
			printf(") = %s\n", errstr);
		}

		do_ioctl_ptr(ifreq_cmds[i].val, NULL);
		printf("ioctl(-1, %s, NULL) = %s\n",
		       ifreq_cmds[i].str, errstr);

		do_ioctl_ptr(ifreq_cmds[i].val, ifr + 1);
		printf("ioctl(-1, %s, %p) = %s\n",
		       ifreq_cmds[i].str, ifr + 1, errstr);
	}

	free(unknown_iff_short_str);
	free(all_bits_iff_short_str);
}

int
main(int argc, char *argv[])
{
	SKIP_IF_PROC_IS_UNAVAILABLE;

	close(0);
	lo_ifindex = ifindex_lo();

	null_fd = open(null_path, O_RDONLY);
	if (null_fd < 0)
		perror_msg_and_fail("open: %s", null_path);

#ifdef INJECT_RETVAL
	unsigned long num_skip;
	bool locked = false;

	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; i++) {
		long ret = ioctl(-1, TUNSETNOCSUM, 0);

		printf("ioctl(-1, TUNSETNOCSUM, 0) = %s%s\n",
		       sprintrc(ret),
		       ret == INJECT_RETVAL ? " (INJECTED)" : "");

		if (ret != INJECT_RETVAL)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Hasn't locked on ioctl(-1, TUNSETNOCSUM, 0)"
				   " returning %d", INJECT_RETVAL);

	if (dup2(null_fd, INJECT_RETVAL) < 0)
		perror_msg_and_fail("dup2(%d, %d)", null_fd, INJECT_RETVAL);
#endif

	test_no_arg();
	test_by_value();
	test_ptr_int_write();
	test_ptr_int_read();
	test_ptr_uint_read();
	test_tun_filter();
	test_sock_fprog();
	test_ifreq();

	puts("+++ exited with 0 +++");
	return 0;
}
