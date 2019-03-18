/*
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <sys/socket.h>
#include <linux/if_packet.h>

static const char *errstr;

static long
packet_mreq_membership(int optname, void *optval, socklen_t len)
{
	long rc = setsockopt(-1, SOL_PACKET, optname, optval, len);
	errstr = sprintrc(rc);
	return rc;
}

static void
test_packet_mreq(const int optname, const char *const optname_str)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct packet_mreq, pmreq);
	socklen_t len = sizeof(struct packet_mreq);

	/* setsockopt with optname unknown */
	packet_mreq_membership(-1, NULL, 0);
#if XLAT_RAW
	printf("setsockopt(-1, %#x, %#x, NULL, 0) = %s\n",
	       SOL_PACKET, -1, errstr);
#elif XLAT_VERBOSE
	printf("setsockopt(-1, %#x /* SOL_PACKET */, %#x /* PACKET_??? */"
	       ", NULL, 0) = %s\n", SOL_PACKET, -1, errstr);
#else
	printf("setsockopt(-1, SOL_PACKET, %#x /* PACKET_??? */, NULL, 0) = %s\n",
	       -1, errstr);
#endif

	/* setsockopt with mr_type unknown */
	pmreq->mr_ifindex = 0;
	pmreq->mr_alen = ARRAY_SIZE(pmreq->mr_address);
	packet_mreq_membership(optname, pmreq, len);
#if XLAT_RAW
	printf("setsockopt(-1, %#x, %#x, {mr_ifindex=%d,"
	       " mr_type=%#x, mr_alen=%d, mr_address=",
	       SOL_PACKET, optname, pmreq->mr_ifindex,
	       pmreq->mr_type, pmreq->mr_alen);
	print_quoted_hex((const void *) pmreq->mr_address,
			 ARRAY_SIZE(pmreq->mr_address));
	printf("}, %d) = %s\n", len, errstr);
#elif XLAT_VERBOSE
	printf("setsockopt(-1, %#x /* SOL_PACKET */, %#x /* %s */"
	       ", {mr_ifindex=%d, mr_type=%#x /* PACKET_MR_??? */"
	       ", mr_alen=%d, mr_address=",
	       SOL_PACKET, optname, optname_str, pmreq->mr_ifindex,
	       pmreq->mr_type, pmreq->mr_alen);
	print_quoted_hex((const void *) pmreq->mr_address,
			 ARRAY_SIZE(pmreq->mr_address));
	printf(" /* ");
	for (unsigned int i = 0; i < ARRAY_SIZE(pmreq->mr_address); i++)
		printf("%s%02x", i > 0 ? ":" : "", pmreq->mr_address[i]);
	printf(" */}, %d) = %s\n", len, errstr);
#else
	printf("setsockopt(-1, SOL_PACKET, %s, {mr_ifindex=%d,"
	       " mr_type=%#x /* PACKET_MR_??? */, mr_alen=%d, mr_address=",
	       optname_str, pmreq->mr_ifindex, pmreq->mr_type, pmreq->mr_alen);
	for (unsigned int i = 0; i < ARRAY_SIZE(pmreq->mr_address); i++)
		printf("%s%02x", i > 0 ? ":" : "", pmreq->mr_address[i]);
	printf("}, %d) = %s\n", len, errstr);
#endif

	/* setsockopt with mr_type unknown and mr_alen > sizeof(mr_address) */
	pmreq->mr_alen = ARRAY_SIZE(pmreq->mr_address) + 1;
	packet_mreq_membership(optname, pmreq, len);
#if XLAT_RAW
	printf("setsockopt(-1, %#x, %#x, {mr_ifindex=%d,"
	       " mr_type=%#x, mr_alen=%d, mr_address=",
	       SOL_PACKET, optname, pmreq->mr_ifindex,
	       pmreq->mr_type, pmreq->mr_alen);
	print_quoted_hex((const void *) pmreq->mr_address,
			 ARRAY_SIZE(pmreq->mr_address));
	printf("}, %d) = %s\n", len, errstr);
#elif XLAT_VERBOSE
	printf("setsockopt(-1, %#x /* SOL_PACKET */, %#x /* %s */"
	       ", {mr_ifindex=%d, mr_type=%#x /* PACKET_MR_??? */"
	       ", mr_alen=%d, mr_address=",
	       SOL_PACKET, optname, optname_str, pmreq->mr_ifindex,
	       pmreq->mr_type, pmreq->mr_alen);
	print_quoted_hex((const void *) pmreq->mr_address,
			 ARRAY_SIZE(pmreq->mr_address));
	printf(" /* ");
	for (unsigned int i = 0; i < ARRAY_SIZE(pmreq->mr_address); i++)
		printf("%s%02x", i > 0 ? ":" : "", pmreq->mr_address[i]);
	printf(" */}, %d) = %s\n", len, errstr);
#else
	printf("setsockopt(-1, SOL_PACKET, %s, {mr_ifindex=%d,"
	       " mr_type=%#x /* PACKET_MR_??? */, mr_alen=%d, mr_address=",
	       optname_str, pmreq->mr_ifindex, pmreq->mr_type, pmreq->mr_alen);
	for (unsigned int i = 0; i < ARRAY_SIZE(pmreq->mr_address); i++)
		printf("%s%02x", i > 0 ? ":" : "", pmreq->mr_address[i]);
	printf("}, %d) = %s\n", len, errstr);
