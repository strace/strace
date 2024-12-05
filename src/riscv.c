/*
 * RISC-V-specific syscall decoders.
 *
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef RISCV64

# include "xlat/riscv_flush_icache_flags.h"
# include "xlat/riscv_hwprobe_flags.h"
# include "xlat/riscv_hwprobe_keys.h"
# include "xlat/riscv_hwprobe_base_behavior_flags.h"
# include "xlat/riscv_hwprobe_ima_ext_0_flags.h"
# include "xlat/riscv_hwprobe_cpuperf_0_flags.h"
# include "xlat/riscv_hwprobe_misaligned_scalar_perf.h"

SYS_FUNC(riscv_flush_icache)
{
	/* uintptr_t start */
	printaddr(tcp->u_arg[0]);
	tprint_arg_next();

	/* uintptr_t end */
	printaddr(tcp->u_arg[1]);
	tprint_arg_next();

	/* uintptr_t flags */
	printflags64(riscv_flush_icache_flags, tcp->u_arg[2],
		     "SYS_RISCV_FLUSH_ICACHE_???");

	return RVAL_DECODED;
}

struct riscv_hwprobe {
	int64_t key;
	uint64_t value;
};

static bool
print_riscv_hwprobe_pair(struct tcb *tcp, void *buf, size_t size, void *data)
{
	struct riscv_hwprobe *hwprobe = buf;
	bool value_valid = (bool)data;
	tprint_struct_begin();

	/* Invalid keys will be overwritten as -1 */
	if (hwprobe->key == -1) {
		PRINT_FIELD_D(*hwprobe, key);
		goto out;
	}

	PRINT_FIELD_XVAL(*hwprobe, key, riscv_hwprobe_keys,
			 "RISCV_HWPROBE_KEY_???");

	if (!value_valid)
		goto out;

	tprint_struct_next();

	switch (hwprobe->key) {
	case RISCV_HWPROBE_KEY_BASE_BEHAVIOR:
		PRINT_FIELD_FLAGS(*hwprobe, value,
				  riscv_hwprobe_base_behavior_flags,
				  "RISCV_HWPROBE_BASE_BEHAVIOR_???");
		break;

	case RISCV_HWPROBE_KEY_IMA_EXT_0:
		PRINT_FIELD_FLAGS(*hwprobe, value,
				  riscv_hwprobe_ima_ext_0_flags,
				  "RISCV_HWPROBE_IMA_EXT_0_???");
		break;

	case RISCV_HWPROBE_KEY_CPUPERF_0:
		PRINT_FIELD_XVAL(*hwprobe, value,
				 riscv_hwprobe_cpuperf_0_flags,
				 "RISCV_HWPROBE_MISALIGNED_???");
		break;

	case RISCV_HWPROBE_KEY_MISALIGNED_SCALAR_PERF:
		PRINT_FIELD_XVAL(*hwprobe, value,
				 riscv_hwprobe_misaligned_scalar_perf,
				 "RISCV_HWPROBE_MISALIGNED_SCALAR_???");
		break;

	case RISCV_HWPROBE_KEY_ZICBOZ_BLOCK_SIZE:
	case RISCV_HWPROBE_KEY_TIME_CSR_FREQ:
		PRINT_FIELD_U(*hwprobe, value);
		break;

	case RISCV_HWPROBE_KEY_MVENDORID:
	case RISCV_HWPROBE_KEY_MARCHID:
	case RISCV_HWPROBE_KEY_MIMPID:
	case RISCV_HWPROBE_KEY_HIGHEST_VIRT_ADDRESS:
	default:
		PRINT_FIELD_X(*hwprobe, value);
		break;
	}

out:
	tprint_struct_end();
	return true;
}

SYS_FUNC(riscv_hwprobe)
{
	/*
	 * riscv_hwprobe() could be used in two ways, controlled by
	 * RISCV_HWPROBE_WHICH_CPUs flag,
	 *  - Without the flag, probe information for specified CPUs
	 *  - With the flag, probe CPUs matching specified information
	 *
	 * We trace and print both at entering and exiting, since all use
	 * cases overwrite parameters with a meaningful value originally.
	 */
	if (entering(tcp))
		tcp->flags |= TCB_REPRINT;

	/*
	 * Without RISCV_HWPROBE_WHICH_CPUS, values of pairs only become
	 * meaningful when leaving the syscall
	 */
	bool value_valid = exiting(tcp) ||
			   (tcp->u_arg[4] & RISCV_HWPROBE_WHICH_CPUS);

	/* struct riscv_hwprobe *pairs */
	struct riscv_hwprobe buf;
	print_array(tcp, tcp->u_arg[0], tcp->u_arg[1], &buf, sizeof(buf),
		    tfetch_mem, print_riscv_hwprobe_pair, (void *)value_valid);
	tprint_arg_next();

	/* size_t pair_count */
	PRINT_VAL_U(tcp->u_arg[1]);
	tprint_arg_next();

	/* size_t cpusetsize */
	const unsigned int len = tcp->u_arg[2];
	PRINT_VAL_U(len);
	tprint_arg_next();

	/* cpu_set_t *cpus */
	print_affinitylist(tcp, tcp->u_arg[3], len);
	tprint_arg_next();

	/* unsigned int flags */
	printflags64(riscv_hwprobe_flags, tcp->u_arg[4], "RISCV_HWPROBE_???");

	return 0;
}

#endif /* RISCV64 */
