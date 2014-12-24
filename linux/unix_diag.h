struct unix_diag_req {
	uint8_t	 sdiag_family;
	uint8_t	 sdiag_protocol;
	uint16_t pad;
	uint32_t udiag_states;
	uint32_t udiag_ino;
	uint32_t udiag_show;
	uint32_t udiag_cookie[2];
};

#define UDIAG_SHOW_NAME		0x01
#define UDIAG_SHOW_PEER		0x04

struct unix_diag_msg {
	uint8_t	 udiag_family;
	uint8_t	 udiag_type;
	uint8_t	 udiag_state;
	uint8_t	 pad;
	uint32_t udiag_ino;
	uint32_t udiag_cookie[2];
};

#define UNIX_DIAG_NAME 0
#define UNIX_DIAG_PEER 2
