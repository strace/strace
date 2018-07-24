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
	printf("setsockopt(-1, SOL_PACKET, %#x /* PACKET_??? */, NULL, 0) = %s\n",
	       -1, errstr);

	/* setsockopt with mr_type unknown */
	pmreq->mr_ifindex = 0;
	pmreq->mr_alen = ARRAY_SIZE(pmreq->mr_address);
	packet_mreq_membership(optname, pmreq, len);
	printf("setsockopt(-1, SOL_PACKET, %s, {mr_ifindex=%d,"
	       " mr_type=%#x /* PACKET_MR_??? */, mr_alen=%d, mr_address=",
	       optname_str, pmreq->mr_ifindex, pmreq->mr_type, pmreq->mr_alen);
	for (unsigned int i = 0; i < ARRAY_SIZE(pmreq->mr_address); i++) {
		printf("%02x", pmreq->mr_address[i]);
	}
	printf("}, %d) = %s\n", len, errstr);

	/* setsockopt with mr_type unknown and mr_alen > sizeof(mr_address) */
	pmreq->mr_alen = ARRAY_SIZE(pmreq->mr_address) + 1;
	packet_mreq_membership(optname, pmreq, len);
	printf("setsockopt(-1, SOL_PACKET, %s, {mr_ifindex=%d,"
	       " mr_type=%#x /* PACKET_MR_??? */, mr_alen=%d, mr_address=",
	       optname_str, pmreq->mr_ifindex, pmreq->mr_type, pmreq->mr_alen);
	for (unsigned int i = 0; i < ARRAY_SIZE(pmreq->mr_address); i++) {
		printf("%02x", pmreq->mr_address[i]);
	}
	printf("}, %d) = %s\n", len, errstr);

	/* setsockopt with mr_type unknown and mr_alen < sizeof(mr_address) */
	pmreq->mr_alen = ARRAY_SIZE(pmreq->mr_address) - 1;
	packet_mreq_membership(optname, pmreq, len);
	printf("setsockopt(-1, SOL_PACKET, %s, {mr_ifindex=%d,"
	       " mr_type=%#x /* PACKET_MR_??? */, mr_alen=%d, mr_address=",
	       optname_str, pmreq->mr_ifindex, pmreq->mr_type, pmreq->mr_alen);
	for (unsigned int i = 0; i < pmreq->mr_alen; i++) {
		printf("%02x", pmreq->mr_address[i]);
	}
	printf("}, %d) = %s\n", len, errstr);

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
		printf("setsockopt(-1, SOL_PACKET, %s, {mr_ifindex=%d,"
		       " mr_type=%s, mr_alen=%d, mr_address=",
		       optname_str, pmreq->mr_ifindex, a[i].type_str, pmreq->mr_alen);
		for (unsigned int i = 0; i < pmreq->mr_alen; i++) {
			printf("%02x", pmreq->mr_address[i]);
		}
		printf("}, %d) = %s\n", len, errstr);
	}

	/* setsockopt with optlen larger than usual */
	len = len + 1;
	packet_mreq_membership(optname, pmreq, len);
	printf("setsockopt(-1, SOL_PACKET, %s, %p,"
	       " %d) = %s\n", optname_str, pmreq, len, errstr);
}

int
main(void)
{
	test_packet_mreq(ARG_STR(PACKET_ADD_MEMBERSHIP));
	test_packet_mreq(ARG_STR(PACKET_DROP_MEMBERSHIP));

	puts("+++ exited with 0 +++");
	return 0;
}
