/*
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include "print_fields.h"

static const char *errstr;

struct tp_stats {
	unsigned int tp_packets, tp_drops, tp_freeze_q_cnt;
};

static long
get_tpacket_stats(void *optval, socklen_t *len)
{
	struct tp_stats *tpstats = optval;
	socklen_t optlen = *len;
	long rc = getsockopt(-1, SOL_PACKET, PACKET_STATISTICS, tpstats, len);
	errstr = sprintrc(rc);
#ifdef INJECT_RETVAL
	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Got a return value of %ld != %d",
				   rc, INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
#endif
	printf("getsockopt(-1, SOL_PACKET, PACKET_STATISTICS");
	if (rc < 0 || optlen <= 0) {
		printf(", %p", tpstats);
	} else if (optlen < sizeof(tpstats->tp_packets)) {
		printf(", {tp_packets=");
		print_quoted_hex(tpstats, optlen);
		printf("}");
	} else {
		printf(", {");
		PRINT_FIELD_U(*tpstats, tp_packets);

		if (optlen > offsetof(struct tp_stats, tp_drops)) {
			optlen -= offsetof(struct tp_stats, tp_drops);
			if (optlen < sizeof(tpstats->tp_drops)) {
				printf(", tp_drops=");
				print_quoted_hex(tpstats, optlen);
			} else {
				printf(", ");
				PRINT_FIELD_U(*tpstats, tp_drops);

				if (optlen > offsetof(struct tp_stats, tp_freeze_q_cnt) -
					   offsetof(struct tp_stats, tp_drops)) {
					optlen -= offsetof(struct tp_stats, tp_freeze_q_cnt) -
					       offsetof(struct tp_stats, tp_drops);
					if (optlen < sizeof(tpstats->tp_freeze_q_cnt)) {
						printf(", tp_freeze_q_cnt=");
						print_quoted_hex(tpstats, optlen);
					} else {
						printf(", ");
						PRINT_FIELD_U(*tpstats, tp_freeze_q_cnt);
					}
				}
			}
		}
		printf("}");
	}
	printf(", [%d]) = %s\n", *len, errstr);

	return rc;
}

int
main(void)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct tp_stats, tp_stats);
	TAIL_ALLOC_OBJECT_CONST_PTR(socklen_t, len);

	/* offset of (truncated) struct tp_stats.tp_packets */
	const unsigned int offset_tp_packets = offsetofend(struct tp_stats, tp_packets);
	const unsigned int tp_packets_truncated = offset_tp_packets - 1;
	/* offset of (truncated) struct tp_stats.tp_drops */
	const unsigned int offset_tp_drops = offsetofend(struct tp_stats, tp_drops);
	const unsigned int tp_drops_truncated = offset_tp_drops - 1;
	/* offset of (truncated) struct tp_stats.tp_freeze_q_cnt */
	const unsigned int offset_tp_freeze_q_cnt = offsetofend(struct tp_stats, tp_freeze_q_cnt);
	const unsigned int tp_freeze_q_cnt_truncated = offset_tp_freeze_q_cnt - 1;

	*len = sizeof(*tp_stats);

	/* classic getsockopt */
	unsigned int optlen = *len;
	get_tpacket_stats(tp_stats, &optlen);

	/* getsockopt with zero optlen */
	optlen = 0;
	get_tpacket_stats(tp_stats, &optlen);

	/*
	 * getsockopt with optlen less than offsetofend(struct tp_stats.tp_packets):
	 * the part of struct tp_stats.tp_packets is printed in hex.
	 */
	optlen = tp_packets_truncated;
	get_tpacket_stats(tp_stats, &optlen);

	/*
	 * getsockopt with optlen equals to offsetofend(struct tp_stats.tp_packets):
	 * struct tp_stats.tp_drops and struct tp_stats.offset_tp_freeze_q_cnt
	 * are not printed.
	 */
	optlen = offset_tp_packets;
	get_tpacket_stats(tp_stats, &optlen);

	/*
	 * getsockopt with optlen greater than offsetofend(struct tp_stats.tp_packets)
	 * but less than offsetofend(struct tp_stats, tp_drops):
	 * the part of struct tp_stats.tp_drops is printed in hex.
	 */
	optlen = tp_drops_truncated;
	get_tpacket_stats(tp_stats, &optlen);

	/*
	 * getsockopt with optlen equals to offsetofend(struct tp_stats.tp_drops):
	 * struct tp_stats.tp_freeze_q_cnt is not printed.
	 */
	optlen = offset_tp_drops;
	get_tpacket_stats(tp_stats, &optlen);

	/*
	 * getsockopt with optlen greater than offsetofend(struct tp_stats.tp_drops)
	 * but less than offsetofend(struct tp_stats, tp_freeze_q_cnt):
	 * the part of struct tp_stats.tp_freeze_q_cnt is printed in hex.
	 */
	optlen = tp_freeze_q_cnt_truncated;
	get_tpacket_stats(tp_stats, &optlen);

	/*
	 * getsockopt with optlen equals to offsetofend(struct tp_stats.tp_freeze_q_cnt):
	 */
	optlen = offset_tp_freeze_q_cnt;
	get_tpacket_stats(tp_stats, &optlen);

	/*
	 * getsockopt with optlen greater than sizeof(struct tp_stats)
	 */
	optlen = offset_tp_freeze_q_cnt + 1;
	get_tpacket_stats(tp_stats, &optlen);

	puts("+++ exited with 0 +++");
	return 0;
}
