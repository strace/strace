/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
 * Copyright (c) 2017 The strace developers.
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
#include "netlink.h"

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <linux/inet_diag.h>
#include <linux/rtnetlink.h>
#include <linux/sock_diag.h>

static void
init_inet_diag_msg(struct nlmsghdr *nlh, unsigned int msg_len,
		   const char *address)
{
	struct inet_diag_msg *msg;

	SET_STRUCT(struct nlmsghdr, nlh,
		.nlmsg_len = msg_len,
		.nlmsg_type = SOCK_DIAG_BY_FAMILY,
		.nlmsg_flags = NLM_F_DUMP
	);

	msg = NLMSG_DATA(nlh);
	SET_STRUCT(struct inet_diag_msg, msg,
		.idiag_family = AF_INET,
		.idiag_state = TCP_LISTEN
	);

	if (!inet_pton(AF_INET, address, msg->id.idiag_src) ||
	    !inet_pton(AF_INET, address, msg->id.idiag_dst))
		perror_msg_and_skip("inet_pton");
}

static void
test_inet_diag_meminfo(const int fd)
{
	const int hdrlen = sizeof(struct inet_diag_msg);
	const char address[] = "87.65.43.21";
	struct nlmsghdr *nlh;
	struct nlattr *nla;
	unsigned int nla_len;
	unsigned int msg_len;
	void *const nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));
	long rc;

	/* len < sizeof(struct inet_diag_meminfo) */
	nla_len = NLA_HDRLEN + 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_MEMINFO
	);
	memcpy(RTA_DATA(nla), "12", 2);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_MEMINFO}"
	       ", \"12\"}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       msg_len, sprintrc(rc));

	/* short read of inet_diag_meminfo */
	nla_len = NLA_HDRLEN + sizeof(struct inet_diag_meminfo);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - (nla_len - 1);
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_MEMINFO
	);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_MEMINFO}"
	       ", %p}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       RTA_DATA(nla), msg_len, sprintrc(rc));

	/* inet_diag_meminfo */
	nla_len = NLA_HDRLEN + sizeof(struct inet_diag_meminfo);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_MEMINFO
	);
	static const struct inet_diag_meminfo minfo = {
		.idiag_rmem = 0xfadcacdb,
		.idiag_wmem = 0xbdabcada,
		.idiag_fmem = 0xbadbfafb,
		.idiag_tmem = 0xfdacdadf
	};
	memcpy(RTA_DATA(nla), &minfo, sizeof(minfo));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_MEMINFO}"
	       ", {idiag_rmem=%u, idiag_wmem=%u, idiag_fmem=%u, idiag_tmem=%u}}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       minfo.idiag_rmem, minfo.idiag_wmem,
	       minfo.idiag_fmem, minfo.idiag_tmem,
	       msg_len, sprintrc(rc));
}

static void
test_inet_diag_vegasinfo(const int fd)
{
	const int hdrlen = sizeof(struct inet_diag_msg);
	const char address[] = "87.65.43.21";
	struct nlmsghdr *nlh;
	struct nlattr *nla;
	unsigned int nla_len;
	unsigned int msg_len;
	void *const nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));
	long rc;

	/* len < sizeof(struct tcpvegas_info) */
	nla_len = NLA_HDRLEN + 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_VEGASINFO
	);
	memcpy(RTA_DATA(nla), "12", 2);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_VEGASINFO}"
	       ", \"12\"}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       msg_len, sprintrc(rc));

	/* short read of tcpvegas_info */
	nla_len = NLA_HDRLEN + sizeof(struct tcpvegas_info);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - (nla_len - 1);
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_VEGASINFO
	);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_VEGASINFO}"
	       ", %p}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       RTA_DATA(nla), msg_len, sprintrc(rc));

	/* tcpvegas_info */
	nla_len = NLA_HDRLEN + sizeof(struct tcpvegas_info);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_VEGASINFO
	);
	static const struct tcpvegas_info vegas = {
		.tcpv_enabled = 0xfadcacdb,
		.tcpv_rttcnt = 0xbdabcada,
		.tcpv_rtt = 0xbadbfafb,
		.tcpv_minrtt = 0xfdacdadf
	};
	memcpy(RTA_DATA(nla), &vegas, sizeof(vegas));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_VEGASINFO}"
	       ", {tcpv_enabled=%u, tcpv_rttcnt=%u, tcpv_rtt=%u"
	       ", tcpv_minrtt=%u}}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       vegas.tcpv_enabled, vegas.tcpv_rttcnt,
	       vegas.tcpv_rtt, vegas.tcpv_minrtt,
	       msg_len, sprintrc(rc));
}

