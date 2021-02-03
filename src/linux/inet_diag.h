/*
 * Copyright (c) 2014-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_LINUX_INET_DIAG_H
# define STRACE_LINUX_INET_DIAG_H

# define TCPDIAG_GETSOCK 18
# define DCCPDIAG_GETSOCK 19

/* Socket identity */
struct inet_diag_sockid {
	uint16_t idiag_sport;
	uint16_t idiag_dport;
	uint32_t idiag_src[4];
	uint32_t idiag_dst[4];
	uint32_t idiag_if;
	uint32_t idiag_cookie[2];
};

/* Request structures */
struct inet_diag_req {
	uint8_t idiag_family;
	uint8_t idiag_src_len;
	uint8_t idiag_dst_len;
	uint8_t idiag_ext;
	struct inet_diag_sockid id;
	uint32_t idiag_states;
	uint32_t idiag_dbs;
};

struct inet_diag_req_v2 {
	uint8_t sdiag_family;
	uint8_t sdiag_protocol;
	uint8_t idiag_ext;
	uint8_t pad;
	uint32_t idiag_states;
	struct inet_diag_sockid id;
};

enum {
	INET_DIAG_REQ_NONE,
	INET_DIAG_REQ_BYTECODE,
	INET_DIAG_REQ_SK_BPF_STORAGES,
	INET_DIAG_REQ_PROTOCOL,
};

struct inet_diag_bc_op {
	unsigned char code;
	unsigned char yes;
	unsigned short no;
};

enum {
	INET_DIAG_BC_NOP,
	INET_DIAG_BC_JMP,
	INET_DIAG_BC_S_GE,
	INET_DIAG_BC_S_LE,
	INET_DIAG_BC_D_GE,
	INET_DIAG_BC_D_LE,
	INET_DIAG_BC_AUTO,
	INET_DIAG_BC_S_COND,
	INET_DIAG_BC_D_COND,
	INET_DIAG_BC_DEV_COND,   /* u32 ifindex */
	INET_DIAG_BC_MARK_COND,
	INET_DIAG_BC_S_EQ,
	INET_DIAG_BC_D_EQ,
	INET_DIAG_BC_CGROUP_COND,
};

struct inet_diag_hostcond {
	uint8_t family;
	uint8_t prefix_len;
	int port;
	uint32_t addr[0];
};

struct inet_diag_markcond {
	uint32_t mark;
	uint32_t mask;
};

/* Info structure */
struct inet_diag_msg {
	uint8_t idiag_family;
	uint8_t idiag_state;
	uint8_t idiag_timer;
	uint8_t idiag_retrans;

	struct inet_diag_sockid id;

	uint32_t idiag_expires;
	uint32_t idiag_rqueue;
	uint32_t idiag_wqueue;
	uint32_t idiag_uid;
	uint32_t idiag_inode;
};

/* Extensions */
enum {
	INET_DIAG_NONE,
	INET_DIAG_MEMINFO,
	INET_DIAG_INFO,
	INET_DIAG_VEGASINFO,
	INET_DIAG_CONG,
	INET_DIAG_TOS,
	INET_DIAG_TCLASS,
	INET_DIAG_SKMEMINFO,
	INET_DIAG_SHUTDOWN,
	INET_DIAG_DCTCPINFO,
	INET_DIAG_PROTOCOL,  /* response attribute only */
	INET_DIAG_SKV6ONLY,
	INET_DIAG_LOCALS,
	INET_DIAG_PEERS,
	INET_DIAG_PAD,
	INET_DIAG_MARK,
	INET_DIAG_BBRINFO,
	INET_DIAG_CLASS_ID,
	INET_DIAG_MD5SIG,
	INET_DIAG_ULP_INFO,
	INET_DIAG_SK_BPF_STORAGES,
	INET_DIAG_CGROUP_ID,
	INET_DIAG_SOCKOPT,
};

/* INET_DIAG_MEM */
struct inet_diag_meminfo {
	uint32_t idiag_rmem;
	uint32_t idiag_wmem;
	uint32_t idiag_fmem;
	uint32_t idiag_tmem;
};

/* INET_DIAG_VEGASINFO */
struct tcpvegas_info {
	uint32_t tcpv_enabled;
	uint32_t tcpv_rttcnt;
	uint32_t tcpv_rtt;
	uint32_t tcpv_minrtt;
};

/* INET_DIAG_DCTCPINFO */
struct tcp_dctcp_info {
	uint16_t dctcp_enabled;
	uint16_t dctcp_ce_state;
	uint32_t dctcp_alpha;
	uint32_t dctcp_ab_ecn;
	uint32_t dctcp_ab_tot;
};

/* INET_DIAG_BBRINFO */
struct tcp_bbr_info {
	uint32_t bbr_bw_lo;
	uint32_t bbr_bw_hi;
	uint32_t bbr_min_rtt;
	uint32_t bbr_pacing_gain;
	uint32_t bbr_cwnd_gain;
};

#endif /* !STRACE_LINUX_INET_DIAG_H */
