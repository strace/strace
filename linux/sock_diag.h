#ifndef STRACE_LINUX_SOCK_DIAG_H
#define STRACE_LINUX_SOCK_DIAG_H

#define SOCK_DIAG_BY_FAMILY 20

struct sock_diag_req {
	uint8_t	sdiag_family;
	uint8_t	sdiag_protocol;
};

#endif /* !STRACE_LINUX_SOCK_DIAG_H */