static void
test_inet_diag_skmeminfo(const int fd)
{
	const int hdrlen = sizeof(struct inet_diag_msg);
	const char address[] = "87.65.43.21";
	struct nlmsghdr *nlh;
	struct nlattr *nla;
	unsigned int nla_len;
	unsigned int msg_len;
	void *const nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));
	long rc;

	/* len < sizeof(uint32_t) */
	nla_len = NLA_HDRLEN + 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_SKMEMINFO
	);
	memcpy(RTA_DATA(nla), "12", 2);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u"
	       ", nla_type=INET_DIAG_SKMEMINFO}, \"12\"}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       msg_len, sprintrc(rc));

	/* len = sizeof(uint32_t) * 2 - 1 */
	nla_len = NLA_HDRLEN + sizeof(uint32_t) * 2 - 1;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_SKMEMINFO
	);
	static const uint32_t mem[] = { 0xaffacbad, 0xffadbcab };
	memcpy(RTA_DATA(nla), mem, sizeof(mem[0]));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_SKMEMINFO}"
	       ", [%u]}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       mem[0], msg_len, sprintrc(rc));

	/* len = sizeof(uint32_t) * 2 */
	nla_len = NLA_HDRLEN + sizeof(uint32_t) * 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_SKMEMINFO
	);
	memcpy(RTA_DATA(nla), mem, sizeof(mem));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_SKMEMINFO}"
	       ", [%u, %u]}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       mem[0], mem[1], msg_len, sprintrc(rc));
}

static void
test_inet_diag_dctcpinfo(const int fd)
{
	const int hdrlen = sizeof(struct inet_diag_msg);
	const char address[] = "87.65.43.21";
	struct nlmsghdr *nlh;
	struct nlattr *nla;
	unsigned int nla_len;
	unsigned int msg_len;
	void *const nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));
	long rc;

	/* len < sizeof(struct tcp_dctcp_info) */
	nla_len = NLA_HDRLEN + 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_DCTCPINFO
	);
	memcpy(RTA_DATA(nla), "12", 2);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_DCTCPINFO}"
	       ", \"12\"}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       msg_len, sprintrc(rc));

	/* short read of tcp_dctcp_info */
	nla_len = NLA_HDRLEN + sizeof(struct tcp_dctcp_info);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - (nla_len - 1);
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_DCTCPINFO
	);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_DCTCPINFO}"
	       ", %p}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       RTA_DATA(nla), msg_len, sprintrc(rc));

	/* tcp_dctcp_info */
	nla_len = NLA_HDRLEN + sizeof(struct tcp_dctcp_info);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_DCTCPINFO
	);
	static const struct tcp_dctcp_info dctcp = {
		.dctcp_enabled = 0xfdac,
		.dctcp_ce_state = 0xfadc,
		.dctcp_alpha = 0xbdabcada,
		.dctcp_ab_ecn = 0xbadbfafb,
		.dctcp_ab_tot = 0xfdacdadf
	};
	memcpy(RTA_DATA(nla), &dctcp, sizeof(dctcp));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_DCTCPINFO}"
	       ", {dctcp_enabled=%u, dctcp_ce_state=%u"
	       ", dctcp_alpha=%u, dctcp_ab_ecn=%u, dctcp_ab_tot=%u}}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       dctcp.dctcp_enabled, dctcp.dctcp_ce_state,
	       dctcp.dctcp_alpha, dctcp.dctcp_ab_ecn,
	       dctcp.dctcp_ab_tot, msg_len, sprintrc(rc));
}

