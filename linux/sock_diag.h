#define SOCK_DIAG_BY_FAMILY 20

struct sock_diag_req {
	uint8_t	sdiag_family;
	uint8_t	sdiag_protocol;
};
