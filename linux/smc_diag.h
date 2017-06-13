#ifndef STRACE_LINUX_SMC_DIAG_H
#define STRACE_LINUX_SMC_DIAG_H

#include <linux/inet_diag.h>

/* Request structure */
struct smc_diag_req {
	uint8_t diag_family;
	uint8_t pad[2];
	uint8_t diag_ext;		/* Query extended information */
	struct inet_diag_sockid	id;
};

struct smc_diag_msg {
	uint8_t diag_family;
	uint8_t diag_state;
	uint8_t diag_fallback;
	uint8_t diag_shutdown;
	struct inet_diag_sockid id;

	uint32_t diag_uid;
	uint64_t diag_inode;
};

/* Extensions */
enum {
	SMC_DIAG_NONE,
	SMC_DIAG_CONNINFO,
	SMC_DIAG_LGRINFO,
	SMC_DIAG_SHUTDOWN,
};

#endif /* !STRACE_LINUX_SMC_DIAG_H */
