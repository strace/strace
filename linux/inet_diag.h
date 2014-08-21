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

/* Request structure */
struct inet_diag_req_v2 {
	uint8_t sdiag_family;
	uint8_t sdiag_protocol;
	uint8_t idiag_ext;
	uint8_t pad;
	uint32_t idiag_states;
	struct inet_diag_sockid id;
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
