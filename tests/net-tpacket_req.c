/*
 * Copyright (c) 2018 The strace developers.
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
set_tpacket_req(int optname, void *optval, socklen_t len)
{
	long rc = setsockopt(-1, SOL_PACKET, optname, optval, len);
	errstr = sprintrc(rc);
	return rc;
}

static void
test_tpacket_req(const int optname, const char *const optname_str)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct tpacket_req, tpreq);
	socklen_t len;

	/* setsockopt with optname unknown */
	set_tpacket_req(-1, NULL, 0);
	printf("setsockopt(-1, SOL_PACKET, %#x /* PACKET_??? */, NULL, 0) = %s\n",
	       -1, errstr);

	/* classic setsockopt */
	len = sizeof(struct tpacket_req);
	set_tpacket_req(optname, tpreq, len);
	printf("setsockopt(-1, SOL_PACKET, %s, {tp_block_size=%u,"
	       " tp_block_nr=%u, tp_frame_size=%u, tp_frame_nr=%u}, %d) = %s\n",
	       optname_str, tpreq->tp_block_size, tpreq->tp_block_nr,
	       tpreq->tp_frame_size, tpreq->tp_frame_nr, len, errstr);

	/* setsockopt with optlen larger than usual */
	len = len + 1;
	set_tpacket_req(optname, tpreq, len);
	printf("setsockopt(-1, SOL_PACKET, %s, %p,"
	       " %d) = %s\n", optname_str, tpreq, len, errstr);
}

int
main(void)
{
	test_tpacket_req(ARG_STR(PACKET_RX_RING));
#ifdef PACKET_TX_RING
	test_tpacket_req(ARG_STR(PACKET_TX_RING));
#endif

	puts("+++ exited with 0 +++");
	return 0;
}
