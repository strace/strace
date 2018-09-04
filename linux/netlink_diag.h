#ifndef STRACE_LINUX_NETLINK_DIAG_H
#define STRACE_LINUX_NETLINK_DIAG_H

struct netlink_diag_req {
	uint8_t sdiag_family;
	uint8_t sdiag_protocol;
	uint16_t pad;
	uint32_t ndiag_ino;
	uint32_t ndiag_show;
	uint32_t ndiag_cookie[2];
};

struct netlink_diag_msg {
	uint8_t ndiag_family;
	uint8_t ndiag_type;
	uint8_t ndiag_protocol;
	uint8_t ndiag_state;

	uint32_t ndiag_portid;
	uint32_t ndiag_dst_portid;
	uint32_t ndiag_dst_group;
	uint32_t ndiag_ino;
	uint32_t ndiag_cookie[2];
};

struct netlink_diag_ring {
	uint32_t ndr_block_size;
	uint32_t ndr_block_nr;
	uint32_t ndr_frame_size;
	uint32_t ndr_frame_nr;
};

#define NDIAG_PROTO_ALL			((uint8_t) ~0)

#endif /* !STRACE_LINUX_NETLINK_DIAG_H */
