#ifndef STRACE_LINUX_PACKET_DIAG_H
#define STRACE_LINUX_PACKET_DIAG_H

struct packet_diag_req {
	uint8_t sdiag_family;
	uint8_t sdiag_protocol;
	uint16_t pad;
	uint32_t pdiag_ino;
	uint32_t pdiag_show;
	uint32_t pdiag_cookie[2];
};

#define PACKET_SHOW_INFO	0x00000001
#define PACKET_SHOW_MCLIST	0x00000002
#define PACKET_SHOW_RING_CFG	0x00000004
#define PACKET_SHOW_FANOUT	0x00000008
#define PACKET_SHOW_MEMINFO	0x00000010
#define PACKET_SHOW_FILTER	0x00000020

struct packet_diag_msg {
	uint8_t pdiag_family;
	uint8_t pdiag_type;
	uint16_t pdiag_num;

	uint32_t pdiag_ino;
	uint32_t pdiag_cookie[2];
};

#endif /* !STRACE_LINUX_PACKET_DIAG_H */