static void
test_inet_diag_bbrinfo(const int fd)
{
	const int hdrlen = sizeof(struct inet_diag_msg);
	const char address[] = "12.34.56.78";
	struct nlmsghdr *nlh;
	struct nlattr *nla;
	unsigned int nla_len;
	unsigned int msg_len;
	void *const nlh0 = tail_alloc(NLMSG_SPACE(hdrlen));
	long rc;

	/* len < sizeof(struct tcp_bbr_info) */
	nla_len = NLA_HDRLEN + 2;
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_BBRINFO
	);
	memcpy(RTA_DATA(nla), "12", 2);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_BBRINFO}"
	       ", \"12\"}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       msg_len, sprintrc(rc));

	/* short read of tcp_bbr_info */
	nla_len = NLA_HDRLEN + sizeof(struct tcp_bbr_info);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - (nla_len - 1);
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_BBRINFO
	);

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_BBRINFO}"
	       ", %p}}, %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       RTA_DATA(nla), msg_len, sprintrc(rc));

	/* tcp_bbr_info */
	nla_len = NLA_HDRLEN + sizeof(struct tcp_bbr_info);
	msg_len = NLMSG_SPACE(hdrlen) + nla_len;
	nlh = nlh0 - nla_len;
	init_inet_diag_msg(nlh, msg_len, address);

	nla = NLMSG_ATTR(nlh, hdrlen);
	SET_STRUCT(struct nlattr, nla,
		.nla_len = nla_len,
		.nla_type = INET_DIAG_BBRINFO
	);
	static const struct tcp_bbr_info bbr = {
		.bbr_bw_lo = 0xfdacdadf,
		.bbr_bw_hi = 0xfadcacdb,
		.bbr_min_rtt = 0xbdabcada,
		.bbr_pacing_gain = 0xbadbfafb,
		.bbr_cwnd_gain = 0xfdacdadf
	};
	memcpy(RTA_DATA(nla), &bbr, sizeof(bbr));

	rc = sendto(fd, nlh, msg_len, MSG_DONTWAIT, NULL, 0);

	printf("sendto(%d, {{len=%u, type=SOCK_DIAG_BY_FAMILY"
	       ", flags=NLM_F_DUMP, seq=0, pid=0}, {idiag_family=AF_INET"
	       ", idiag_state=TCP_LISTEN, idiag_timer=0, idiag_retrans=0"
	       ", id={idiag_sport=htons(0), idiag_dport=htons(0)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_src)"
	       ", inet_pton(AF_INET, \"%s\", &idiag_dst)"
	       ", idiag_if=0, idiag_cookie=[0, 0]}, idiag_expires=0"
	       ", idiag_rqueue=0, idiag_wqueue=0, idiag_uid=0"
	       ", idiag_inode=0}, {{nla_len=%u, nla_type=INET_DIAG_BBRINFO}"
	       ", {bbr_bw_lo=%#x, bbr_bw_hi=%#x, bbr_min_rtt=%u"
	       ", bbr_pacing_gain=%u, bbr_cwnd_gain=%u}}}"
	       ", %u, MSG_DONTWAIT, NULL, 0) = %s\n",
	       fd, msg_len, address, address, nla_len,
	       bbr.bbr_bw_lo, bbr.bbr_bw_hi, bbr.bbr_min_rtt,
	       bbr.bbr_pacing_gain, bbr.bbr_cwnd_gain,
	       msg_len, sprintrc(rc));
}

int main(void)
{
	skip_if_unavailable("/proc/self/fd/");

	int fd = create_nl_socket(NETLINK_SOCK_DIAG);

	test_inet_diag_meminfo(fd);
	test_inet_diag_vegasinfo(fd);
	test_inet_diag_skmeminfo(fd);
	test_inet_diag_dctcpinfo(fd);
	test_inet_diag_bbrinfo(fd);

	printf("+++ exited with 0 +++\n");

	return 0;
}
