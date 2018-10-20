/*
 * Copyright (c) 2017-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#ifndef STRACE_LINUX_SMC_DIAG_H
# define STRACE_LINUX_SMC_DIAG_H

# include <linux/inet_diag.h>

# include "gcc_compat.h"

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
	union {
		uint8_t diag_fallback;
		uint8_t diag_mode;
	};
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
	SMC_DIAG_DMBINFO,
	SMC_DIAG_FALLBACK,
};

/* SMC_DIAG_CONNINFO */
struct smc_diag_cursor {
	uint16_t reserved;
	uint16_t wrap;
	uint32_t count;
};

struct smc_diag_conninfo {
	uint32_t		token;
	uint32_t		sndbuf_size;
	uint32_t		rmbe_size;
	uint32_t		peer_rmbe_size;
	struct smc_diag_cursor	rx_prod;
	struct smc_diag_cursor	rx_cons;
	struct smc_diag_cursor	tx_prod;
	struct smc_diag_cursor	tx_cons;
	uint8_t			rx_prod_flags;
	uint8_t			rx_conn_state_flags;
	uint8_t			tx_prod_flags;
	uint8_t			tx_conn_state_flags;
	struct smc_diag_cursor	tx_prep;
	struct smc_diag_cursor	tx_sent;
	struct smc_diag_cursor	tx_fin;
};

/* SMC_DIAG_LINKINFO */
struct smc_diag_linkinfo {
	uint8_t link_id;
	uint8_t ibname[64]; /* IB_DEVICE_NAME_MAX */
	uint8_t ibport;
	uint8_t gid[40];
	uint8_t peer_gid[40];
};

/* SMC_DIAG_LGRINFO */
struct smc_diag_lgrinfo {
	struct smc_diag_linkinfo lnk[1];
	uint8_t role;
};

/* SMC_DIAG_DMBINFO */
struct smcd_diag_dmbinfo {
	uint32_t linkid;
	uint64_t ATTRIBUTE_ALIGNED(8) peer_gid;
	uint64_t ATTRIBUTE_ALIGNED(8) my_gid;
	uint64_t ATTRIBUTE_ALIGNED(8) token;
	uint64_t ATTRIBUTE_ALIGNED(8) peer_token;
};

/* SMC_DIAG_FALLBACK */
struct smc_diag_fallback {
	uint32_t reason;
	uint32_t peer_diagnosis;
};

#endif /* !STRACE_LINUX_SMC_DIAG_H */