#endif

	/* setsockopt with mr_type unknown and mr_alen < sizeof(mr_address) */
	pmreq->mr_alen = ARRAY_SIZE(pmreq->mr_address) - 1;
	packet_mreq_membership(optname, pmreq, len);
#if XLAT_RAW
	printf("setsockopt(-1, %#x, %#x, {mr_ifindex=%d,"
	       " mr_type=%#x, mr_alen=%d, mr_address=",
	       SOL_PACKET, optname, pmreq->mr_ifindex,
	       pmreq->mr_type, pmreq->mr_alen);
	print_quoted_hex((const void *) pmreq->mr_address, pmreq->mr_alen);
	printf("}, %d) = %s\n", len, errstr);
#elif XLAT_VERBOSE
	printf("setsockopt(-1, %#x /* SOL_PACKET */, %#x /* %s */"
	       ", {mr_ifindex=%d, mr_type=%#x /* PACKET_MR_??? */"
	       ", mr_alen=%d, mr_address=",
	       SOL_PACKET, optname, optname_str, pmreq->mr_ifindex,
	       pmreq->mr_type, pmreq->mr_alen);
	print_quoted_hex((const void *) pmreq->mr_address, pmreq->mr_alen);
	printf(" /* ");
	for (unsigned int i = 0; i < pmreq->mr_alen; i++)
		printf("%s%02x", i > 0 ? ":" : "", pmreq->mr_address[i]);
	printf(" */}, %d) = %s\n", len, errstr);
#else
	printf("setsockopt(-1, SOL_PACKET, %s, {mr_ifindex=%d,"
	       " mr_type=%#x /* PACKET_MR_??? */, mr_alen=%d, mr_address=",
	       optname_str, pmreq->mr_ifindex, pmreq->mr_type, pmreq->mr_alen);
	for (unsigned int i = 0; i < pmreq->mr_alen; i++)
		printf("%s%02x", i > 0 ? ":" : "", pmreq->mr_address[i]);
	printf("}, %d) = %s\n", len, errstr);
#endif

	/* setsockopt with valid mr_type */
	pmreq->mr_alen = ARRAY_SIZE(pmreq->mr_address);
	static const struct {
		unsigned short type;
		const char *const type_str;
	} a[] = {
		{ ARG_STR(PACKET_MR_MULTICAST) },
		{ ARG_STR(PACKET_MR_PROMISC) },
		{ ARG_STR(PACKET_MR_ALLMULTI) },
#ifdef PACKET_MR_UNICAST
		{ ARG_STR(PACKET_MR_UNICAST) },
#endif
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(a); i++) {
		pmreq->mr_type = a[i].type;
		packet_mreq_membership(optname, pmreq, len);
#if XLAT_RAW
		printf("setsockopt(-1, %#x, %#x, {mr_ifindex=%d,"
		       " mr_type=%#x, mr_alen=%d, mr_address=",
		       SOL_PACKET, optname, pmreq->mr_ifindex,
		       pmreq->mr_type, pmreq->mr_alen);
		print_quoted_hex((const void *) pmreq->mr_address, pmreq->mr_alen);
		printf("}, %d) = %s\n", len, errstr);
#elif XLAT_VERBOSE
		printf("setsockopt(-1, %#x /* SOL_PACKET */, %#x /* %s */"
		       ", {mr_ifindex=%d, mr_type=%#x /* %s */"
		       ", mr_alen=%d, mr_address=",
		       SOL_PACKET, optname, optname_str, pmreq->mr_ifindex,
		       pmreq->mr_type, a[i].type_str, pmreq->mr_alen);
		print_quoted_hex((const void *) pmreq->mr_address, pmreq->mr_alen);
		printf(" /* ");
		for (unsigned int i = 0; i < pmreq->mr_alen; i++)
			printf("%s%02x", i > 0 ? ":" : "", pmreq->mr_address[i]);
		printf(" */}, %d) = %s\n", len, errstr);
#else
		printf("setsockopt(-1, SOL_PACKET, %s, {mr_ifindex=%d,"
		       " mr_type=%s, mr_alen=%d, mr_address=",
		       optname_str, pmreq->mr_ifindex, a[i].type_str, pmreq->mr_alen);
		for (unsigned int i = 0; i < pmreq->mr_alen; i++)
			printf("%s%02x", i > 0 ? ":" : "", pmreq->mr_address[i]);
		printf("}, %d) = %s\n", len, errstr);
#endif
	}

	/* setsockopt with optlen larger than usual */
	len = len + 1;
	packet_mreq_membership(optname, pmreq, len);
#if XLAT_RAW
	printf("setsockopt(-1, %#x, %#x, %p, %d) = %s\n",
	       SOL_PACKET, optname, pmreq, len, errstr);
#elif XLAT_VERBOSE
	printf("setsockopt(-1, %#x /* SOL_PACKET */, %#x /* %s */"
	       ", %p, %d) = %s\n", SOL_PACKET, optname, optname_str,
	       pmreq, len, errstr);
#else
	printf("setsockopt(-1, SOL_PACKET, %s, %p,"
	       " %d) = %s\n", optname_str, pmreq, len, errstr);
#endif
}

int
main(void)
{
	test_packet_mreq(ARG_STR(PACKET_ADD_MEMBERSHIP));
	test_packet_mreq(ARG_STR(PACKET_DROP_MEMBERSHIP));

	puts("+++ exited with 0 +++");
	return 0;
}
