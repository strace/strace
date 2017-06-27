#ifndef STRACE_LINUX_INET_DIAG_H
#define STRACE_LINUX_INET_DIAG_H

#define TCPDIAG_GETSOCK 18
#define DCCPDIAG_GETSOCK 19

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
};

#endif /* !STRACE_LINUX_INET_DIAG_H */
