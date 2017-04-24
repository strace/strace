#include "tests.h"

#ifdef HAVE_LINUX_BTRFS_H

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/vfs.h>
#include <linux/fs.h>
#include <linux/btrfs.h>
#include <linux/magic.h>
#include "xlat.h"

#include "xlat/btrfs_balance_args.h"
#include "xlat/btrfs_balance_flags.h"
#include "xlat/btrfs_balance_state.h"
#include "xlat/btrfs_compress_types.h"
#include "xlat/btrfs_defrag_flags.h"
#include "xlat/btrfs_dev_stats_values.h"
#include "xlat/btrfs_dev_stats_flags.h"
#include "xlat/btrfs_qgroup_inherit_flags.h"
#include "xlat/btrfs_qgroup_limit_flags.h"
#include "xlat/btrfs_scrub_flags.h"
#include "xlat/btrfs_send_flags.h"
#include "xlat/btrfs_space_info_flags.h"
#include "xlat/btrfs_snap_flags_v2.h"
#include "xlat/btrfs_tree_objectids.h"
#include "xlat/btrfs_features_compat.h"
#include "xlat/btrfs_features_compat_ro.h"
#include "xlat/btrfs_features_incompat.h"
#include "xlat/btrfs_key_types.h"

#ifdef HAVE_LINUX_FIEMAP_H
# include <linux/fiemap.h>
# include "xlat/fiemap_flags.h"
# include "xlat/fiemap_extent_flags.h"
#endif

#ifndef BTRFS_LABEL_SIZE
# define BTRFS_LABEL_SIZE 256
#endif

#ifndef BTRFS_NAME_LEN
# define BTRFS_NAME_LEN 255
#endif

/*
 * Prior to Linux 3.12, the BTRFS_IOC_DEFAULT_SUBVOL used u64 in
 * its definition, which isn't exported by the kernel.
 */
typedef __u64 u64;

static const char *btrfs_test_root;
static int btrfs_test_dir_fd;
static bool verbose = false;
static bool write_ok = false;

const unsigned char uuid_reference[BTRFS_UUID_SIZE] = {
	0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,
	0xfe, 0xdc, 0xba, 0x98, 0x76, 0x54, 0x32, 0x10,
};

const char uuid_reference_string[] = "01234567-89ab-cdef-fedc-ba9876543210";

#ifndef BTRFS_IOC_QUOTA_RESCAN
struct btrfs_ioctl_quota_rescan_args {
	uint64_t flags, progress, reserved[6];
};
# define BTRFS_IOC_QUOTA_RESCAN _IOW(BTRFS_IOCTL_MAGIC, 44, \
					struct btrfs_ioctl_quota_rescan_args)
# define BTRFS_IOC_QUOTA_RESCAN_STATUS _IOR(BTRFS_IOCTL_MAGIC, 45, \
					struct btrfs_ioctl_quota_rescan_args)
#endif

#ifndef BTRFS_IOC_QUOTA_RESCAN_WAIT
# define BTRFS_IOC_QUOTA_RESCAN_WAIT _IO(BTRFS_IOCTL_MAGIC, 46)
#endif

#ifndef BTRFS_IOC_GET_FEATURES
# define BTRFS_IOC_GET_FEATURES _IOR(BTRFS_IOCTL_MAGIC, 57, \
                                   struct btrfs_ioctl_feature_flags)
# define BTRFS_IOC_SET_FEATURES _IOW(BTRFS_IOCTL_MAGIC, 57, \
                                   struct btrfs_ioctl_feature_flags[2])
# define BTRFS_IOC_GET_SUPPORTED_FEATURES _IOR(BTRFS_IOCTL_MAGIC, 57, \
                                   struct btrfs_ioctl_feature_flags[3])
#endif

#ifndef HAVE_STRUCT_BTRFS_IOCTL_FEATURE_FLAGS_COMPAT_FLAGS
struct btrfs_ioctl_feature_flags {
        uint64_t compat_flags;
        uint64_t compat_ro_flags;
        uint64_t incompat_flags;
};
#endif

#ifndef HAVE_STRUCT_BTRFS_IOCTL_DEFRAG_RANGE_ARGS_START
struct btrfs_ioctl_defrag_range_args {
        uint64_t start;
        uint64_t len;
        uint64_t flags;
        uint32_t extent_thresh;
        uint32_t compress_type;
        uint32_t unused[4];
};
#endif

#ifndef FIDEDUPERANGE
# define FIDEDUPERANGE   _IOWR(0x94, 54, struct file_dedupe_range)
struct file_dedupe_range_info {
	int64_t dest_fd;	/* in - destination file */
	uint64_t dest_offset;	/* in - start of extent in destination */
	uint64_t bytes_deduped;	/* out - total # of bytes we were able
				 * to dedupe from this file. */
	/* status of this dedupe operation:
	 * < 0 for error
	 * == FILE_DEDUPE_RANGE_SAME if dedupe succeeds
	 * == FILE_DEDUPE_RANGE_DIFFERS if data differs
	 */
	int32_t status;		/* out - see above description */
	uint32_t reserved;	/* must be zero */
};

struct file_dedupe_range {
	uint64_t src_offset;	/* in - start of extent in source */
	uint64_t src_length;	/* in - length of extent */
	uint16_t dest_count;	/* in - total elements in info array */
	uint16_t reserved1;	/* must be zero */
	uint32_t reserved2;	/* must be zero */
	struct file_dedupe_range_info info[0];
};
#endif

#ifndef BTRFS_IOC_TREE_SEARCH_V2
# define BTRFS_IOC_TREE_SEARCH_V2 _IOWR(BTRFS_IOCTL_MAGIC, 17, \
                                           struct btrfs_ioctl_search_args_v2)
struct btrfs_ioctl_search_args_v2 {
	struct btrfs_ioctl_search_key key; /* in/out - search parameters */
	uint64_t buf_size;		   /* in - size of buffer
					    * out - on EOVERFLOW: needed size
					    *       to store item */
        uint64_t buf[0];		   /* out - found items */
};
#endif


static const char *
maybe_print_uint64max(uint64_t val)
{
	if (val == UINT64_MAX)
		return " /* UINT64_MAX */";
	return "";
}

/* takes highest valid flag bit */
static uint64_t
max_flags_plus_one(int bit)
{
	int i;
	uint64_t val = 0;
	if (bit == -1)
		return 1;
	for (i = 0; i <= bit + 1 && i < 64; i++)
		val |= (1ULL << i);
	return val;
}

/*
 * Consumes no arguments, returns nothing:
 *
 * - BTRFS_IOC_TRANS_START
 * - BTRFS_IOC_TRANS_END
 */
static void
btrfs_test_trans_ioctls(void)
{
	ioctl(-1, BTRFS_IOC_TRANS_START, NULL);
	printf("ioctl(-1, BTRFS_IOC_TRANS_START) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_TRANS_END, NULL);
	printf("ioctl(-1, BTRFS_IOC_TRANS_END) = -1 EBADF (%m)\n");
}

/*
 * Consumes no arguments, returns nothing:
 * - BTRFS_IOC_SYNC
 *
 * Consumes argument, returns nothing
 * - BTRFS_IOC_WAIT_SYNC
 */
static void
btrfs_test_sync_ioctls(void)
{
	uint64_t u64val = 0xdeadbeefbadc0dedULL;

	ioctl(-1, BTRFS_IOC_SYNC, NULL);
	printf("ioctl(-1, BTRFS_IOC_SYNC) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_WAIT_SYNC, NULL);
	printf("ioctl(-1, BTRFS_IOC_WAIT_SYNC, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_WAIT_SYNC, &u64val);
	printf("ioctl(-1, BTRFS_IOC_WAIT_SYNC, [%" PRIu64
	       "]) = -1 EBADF (%m)\n", u64val);

	/*
	 * The live test of BTRFS_IOC_SYNC happens as a part of the test
	 * for BTRFS_IOC_LOGICAL_INO
	 */
}

static void
btrfs_print_qgroup_inherit(struct btrfs_qgroup_inherit *inherit)
{
	printf("{flags=");
	printflags(btrfs_qgroup_inherit_flags, inherit->flags,
		   "BTRFS_QGROUP_INHERIT_???");
	printf(", num_qgroups=%" PRI__u64
	       ", num_ref_copies=%" PRI__u64
	       ", num_excl_copies=%" PRI__u64", lim={flags=",
	       inherit->num_qgroups, inherit->num_ref_copies,
	       inherit->num_excl_copies);
	printflags(btrfs_qgroup_limit_flags,
		   inherit->lim.flags,
		   "BTRFS_QGROUP_LIMIT_???");
	printf(", max_rfer=%" PRI__u64 ", max_excl=%" PRI__u64
	       ", rsv_rfer=%" PRI__u64 ", rsv_excl=%" PRI__u64
	       "}, qgroups=",
	       inherit->lim.max_rfer, inherit->lim.max_excl,
	       inherit->lim.rsv_rfer, inherit->lim.rsv_excl);
	if (verbose) {
		unsigned int i;
		printf("[");
		for (i = 0; i < inherit->num_qgroups; i++) {
			if (i > 0)
				printf(", ");
			printf("%" PRI__u64, inherit->qgroups[i]);
		}
		printf("]");
	} else
		printf("...");
	printf("}");
}


static void
btrfs_print_vol_args_v2(struct btrfs_ioctl_vol_args_v2 *args, int print_qgroups)
{
	printf("{fd=%d, flags=", (int) args->fd);
	printflags(btrfs_snap_flags_v2, args->flags, "BTRFS_SUBVOL_???");

	if (args->flags & BTRFS_SUBVOL_QGROUP_INHERIT) {
		printf(", size=%" PRI__u64 ", qgroup_inherit=", args->size);
		if (args->qgroup_inherit && print_qgroups)
			btrfs_print_qgroup_inherit(args->qgroup_inherit);
		else if (args->qgroup_inherit)
			printf("%p", args->qgroup_inherit);
		else
			printf("NULL");
	}
	printf(", name=\"%s\"}", args->name);
}

/*
 * Consumes argument, returns nothing:
 * - BTRFS_IOC_SNAP_CREATE
 * - BTRFS_IOC_SUBVOL_CREATE
 * - BTRFS_IOC_SNAP_DESTROY
 * - BTRFS_IOC_DEFAULT_SUBVOL
 *
 * Consumes argument, returns u64:
 * - BTRFS_IOC_SNAP_CREATE_V2
 * - BTRFS_IOC_SUBVOL_CREATE_V2
 */

static void
btrfs_test_subvol_ioctls(void)
{
	const char *subvol_name = "subvol-name";
	char *long_subvol_name;
	void *bad_pointer = (void *) (unsigned long) 0xdeadbeeffffffeedULL;
	uint64_t u64val = 0xdeadbeefbadc0dedULL;
	struct btrfs_ioctl_vol_args vol_args = {};
	struct btrfs_ioctl_vol_args_v2 vol_args_v2 = {
		.fd = 2,
		.flags = max_flags_plus_one(2),
	};

	long_subvol_name = malloc(BTRFS_PATH_NAME_MAX);
	if (!long_subvol_name)
		perror_msg_and_fail("malloc failed");
	memset(long_subvol_name, 'f', BTRFS_PATH_NAME_MAX);
	long_subvol_name[BTRFS_PATH_NAME_MAX - 1] = '\0';

	strcpy(vol_args.name, subvol_name);

	ioctl(-1, BTRFS_IOC_SNAP_CREATE, NULL);
	printf("ioctl(-1, BTRFS_IOC_SNAP_CREATE, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_SNAP_CREATE, &vol_args);
	printf("ioctl(-1, BTRFS_IOC_SNAP_CREATE, "
	       "{fd=0, name=\"%s\"}) = -1 EBADF (%m)\n", vol_args.name);

	ioctl(-1, BTRFS_IOC_SUBVOL_CREATE, &vol_args);
	printf("ioctl(-1, BTRFS_IOC_SUBVOL_CREATE, "
	       "{fd=0, name=\"%s\"}) = -1 EBADF (%m)\n", vol_args.name);

	ioctl(-1, BTRFS_IOC_SNAP_DESTROY, &vol_args);
	printf("ioctl(-1, BTRFS_IOC_SNAP_DESTROY,"
	       " {fd=0, name=\"%s\"}) = -1 EBADF (%m)\n", vol_args.name);

	strncpy(vol_args.name, long_subvol_name, BTRFS_PATH_NAME_MAX);
	ioctl(-1, BTRFS_IOC_SNAP_CREATE, &vol_args);
	printf("ioctl(-1, BTRFS_IOC_SNAP_CREATE,"
	       " {fd=0, name=\"%s\"}) = -1 EBADF (%m)\n", vol_args.name);

	ioctl(-1, BTRFS_IOC_SUBVOL_CREATE, &vol_args);
	printf("ioctl(-1, BTRFS_IOC_SUBVOL_CREATE,"
	       " {fd=0, name=\"%s\"}) = -1 EBADF (%m)\n", vol_args.name);

	ioctl(-1, BTRFS_IOC_SNAP_DESTROY, &vol_args);
	printf("ioctl(-1, BTRFS_IOC_SNAP_DESTROY,"
	       " {fd=0, name=\"%s\"}) = -1 EBADF (%m)\n", vol_args.name);

	long_subvol_name = realloc(long_subvol_name, BTRFS_SUBVOL_NAME_MAX);
	if (!long_subvol_name)
		perror_msg_and_fail("realloc failed");

	ioctl(-1, BTRFS_IOC_SNAP_CREATE_V2, NULL);
	printf("ioctl(-1, BTRFS_IOC_SNAP_CREATE_V2, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_SUBVOL_CREATE_V2, NULL);
	printf("ioctl(-1, BTRFS_IOC_SUBVOL_CREATE_V2, NULL) = -1 EBADF (%m)\n");

	strcpy(vol_args_v2.name, subvol_name);
	printf("ioctl(-1, BTRFS_IOC_SNAP_CREATE_V2, ");
	btrfs_print_vol_args_v2(&vol_args_v2, 1);
	ioctl(-1, BTRFS_IOC_SNAP_CREATE_V2, &vol_args_v2);
	printf(") = -1 EBADF (%m)\n");

	printf("ioctl(-1, BTRFS_IOC_SUBVOL_CREATE_V2, ");
	btrfs_print_vol_args_v2(&vol_args_v2, 1);
	ioctl(-1, BTRFS_IOC_SUBVOL_CREATE_V2, &vol_args_v2);
	printf(") = -1 EBADF (%m)\n");

	strncpy(vol_args_v2.name, long_subvol_name, BTRFS_SUBVOL_NAME_MAX);
	printf("ioctl(-1, BTRFS_IOC_SNAP_CREATE_V2, ");
	btrfs_print_vol_args_v2(&vol_args_v2, 1);
	ioctl(-1, BTRFS_IOC_SNAP_CREATE_V2, &vol_args_v2);
	printf(") = -1 EBADF (%m)\n");

	printf("ioctl(-1, BTRFS_IOC_SUBVOL_CREATE_V2, ");
	btrfs_print_vol_args_v2(&vol_args_v2, 1);
	ioctl(-1, BTRFS_IOC_SUBVOL_CREATE_V2, &vol_args_v2);
	printf(") = -1 EBADF (%m)\n");

	strcpy(vol_args_v2.name, subvol_name);
	vol_args_v2.qgroup_inherit = bad_pointer;

	printf("ioctl(-1, BTRFS_IOC_SNAP_CREATE_V2, ");
	btrfs_print_vol_args_v2(&vol_args_v2, 0);
	ioctl(-1, BTRFS_IOC_SNAP_CREATE_V2, &vol_args_v2);
	printf(") = -1 EBADF (%m)\n");

	printf("ioctl(-1, BTRFS_IOC_SUBVOL_CREATE_V2, ");
	btrfs_print_vol_args_v2(&vol_args_v2, 0);
	ioctl(-1, BTRFS_IOC_SUBVOL_CREATE_V2, &vol_args_v2);
	printf(") = -1 EBADF (%m)\n");

	const unsigned int n_qgroups = 8;
	unsigned int i;
	struct btrfs_qgroup_inherit *inherit;
	vol_args_v2.size =
		sizeof(*inherit) + n_qgroups * sizeof(inherit->qgroups[0]);
	inherit = tail_alloc(vol_args_v2.size);

	inherit->flags = 0x3;
	inherit->num_ref_copies = 0;
	inherit->num_excl_copies = 0;
	inherit->num_qgroups = n_qgroups;
	for (i = 0; i < n_qgroups; i++)
		inherit->qgroups[i] = 1ULL << i;
	inherit->lim.flags = 0x7f;
	inherit->lim.max_rfer = u64val;
	inherit->lim.max_excl = u64val;
	inherit->lim.rsv_rfer = u64val;
	inherit->lim.rsv_excl = u64val;
	vol_args_v2.qgroup_inherit = inherit;

	printf("ioctl(-1, BTRFS_IOC_SNAP_CREATE_V2, ");
	btrfs_print_vol_args_v2(&vol_args_v2, 1);
	ioctl(-1, BTRFS_IOC_SNAP_CREATE_V2, &vol_args_v2);
	printf(") = -1 EBADF (%m)\n");

	printf("ioctl(-1, BTRFS_IOC_SUBVOL_CREATE_V2, ");
	btrfs_print_vol_args_v2(&vol_args_v2, 1);
	ioctl(-1, BTRFS_IOC_SUBVOL_CREATE_V2, &vol_args_v2);
	printf(") = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_DEFAULT_SUBVOL, NULL);
	printf("ioctl(-1, BTRFS_IOC_DEFAULT_SUBVOL, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_DEFAULT_SUBVOL, &u64val);
	printf("ioctl(-1, BTRFS_IOC_DEFAULT_SUBVOL, [%"
	       PRIu64 "]) = -1 EBADF (%m)\n", u64val);

	printf("ioctl(-1, BTRFS_IOC_SUBVOL_SETFLAGS, ");
	printflags(btrfs_snap_flags_v2, vol_args_v2.flags,
		   "BTRFS_SUBVOL_???");
	ioctl(-1, BTRFS_IOC_SUBVOL_SETFLAGS, &vol_args_v2.flags);
	printf(") = -1 EBADF (%m)\n");

	if (write_ok) {
		struct btrfs_ioctl_vol_args_v2 args_passed;
		/*
		 * Returns transid if flags & BTRFS_SUBVOL_CREATE_ASYNC
		 * - BTRFS_IOC_SNAP_CREATE_V2
		 * - BTRFS_IOC_SUBVOL_CREATE_V2
		 */
		int subvolfd;

		strncpy(vol_args_v2.name, subvol_name,
			sizeof(vol_args_v2.name));
		vol_args_v2.flags = BTRFS_SUBVOL_CREATE_ASYNC;
		vol_args_v2.size = 0;
		vol_args_v2.qgroup_inherit = NULL;
		args_passed = vol_args_v2;
		printf("ioctl(%d, BTRFS_IOC_SUBVOL_CREATE_V2, ",
			btrfs_test_dir_fd);
		btrfs_print_vol_args_v2(&vol_args_v2, 1);
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_SUBVOL_CREATE_V2,
		      &args_passed);
		printf(" => {transid=%" PRI__u64"}) = 0\n",
			args_passed.transid);

		subvolfd = openat(btrfs_test_dir_fd, subvol_name,
				  O_RDONLY|O_DIRECTORY);
		if (subvolfd < 0)
			perror_msg_and_fail("openat(%s) failed", subvol_name);

		strncpy(vol_args_v2.name, long_subvol_name, BTRFS_NAME_LEN);
		vol_args_v2.fd = subvolfd;
		args_passed = vol_args_v2;
		printf("ioctl(%d, BTRFS_IOC_SNAP_CREATE_V2, ",
		       btrfs_test_dir_fd);
		btrfs_print_vol_args_v2(&args_passed, 1);
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_SNAP_CREATE_V2,
		      &args_passed);
		printf(" => {transid=%" PRI__u64"}) = 0\n",
			args_passed.transid);

		/* This only works when mounted w/ -ouser_subvol_rm_allowed */
		strncpy(vol_args.name, long_subvol_name, 255);
		vol_args.name[255] = 0;
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_SNAP_DESTROY, &vol_args);
		printf("ioctl(%d, BTRFS_IOC_SNAP_DESTROY, "
		       "{fd=%d, name=\"%.*s\"}) = 0\n",
		       btrfs_test_dir_fd, (int) vol_args.fd, 255, long_subvol_name);

		strcpy(vol_args.name, subvol_name);
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_SNAP_DESTROY, &vol_args);
		printf("ioctl(%d, BTRFS_IOC_SNAP_DESTROY, "
		       "{fd=%d, name=\"%s\"}) = 0\n",
		       btrfs_test_dir_fd, (int) vol_args.fd, subvol_name);

		close(subvolfd);
	}
	free(long_subvol_name);
}

static void
btrfs_print_balance_args(struct btrfs_balance_args *args)
{
	printf("{profiles=");
	printflags(btrfs_space_info_flags, args->profiles,
		   "BTRFS_BLOCK_GROUP_???");
	printf(", usage=%"PRI__u64 "%s, devid=%"PRI__u64 "%s, pstart=%"PRI__u64
	       "%s, pend=%"PRI__u64 "%s, vstart=%"PRI__u64 "%s, vend=%"PRI__u64
	       "%s, target=%"PRI__u64 "%s, flags=",
		args->usage, maybe_print_uint64max(args->usage),
		args->devid, maybe_print_uint64max(args->devid),
		args->pstart, maybe_print_uint64max(args->pstart),
		args->pend, maybe_print_uint64max(args->pend),
		args->vstart, maybe_print_uint64max(args->vstart),
		args->vend, maybe_print_uint64max(args->vend),
		args->target, maybe_print_uint64max(args->target));
	printflags(btrfs_balance_args, args->flags, "BTRFS_BALANCE_ARGS_???");
	printf("}");
}

/*
 * Accepts argument, returns nothing
 * - BTRFS_IOC_BALANCE
 * - BTRFS_IOC_BALANCE_CTL
 *
 * Accepts argument, returns argument
 * - BTRFS_IOC_BALANCE_V2
 */
static void
btrfs_test_balance_ioctls(void)
{
	struct btrfs_ioctl_balance_args args = {
		.flags = 0x3f,
		.data = {
			.profiles = 0x7,
			.flags = 0x7,
			.devid = 1,
			.pend = -1ULL,
			.vend = -1ULL,
		},

		.meta = {
			.profiles = 0x38,
			.flags = 0x38,
			.devid = 1,
		},

		.sys = {
			.profiles = 0x1c0 | (1ULL << 48),
			.flags = 0x4c0,
			.devid = 1,
		},
	};
	struct btrfs_ioctl_vol_args vol_args = {};

	ioctl(-1, BTRFS_IOC_BALANCE_CTL, 1);
	printf("ioctl(-1, BTRFS_IOC_BALANCE_CTL, "
	       "BTRFS_BALANCE_CTL_PAUSE) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_BALANCE_CTL, 2);
	printf("ioctl(-1, BTRFS_IOC_BALANCE_CTL, "
	       "BTRFS_BALANCE_CTL_CANCEL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_BALANCE, NULL);
	printf("ioctl(-1, BTRFS_IOC_BALANCE) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_BALANCE, &vol_args);
	printf("ioctl(-1, BTRFS_IOC_BALANCE) = -1 EBADF (%m)\n");

	/* struct btrfs_ioctl_balance_args */
	ioctl(-1, BTRFS_IOC_BALANCE_V2, NULL);
	printf("ioctl(-1, BTRFS_IOC_BALANCE_V2, NULL) = -1 EBADF (%m)\n");

	printf("ioctl(-1, BTRFS_IOC_BALANCE_V2, {flags=");
	printflags(btrfs_balance_flags, args.flags, "BTRFS_BALANCE_???");
	printf(", data=");
	btrfs_print_balance_args(&args.data);
	printf(", meta=");
	btrfs_print_balance_args(&args.meta);
	printf(", sys=");
	btrfs_print_balance_args(&args.sys);
	ioctl(-1, BTRFS_IOC_BALANCE_V2, &args);
	printf("}) = -1 EBADF (%m)\n");

	if (write_ok) {
		args.flags = BTRFS_BALANCE_DATA | BTRFS_BALANCE_METADATA |
			     BTRFS_BALANCE_SYSTEM;
		args.data.flags = 0;
		args.data.profiles = 0;
		args.meta.flags = 0;
		args.meta.profiles = 0;
		args.sys.flags = 0;
		args.sys.profiles = 0;
		printf("ioctl(%d, BTRFS_IOC_BALANCE_V2, {flags=",
			btrfs_test_dir_fd);

		printflags(btrfs_balance_flags, args.flags,
			   "BTRFS_BALANCE_???");
		printf(", data=");
		btrfs_print_balance_args(&args.data);
		printf(", meta=");
		btrfs_print_balance_args(&args.meta);
		printf(", sys=");
		btrfs_print_balance_args(&args.sys);
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_BALANCE_V2,  &args);
		printf("} => {flags=");
		printflags(btrfs_balance_flags, args.flags,
			   "BTRFS_BALANCE_???");
		printf(", state=");
		printflags(btrfs_balance_state, args.state,
			   "BTRFS_BALANCE_STATE_???");
		printf(", data=");
		btrfs_print_balance_args(&args.data);
		printf(", meta=");
		btrfs_print_balance_args(&args.meta);
		printf(", sys=");
		btrfs_print_balance_args(&args.sys);
		printf("}) = 0\n");
	}
}

/*
 * Consumes argument, returns nothing:
 * - BTRFS_IOC_RESIZE
 *
 * Requires /dev/btrfs-control, consumes argument, returns nothing:
 * - BTRFS_IOC_SCAN_DEV
 * - BTRFS_IOC_DEVICES_READY
 *
 */
static void
btrfs_test_device_ioctls(void)
{
	const char *devid = "1";
	const char *devname = "/dev/sda1";
	struct btrfs_ioctl_vol_args args = {
		.fd = 2,
	};

	ioctl(-1, BTRFS_IOC_RESIZE, NULL);
	printf("ioctl(-1, BTRFS_IOC_RESIZE, NULL) = -1 EBADF (%m)\n");

	strcpy(args.name, devid);
	ioctl(-1, BTRFS_IOC_RESIZE, &args);
	printf("ioctl(-1, BTRFS_IOC_RESIZE, "
	       "{fd=%d, name=\"%s\"}) = -1 EBADF (%m)\n",
	       (int) args.fd, args.name);

	ioctl(-1, BTRFS_IOC_SCAN_DEV, NULL);
	printf("ioctl(-1, BTRFS_IOC_SCAN_DEV, NULL) = -1 EBADF (%m)\n");

	strcpy(args.name, devname);
	ioctl(-1, BTRFS_IOC_SCAN_DEV, &args);
	printf("ioctl(-1, BTRFS_IOC_SCAN_DEV, "
	       "{fd=%d, name=\"%s\"}) = -1 EBADF (%m)\n",
	       (int) args.fd, args.name);

	ioctl(-1, BTRFS_IOC_ADD_DEV, NULL);
	printf("ioctl(-1, BTRFS_IOC_ADD_DEV, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_ADD_DEV, &args);
	printf("ioctl(-1, BTRFS_IOC_ADD_DEV, "
	       "{fd=%d, name=\"%s\"}) = -1 EBADF (%m)\n",
	       (int) args.fd, args.name);

	ioctl(-1, BTRFS_IOC_RM_DEV, NULL);
	printf("ioctl(-1, BTRFS_IOC_RM_DEV, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_RM_DEV, &args);
	printf("ioctl(-1, BTRFS_IOC_RM_DEV, "
	       "{fd=%d, name=\"%s\"}) = -1 EBADF (%m)\n",
	       (int) args.fd, args.name);

}

/*
 * Consumes argument, returns nothing:
 * - BTRFS_IOC_CLONE
 * - BTRFS_IOC_CLONE_RANGE
 */
static void
btrfs_test_clone_ioctls(void)
{
	int clone_fd = 4;
	struct btrfs_ioctl_clone_range_args args = {
		.src_fd = clone_fd,
		.src_offset = 4096,
		.src_length = 16384,
		.dest_offset = 128 * 1024,
	};

	ioctl(-1, BTRFS_IOC_CLONE, clone_fd);
	printf("ioctl(-1, BTRFS_IOC_CLONE or FICLONE, %x) = -1 EBADF (%m)\n",
		clone_fd);

	ioctl(-1, BTRFS_IOC_CLONE_RANGE, NULL);
	printf("ioctl(-1, BTRFS_IOC_CLONE_RANGE or FICLONERANGE, "
	       "NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_CLONE_RANGE, &args);
	printf("ioctl(-1, BTRFS_IOC_CLONE_RANGE or FICLONERANGE, "
	       "{src_fd=%d, src_offset=%" PRI__u64 ", src_length=%" PRI__u64
	       ", dest_offset=%" PRI__u64 "}) = -1 EBADF (%m)\n",
		(int) args.src_fd, args.src_offset, args.src_length,
		args.dest_offset);
}

#define BTRFS_COMPRESS_TYPES 2
#define BTRFS_INVALID_COMPRESS (BTRFS_COMPRESS_TYPES + 1)

static void
btrfs_print_defrag_range_args(struct btrfs_ioctl_defrag_range_args *args)
{
	printf("{start=%" PRIu64", len=%" PRIu64 "%s, flags=",
		(uint64_t) args->start, (uint64_t) args->len,
		maybe_print_uint64max(args->len));

	printflags(btrfs_defrag_flags, args->flags, "BTRFS_DEFRAG_RANGE_???");
	printf(", extent_thresh=%u, compress_type=", args->extent_thresh);
	printxval(btrfs_compress_types, args->compress_type,
		  "BTRFS_COMPRESS_???");
	printf("}");
}

/*
 * Consumes argument, returns nothing:
 * - BTRFS_IOC_DEFRAG
 * - BTRFS_DEFRAG_RANGE
 */
static void
btrfs_test_defrag_ioctls(void)
{
	struct btrfs_ioctl_vol_args vol_args = {};
	struct btrfs_ioctl_defrag_range_args args = {
		.start = 0,
		.len = -1ULL,
		.flags = max_flags_plus_one(1),
		.extent_thresh = 128 * 1024,
		.compress_type = 2, /* BTRFS_COMPRESS_LZO */
	};

	/*
	 * These are documented as using vol_args but don't
	 * actually consume it.
	 */
	ioctl(-1, BTRFS_IOC_DEFRAG, NULL);
	printf("ioctl(-1, BTRFS_IOC_DEFRAG) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_DEFRAG, &vol_args);
	printf("ioctl(-1, BTRFS_IOC_DEFRAG) = -1 EBADF (%m)\n");

	/* struct btrfs_ioctl_defrag_range_args */
	ioctl(-1, BTRFS_IOC_DEFRAG_RANGE, NULL);
	printf("ioctl(-1, BTRFS_IOC_DEFRAG_RANGE, NULL) = -1 EBADF (%m)\n");

	printf("ioctl(-1, BTRFS_IOC_DEFRAG_RANGE, ");
	btrfs_print_defrag_range_args(&args);
	ioctl(-1, BTRFS_IOC_DEFRAG_RANGE, &args);
	printf(") = -1 EBADF (%m)\n");

	args.compress_type = BTRFS_INVALID_COMPRESS;
	printf("ioctl(-1, BTRFS_IOC_DEFRAG_RANGE, ");
	btrfs_print_defrag_range_args(&args);
	ioctl(-1, BTRFS_IOC_DEFRAG_RANGE, &args);
	printf(") = -1 EBADF (%m)\n");

	args.len--;
	printf("ioctl(-1, BTRFS_IOC_DEFRAG_RANGE, ");
	btrfs_print_defrag_range_args(&args);
	ioctl(-1, BTRFS_IOC_DEFRAG_RANGE, &args);
	printf(") = -1 EBADF (%m)\n");
}

static const char *
xlookup(const struct xlat *xlat, const uint64_t val)
{
	for (; xlat->str != NULL; xlat++)
		if (xlat->val == val)
			return xlat->str;
	return NULL;
}

static void
btrfs_print_objectid(uint64_t objectid)
{
	const char *str = xlookup(btrfs_tree_objectids, objectid);
	printf("%" PRIu64, objectid);
	if (str)
		printf(" /* %s */", str);
}

static void
btrfs_print_key_type(uint32_t type)
{
	const char *str = xlookup(btrfs_key_types, type);
	printf("%u", type);
	if (str)
		printf(" /* %s */", str);
}

static void
btrfs_print_search_key(struct btrfs_ioctl_search_key *key)
{
	printf("key={tree_id=");
	btrfs_print_objectid(key->tree_id);
	if (verbose || key->min_objectid != 256) {
		printf(", min_objectid=");
		btrfs_print_objectid(key->min_objectid);
	}
	if (verbose || key->max_objectid != -256ULL) {
		printf(", max_objectid=");
		btrfs_print_objectid(key->max_objectid);
	}
	if (key->min_offset)
		printf(", min_offset=%" PRI__u64 "%s",
		       key->min_offset, maybe_print_uint64max(key->min_offset));
	if (key->max_offset)
		printf(", max_offset=%" PRI__u64 "%s",
		       key->max_offset, maybe_print_uint64max(key->max_offset));
	if (key->min_transid)
		printf(", min_transid=%" PRI__u64"%s", key->min_transid,
		       maybe_print_uint64max(key->min_transid));
	if (key->max_transid)
		printf(", max_transid=%" PRI__u64"%s", key->max_transid,
		       maybe_print_uint64max(key->max_transid));
	printf(", min_type=");
	btrfs_print_key_type(key->min_type);
	printf(", max_type=");
	btrfs_print_key_type(key->max_type);
	printf(", nr_items=%u}", key->nr_items);
}

static void
btrfs_print_tree_search_buf(struct btrfs_ioctl_search_key *key,
			    void *buf, uint64_t buf_size)
{
	if (verbose) {
		uint64_t i;
		uint64_t off = 0;
		printf("[");
		for (i = 0; i < key->nr_items; i++) {
			struct btrfs_ioctl_search_header *sh;
			sh = (typeof(sh))(buf + off);
			if (i)
				printf(", ");
			printf("{transid=%" PRI__u64 ", objectid=",
				sh->transid);
			btrfs_print_objectid(sh->objectid);
			printf(", offset=%" PRI__u64 ", type=", sh->offset);
			btrfs_print_key_type(sh->type);
			printf(", len=%u}", sh->len);
			off += sizeof(*sh) + sh->len;
		}
		printf("]");
	} else
		printf("...");
}

/*
 * Consumes argument, returns argument:
 * - BTRFS_IOC_TREE_SEARCH
 * - BTRFS_IOC_TREE_SEARCH_V2
 */
static void
btrfs_test_search_ioctls(void)
{
	struct btrfs_ioctl_search_key key_reference = {
		.tree_id = 5,
		.min_objectid = 256,
		.max_objectid = -1ULL,
		.min_offset = 0,
		.max_offset = -1ULL,
		.min_transid = 0,
		.max_transid = -1ULL,
		.min_type = 0,
		.max_type = -1U,
		.nr_items = 10,
	};
	struct btrfs_ioctl_search_args search_args;
	struct btrfs_ioctl_search_args_v2 search_args_v2 = {
		.buf_size = 4096,
	};

	ioctl(-1, BTRFS_IOC_TREE_SEARCH, NULL);
	printf("ioctl(-1, BTRFS_IOC_TREE_SEARCH, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_TREE_SEARCH_V2, NULL);
	printf("ioctl(-1, BTRFS_IOC_TREE_SEARCH_V2, NULL) = -1 EBADF (%m)\n");

	search_args.key = key_reference;
	printf("ioctl(-1, BTRFS_IOC_TREE_SEARCH, {");
	btrfs_print_search_key(&search_args.key);
	ioctl(-1, BTRFS_IOC_TREE_SEARCH, &search_args);
	printf("}) = -1 EBADF (%m)\n");

	search_args_v2.key = key_reference;
	printf("ioctl(-1, BTRFS_IOC_TREE_SEARCH_V2, {");
	btrfs_print_search_key(&search_args_v2.key);
	ioctl(-1, BTRFS_IOC_TREE_SEARCH_V2, &search_args_v2);
	printf(", buf_size=%" PRIu64 "}) = -1 EBADF (%m)\n",
	       (uint64_t)search_args_v2.buf_size);

	key_reference.min_objectid = 6;
	key_reference.max_objectid = 7;
	search_args.key = key_reference;
	printf("ioctl(-1, BTRFS_IOC_TREE_SEARCH, {");
	btrfs_print_search_key(&search_args.key);
	ioctl(-1, BTRFS_IOC_TREE_SEARCH, &search_args);
	printf("}) = -1 EBADF (%m)\n");

	search_args_v2.key = key_reference;
	printf("ioctl(-1, BTRFS_IOC_TREE_SEARCH_V2, {");
	btrfs_print_search_key(&search_args_v2.key);
	ioctl(-1, BTRFS_IOC_TREE_SEARCH_V2, &search_args_v2);
	printf(", buf_size=%" PRIu64 "}) = -1 EBADF (%m)\n",
	       (uint64_t)search_args_v2.buf_size);

	key_reference.min_offset++;
	key_reference.max_offset--;
	search_args.key = key_reference;
	printf("ioctl(-1, BTRFS_IOC_TREE_SEARCH, {");
	btrfs_print_search_key(&search_args.key);
	ioctl(-1, BTRFS_IOC_TREE_SEARCH, &search_args);
	printf("}) = -1 EBADF (%m)\n");

	search_args_v2.key = key_reference;
	printf("ioctl(-1, BTRFS_IOC_TREE_SEARCH_V2, {");
	btrfs_print_search_key(&search_args_v2.key);
	ioctl(-1, BTRFS_IOC_TREE_SEARCH_V2, &search_args_v2);
	printf(", buf_size=%" PRIu64 "}) = -1 EBADF (%m)\n",
	       (uint64_t)search_args_v2.buf_size);

	key_reference.min_transid++;
	key_reference.max_transid--;
	search_args.key = key_reference;
	printf("ioctl(-1, BTRFS_IOC_TREE_SEARCH, {");
	btrfs_print_search_key(&search_args.key);
	ioctl(-1, BTRFS_IOC_TREE_SEARCH, &search_args);
	printf("}) = -1 EBADF (%m)\n");

	search_args_v2.key = key_reference;
	printf("ioctl(-1, BTRFS_IOC_TREE_SEARCH_V2, {");
	btrfs_print_search_key(&search_args_v2.key);
	ioctl(-1, BTRFS_IOC_TREE_SEARCH_V2, &search_args_v2);
	printf(", buf_size=%" PRIu64 "}) = -1 EBADF (%m)\n",
	       (uint64_t)search_args_v2.buf_size);

	key_reference.min_type = 1;
	key_reference.max_type = 12;
	search_args.key = key_reference;
	printf("ioctl(-1, BTRFS_IOC_TREE_SEARCH, {");
	btrfs_print_search_key(&search_args.key);
	ioctl(-1, BTRFS_IOC_TREE_SEARCH, &search_args);
	printf("}) = -1 EBADF (%m)\n");

	search_args_v2.key = key_reference;
	printf("ioctl(-1, BTRFS_IOC_TREE_SEARCH_V2, {");
	btrfs_print_search_key(&search_args_v2.key);
	ioctl(-1, BTRFS_IOC_TREE_SEARCH_V2, &search_args_v2);
	printf(", buf_size=%" PRIu64 "}) = -1 EBADF (%m)\n",
	       (uint64_t)search_args_v2.buf_size);

	if (btrfs_test_root) {
		struct btrfs_ioctl_search_args_v2 *args;
		int bufsize = 4096;

		key_reference.tree_id = 5;
		key_reference.min_type = 1;
		key_reference.max_type = 1;
		key_reference.min_objectid = 256;
		key_reference.max_objectid = 357;
		key_reference.min_offset = 0;
		key_reference.max_offset = -1ULL;

		search_args.key = key_reference;
		printf("ioctl(%d, BTRFS_IOC_TREE_SEARCH, {",
			btrfs_test_dir_fd);
		btrfs_print_search_key(&search_args.key);
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_TREE_SEARCH, &search_args);
		printf("} => {key={nr_items=%u}, buf=",
			search_args.key.nr_items);
		btrfs_print_tree_search_buf(&search_args.key, search_args.buf,
					    sizeof(search_args.buf));
		printf("}) = 0\n");

		args = malloc(sizeof(*args) + bufsize);
		if (!args)
			perror_msg_and_fail("malloc failed");

		args->key = key_reference;
		args->buf_size = bufsize;
		printf("ioctl(%d, BTRFS_IOC_TREE_SEARCH_V2, {",
			btrfs_test_dir_fd);
		btrfs_print_search_key(&key_reference);
		printf(", buf_size=%" PRIu64 "}", (uint64_t) args->buf_size);
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_TREE_SEARCH_V2, args);
		printf(" => {key={nr_items=%u}, buf_size=%" PRIu64 ", buf=",
			args->key.nr_items, (uint64_t)args->buf_size);
		btrfs_print_tree_search_buf(&args->key, args->buf,
					    args->buf_size);
		printf("}) = 0\n");

		args->key = key_reference;
		args->buf_size = sizeof(struct btrfs_ioctl_search_header);
		printf("ioctl(%d, BTRFS_IOC_TREE_SEARCH_V2, {",
			btrfs_test_dir_fd);
		btrfs_print_search_key(&args->key);
		printf(", buf_size=%" PRIu64 "}", (uint64_t)args->buf_size);
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_TREE_SEARCH_V2, args);
		printf(" => {buf_size=%" PRIu64 "}) = -1 EOVERFLOW (%m)\n",
			(uint64_t)args->buf_size);
		free(args);
	}
}

/*
 * Consumes argument, returns argument:
 * - BTRFS_IOC_INO_LOOKUP
 */
static void
btrfs_test_ino_lookup_ioctl(void)
{
	struct btrfs_ioctl_ino_lookup_args args = {
		.treeid = 5,
		.objectid = 256,
	};

	ioctl(-1, BTRFS_IOC_INO_LOOKUP, NULL);
	printf("ioctl(-1, BTRFS_IOC_INO_LOOKUP, NULL) = -1 EBADF (%m)\n");

	printf("ioctl(-1, BTRFS_IOC_INO_LOOKUP, {treeid=");
	btrfs_print_objectid(args.treeid);
	printf(", objectid=");
	btrfs_print_objectid(args.objectid);
	ioctl(-1, BTRFS_IOC_INO_LOOKUP, &args);
	printf("}) = -1 EBADF (%m)\n");

	if (btrfs_test_root) {
		printf("ioctl(%d, BTRFS_IOC_INO_LOOKUP, {treeid=",
		       btrfs_test_dir_fd);
		btrfs_print_objectid(args.treeid);
		printf(", objectid=");
		btrfs_print_objectid(args.objectid);
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_INO_LOOKUP, &args);
		printf("} => {name=\"%s\"}) = 0\n", args.name);
	}
}

/*
 * Consumes argument, returns argument:
 * - BTRFS_IOC_SPACE_INFO
 */
static void
btrfs_test_space_info_ioctl(void)
{
	struct btrfs_ioctl_space_args args = {};

	ioctl(-1, BTRFS_IOC_SPACE_INFO, NULL);
	printf("ioctl(-1, BTRFS_IOC_SPACE_INFO, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_SPACE_INFO, &args);
	printf("ioctl(-1, BTRFS_IOC_SPACE_INFO, "
	       "{space_slots=%" PRI__u64 "}) = -1 EBADF (%m)\n",
		args.space_slots);

	if (btrfs_test_root) {
		struct btrfs_ioctl_space_args args_passed;
		struct btrfs_ioctl_space_args *argsp;
		args_passed = args;
		printf("ioctl(%d, BTRFS_IOC_SPACE_INFO, "
		       "{space_slots=%" PRI__u64 "}",
		       btrfs_test_dir_fd, args_passed.space_slots);
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_SPACE_INFO, &args_passed);
		printf(" => {total_spaces=%" PRI__u64 "}) = 0\n",
			args_passed.total_spaces);

		argsp = malloc(sizeof(args) +
			args_passed.total_spaces * sizeof(args.spaces[0]));
		if (!argsp)
			perror_msg_and_fail("malloc failed");

		*argsp = args;
		argsp->space_slots = args_passed.total_spaces;
		printf("ioctl(%d, BTRFS_IOC_SPACE_INFO, "
		       "{space_slots=%" PRI__u64 "}",
		       btrfs_test_dir_fd, argsp->space_slots);
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_SPACE_INFO, argsp);
		printf(" => {total_spaces=%" PRI__u64 ", spaces=",
			argsp->total_spaces);
		if (verbose) {
			unsigned int i;
			printf("[");
			for (i = 0; i < argsp->total_spaces; i++) {
				struct btrfs_ioctl_space_info *info;
				info = &argsp->spaces[i];
				if (i)
					printf(", ");
				printf("{flags=");
				printflags(btrfs_space_info_flags, info->flags,
					   "BTRFS_SPACE_INFO_???");
				printf(", total_bytes=%" PRI__u64
				       ", used_bytes=%" PRI__u64 "}",
				       info->total_bytes, info->used_bytes);
			}

			printf("]");
		} else
			printf("...");
		printf("}) = 0\n");
		free(argsp);
	}
}

/*
 * Consumes no arguments, returns nothing:
 * - BTRFS_IOC_SCRUB_CANCEL
 * Consumes argument, returns argument:
 - * BTRFS_IOC_SCRUB
 - * BTRFS_IOC_SCRUB_PROGRESS
 */
static void
btrfs_test_scrub_ioctls(void)
{
	struct btrfs_ioctl_scrub_args args = {
		.devid = 1,
		.start = 0,
		.end = -1ULL,
		.flags = max_flags_plus_one(0),
	};

	ioctl(-1, BTRFS_IOC_SCRUB, NULL);
	printf("ioctl(-1, BTRFS_IOC_SCRUB, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_SCRUB_CANCEL, NULL);
	printf("ioctl(-1, BTRFS_IOC_SCRUB_CANCEL) = -1 EBADF (%m)\n");

	printf("ioctl(-1, BTRFS_IOC_SCRUB, {devid=%" PRI__u64 ", start=%"
		PRI__u64 "%s, end=%" PRI__u64"%s, flags=",
		args.devid, args.start, maybe_print_uint64max(args.start),
		args.end, maybe_print_uint64max(args.end));
	printflags(btrfs_scrub_flags, args.flags, "BTRFS_SCRUB_???");
	ioctl(-1, BTRFS_IOC_SCRUB, &args);
	printf("}) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_SCRUB_PROGRESS, NULL);
	printf("ioctl(-1, BTRFS_IOC_SCRUB_PROGRESS, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_SCRUB_PROGRESS, &args);
	printf("ioctl(-1, BTRFS_IOC_SCRUB_PROGRESS, "
	       "{devid=%" PRI__u64 "}) = -1 EBADF (%m)\n", args.devid);
}

/*
 * Consumes argument, returns argument:
 * - BTRFS_IOC_DEV_INFO
 */
static void
btrfs_test_dev_info_ioctl(void)
{
	struct btrfs_ioctl_dev_info_args args = {
		.devid = 1,
	};
	memcpy(&args.uuid, uuid_reference, BTRFS_UUID_SIZE);

	ioctl(-1, BTRFS_IOC_DEV_INFO, NULL);
	printf("ioctl(-1, BTRFS_IOC_DEV_INFO, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_DEV_INFO, &args);
	printf("ioctl(-1, BTRFS_IOC_DEV_INFO, "
	       "{devid=%" PRI__u64", uuid=%s}) = -1 EBADF (%m)\n",
		args.devid, uuid_reference_string);
}

/*
 * Consumes argument, returns argument:
 * - BTRFS_IOC_INO_PATHS
 * - BTRFS_IOC_LOGICAL_INO
 */
static void
btrfs_test_ino_path_ioctls(void)
{
	char buf[16384];
	struct btrfs_ioctl_ino_path_args args = {
		.inum = 256,
		.size = sizeof(buf),
		.fspath = (unsigned long)buf,
	};

	ioctl(-1, BTRFS_IOC_INO_PATHS, NULL);
	printf("ioctl(-1, BTRFS_IOC_INO_PATHS, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_LOGICAL_INO, NULL);
	printf("ioctl(-1, BTRFS_IOC_LOGICAL_INO, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_INO_PATHS, &args);
	printf("ioctl(-1, BTRFS_IOC_INO_PATHS, "
	       "{inum=%" PRI__u64", size=%" PRI__u64
	       ", fspath=0x%" PRI__x64 "}) = -1 EBADF (%m)\n",
	       args.inum, args.size, args.fspath);

	ioctl(-1, BTRFS_IOC_LOGICAL_INO, &args);
	printf("ioctl(-1, BTRFS_IOC_LOGICAL_INO, {logical=%" PRI__u64
	       ", size=%" PRI__u64", inodes=0x%" PRI__x64
	       "}) = -1 EBADF (%m)\n", args.inum, args.size, args.fspath);

#ifdef HAVE_LINUX_FIEMAP_H
	if (btrfs_test_root) {
		int size;
		struct stat si;
		int ret;
		struct btrfs_data_container *data = (void *)buf;
		struct fiemap *fiemap;
		int fd;

		ret = fstat(btrfs_test_dir_fd, &si);
		if (ret)
			perror_msg_and_fail("fstat failed");

		args.inum = si.st_ino;
		printf("ioctl(%d, BTRFS_IOC_INO_PATHS, "
		       "{inum=%" PRI__u64", size=%" PRI__u64
		       ", fspath=0x%" PRI__x64"}",
		       btrfs_test_dir_fd, args.inum, args.size,
		       args.fspath);
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_INO_PATHS, &args);
		printf(" => {fspath={bytes_left=%u, bytes_missing=%u, elem_cnt=%u, elem_missed=%u, val=",
			data->bytes_left, data->bytes_missing, data->elem_cnt,
			data->elem_missed);
		if (verbose) {
			printf("[\"strace-test\"]");
		} else
			printf("...");
		printf("}}) = 0\n");

		fd = openat(btrfs_test_dir_fd, "file1", O_RDWR|O_CREAT, 0600);
		if (fd < 0)
			perror_msg_and_fail("openat(file1) failed");

		ret = fstat(fd, &si);
		if (ret)
			perror_msg_and_fail("fstat failed");

		if (write(fd, buf, sizeof(buf)) < 0)
			perror_msg_and_fail("write: fd");

		/*
		 * Force delalloc so we can actually
		 * search for the extent.
		 */
		fsync(fd);
		ioctl(fd, BTRFS_IOC_SYNC, NULL);
		printf("ioctl(%d, BTRFS_IOC_SYNC) = 0\n", fd);

		size = sizeof(*fiemap) + 2 * sizeof(fiemap->fm_extents[0]);
		fiemap = malloc(size);
		if (!fiemap)
			perror_msg_and_fail("malloc failed");
		memset(fiemap, 0, size);

		fiemap->fm_length = sizeof(buf);
		fiemap->fm_extent_count = 2;

		/* This is also a live test for FIEMAP */
		printf("ioctl(%d, FS_IOC_FIEMAP, {fm_start=%" PRI__u64
		       ", fm_length=%" PRI__u64", fm_flags=",
		       fd, fiemap->fm_start, fiemap->fm_length);
		printflags(fiemap_flags, fiemap->fm_flags, "FIEMAP_FLAG_???");
		printf(", fm_extent_count=%u}", fiemap->fm_extent_count);
		ioctl(fd, FS_IOC_FIEMAP, fiemap);
		printf(" => {fm_flags=");
		printflags(fiemap_flags, fiemap->fm_flags, "FIEMAP_FLAG_???");
		printf(", fm_mapped_extents=%u, fm_extents=",
			fiemap->fm_mapped_extents);
		if (verbose) {
			printf("[");
			unsigned int i;
			for (i = 0; i < fiemap->fm_mapped_extents; i++) {
				struct fiemap_extent *fe;
				fe = &fiemap->fm_extents[i];
				if (i)
					printf(", ");
				printf("{fe_logical=%" PRI__u64
				       ", fe_physical=%" PRI__u64
				       ", fe_length=%" PRI__u64
				       ", ",
				       fe->fe_logical, fe->fe_physical,
				       fe->fe_length);
				printflags(fiemap_extent_flags, fe->fe_flags,
					   "FIEMAP_EXTENT_???");
				printf("}");
			}
			printf("]");
		} else
			printf("...");
		printf("}) = 0\n");

		args.inum = fiemap->fm_extents[0].fe_physical;
		printf("ioctl(%d, BTRFS_IOC_LOGICAL_INO, {logical=%" PRI__u64
		       ", size=%" PRI__u64", inodes=0x%" PRI__x64"}",
		       fd, args.inum, args.size, args.fspath);
		ioctl(fd, BTRFS_IOC_LOGICAL_INO, &args);
		printf(" => {inodes={bytes_left=%u, bytes_missing=%u, elem_cnt=%u, elem_missed=%u, val=",
			data->bytes_left, data->bytes_missing, data->elem_cnt,
			data->elem_missed);
		if (verbose) {
			printf("[{inum=%llu, offset=0, root=5}]",
			       (unsigned long long) si.st_ino);
		} else
			printf("...");
		printf("}}) = 0\n");
		close(fd);
		free(fiemap);
	}
#endif /* HAVE_LINUX_FIEMAP_H */
}

/*
 * Consumes argument, returns argument:
 * - BTRFS_IOC_SET_RECEIVED_SUBVOL
 */
static void
btrfs_test_set_received_subvol_ioctl(void)
{
	struct btrfs_ioctl_received_subvol_args args = {
		.stransid = 0x12345,
		.stime = {
			.sec = 1463193386,
			.nsec = 12345,
		},
	};
	memcpy(&args.uuid, uuid_reference, BTRFS_UUID_SIZE);

	ioctl(-1, BTRFS_IOC_SET_RECEIVED_SUBVOL, NULL);
	printf("ioctl(-1, BTRFS_IOC_SET_RECEIVED_SUBVOL, "
	       "NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_SET_RECEIVED_SUBVOL, &args);
	printf("ioctl(-1, BTRFS_IOC_SET_RECEIVED_SUBVOL, "
	       "{uuid=%s, stransid=%" PRI__u64", stime=%" PRI__u64
	       ".%u, flags=0}) = -1 EBADF (%m)\n",
	       uuid_reference_string, args.stransid, args.stime.sec,
	       args.stime.nsec);
}

/*
 * Consumes argument, returns nothing (output is via send_fd)
 * - BTRFS_IOC_SEND
 */
static void
btrfs_test_send_ioctl(void)
{
	uint64_t u64_array[2] = { 256, 257 };
	struct btrfs_ioctl_send_args args = {
		.send_fd = 4,
		.parent_root = 257,
		.flags = max_flags_plus_one(2),
	};

	ioctl(-1, BTRFS_IOC_SEND, NULL);
	printf("ioctl(-1, BTRFS_IOC_SEND, NULL) = -1 EBADF (%m)\n");

	printf("ioctl(-1, BTRFS_IOC_SEND, "
	       "{send_fd=%d, clone_sources_count=%" PRI__u64
	       ", clone_sources=",
	       (int) args.send_fd, args.clone_sources_count);
	if (verbose)
		printf("NULL");
	else
		printf("...");
	printf(", parent_root=");
	btrfs_print_objectid(args.parent_root);
	printf(", flags=");
	printflags(btrfs_send_flags, args.flags, "BTRFS_SEND_FLAGS_???");
	ioctl(-1, BTRFS_IOC_SEND, &args);
	printf("}) = -1 EBADF (%m)\n");

	args.clone_sources_count = 2;
	args.clone_sources = (__u64 *) (void *) u64_array;

	printf("ioctl(-1, BTRFS_IOC_SEND, "
	       "{send_fd=%d, clone_sources_count=%" PRI__u64
	       ", clone_sources=",
	       (int) args.send_fd, args.clone_sources_count);
	if (verbose) {
		printf("[");
		btrfs_print_objectid(u64_array[0]);
		printf(", ");
		btrfs_print_objectid(u64_array[1]);
		printf("]");
	} else
		printf("...");
	printf(", parent_root=");
	btrfs_print_objectid(args.parent_root);
	printf(", flags=");
	printflags(btrfs_send_flags, args.flags, "BTRFS_SEND_FLAGS_???");
	ioctl(-1, BTRFS_IOC_SEND, &args);
	printf("}) = -1 EBADF (%m)\n");
}

/*
 * Consumes argument, returns nothing:
 * - BTRFS_IOC_QUOTA_CTL
 */
static void
btrfs_test_quota_ctl_ioctl(void)
{
	struct btrfs_ioctl_quota_ctl_args args = {
		.cmd = 1,
	};

	ioctl(-1, BTRFS_IOC_QUOTA_CTL, NULL);
	printf("ioctl(-1, BTRFS_IOC_QUOTA_CTL, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_QUOTA_CTL, &args);
	printf("ioctl(-1, BTRFS_IOC_QUOTA_CTL, "
	       "BTRFS_QUOTA_CTL_ENABLE}) = -1 EBADF (%m)\n");

	args.cmd = 2;
	ioctl(-1, BTRFS_IOC_QUOTA_CTL, &args);
	printf("ioctl(-1, BTRFS_IOC_QUOTA_CTL, "
	       "BTRFS_QUOTA_CTL_DISABLE}) = -1 EBADF (%m)\n");

	args.cmd = 3;
	ioctl(-1, BTRFS_IOC_QUOTA_CTL, &args);
	printf("ioctl(-1, BTRFS_IOC_QUOTA_CTL, "
	       "BTRFS_QUOTA_CTL_RESCAN__NOTUSED}) = -1 EBADF (%m)\n");

	args.cmd = 4;
	ioctl(-1, BTRFS_IOC_QUOTA_CTL, &args);
	printf("ioctl(-1, BTRFS_IOC_QUOTA_CTL, "
	       "0x4 /* BTRFS_QUOTA_CTL_??? */}) = -1 EBADF (%m)\n");
}

/*
 * Consumes argument, returns nothing:
 * - BTRFS_IOC_QGROUP_ASSIGN
 */
static void
btrfs_test_qgroup_assign_ioctl(void)
{
	struct btrfs_ioctl_qgroup_assign_args args = {
		.assign = 1,
		.src = 257,
		.dst = 258,
	};

	ioctl(-1, BTRFS_IOC_QGROUP_ASSIGN, NULL);
	printf("ioctl(-1, BTRFS_IOC_QGROUP_ASSIGN, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_QGROUP_ASSIGN, &args);
	printf("ioctl(-1, BTRFS_IOC_QGROUP_ASSIGN, "
	       "{assign=%" PRI__u64", src=%" PRI__u64", dst=%" PRI__u64
	       "}) = -1 EBADF (%m)\n", args.assign, args.src, args.dst);
}

/*
 * Consumes argument, returns nothing:
 * - BTRFS_IOC_QGROUP_CREATE
  */
static void
btrfs_test_qgroup_create_ioctl(void)
{
	struct btrfs_ioctl_qgroup_create_args args = {
		.create = 1,
		.qgroupid = 257,
	};

	ioctl(-1, BTRFS_IOC_QGROUP_CREATE, NULL);
	printf("ioctl(-1, BTRFS_IOC_QGROUP_CREATE, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_QGROUP_CREATE, &args);
	printf("ioctl(-1, BTRFS_IOC_QGROUP_CREATE, "
	       "{create=%" PRI__u64", qgroupid=%" PRI__u64
	       "}) = -1 EBADF (%m)\n", args.create, args.qgroupid);
}

/*
 * Consumes nothing, returns nothing:
 * - BTRFS_IOC_QUOTA_RESCAN_WAIT
 * Consumes argument, returns nothing:
 * - BTRFS_IOC_QUOTA_RESCAN
 */
static void
btrfs_test_quota_rescan_ioctl(void)
{
	struct btrfs_ioctl_quota_rescan_args args = {
		.progress = 1,
	};

	ioctl(-1, BTRFS_IOC_QUOTA_RESCAN, NULL);
	printf("ioctl(-1, BTRFS_IOC_QUOTA_RESCAN, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_QUOTA_RESCAN, &args);
	printf("ioctl(-1, BTRFS_IOC_QUOTA_RESCAN, "
	       "{flags=0}) = -1 EBADF (%m)\n");
	ioctl(-1, BTRFS_IOC_QUOTA_RESCAN_WAIT, NULL);
	printf("ioctl(-1, BTRFS_IOC_QUOTA_RESCAN_WAIT) = -1 EBADF (%m)\n");

}

/*
 * Consumes argument, returns nothing:
 * - BTRFS_IOC_SET_FSLABEL
 *
 * Consumes no argument, returns argument:
 * - BTRFS_IOC_GET_FS_LABEL
 */
static void
btrfs_test_label_ioctls(void)
{
	char label[BTRFS_LABEL_SIZE] = "btrfs-label";

	ioctl(-1, BTRFS_IOC_SET_FSLABEL, NULL);
	printf("ioctl(-1, BTRFS_IOC_SET_FSLABEL, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_SET_FSLABEL, label);
	printf("ioctl(-1, BTRFS_IOC_SET_FSLABEL, \"%s\") = -1 EBADF (%m)\n",
		label);

	if (write_ok) {
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_SET_FSLABEL, label);
		printf("ioctl(%d, BTRFS_IOC_SET_FSLABEL, \"%s\") = 0\n",
			btrfs_test_dir_fd, label);

		ioctl(btrfs_test_dir_fd, BTRFS_IOC_GET_FSLABEL, label);
		printf("ioctl(%d, BTRFS_IOC_GET_FSLABEL, \"%s\") = 0\n",
			btrfs_test_dir_fd, label);
	}
}

/*
 * Consumes argument, returns argument:
 * - BTRFS_IOC_GET_DEV_STATS
 */
static void
btrfs_test_get_dev_stats_ioctl(void)
{
	struct btrfs_ioctl_get_dev_stats args = {
		.devid = 1,
		.nr_items = 5,
		.flags = max_flags_plus_one(0),
	};

	ioctl(-1, BTRFS_IOC_GET_DEV_STATS, NULL);
	printf("ioctl(-1, BTRFS_IOC_GET_DEV_STATS, NULL) = -1 EBADF (%m)\n");

	printf("ioctl(-1, BTRFS_IOC_GET_DEV_STATS, {devid=%" PRI__u64
		", nr_items=%" PRI__u64", flags=",
		args.devid, args.nr_items);
	printflags(btrfs_dev_stats_flags, args.flags,
		     "BTRFS_DEV_STATS_???");
	ioctl(-1, BTRFS_IOC_GET_DEV_STATS, &args);
	printf("}) = -1 EBADF (%m)\n");

	if (write_ok) {
		unsigned int i;
		args.flags = BTRFS_DEV_STATS_RESET;
		printf("ioctl(%d, BTRFS_IOC_GET_DEV_STATS, {devid=%" PRI__u64
			", nr_items=%" PRI__u64", flags=",
			btrfs_test_dir_fd, args.devid, args.nr_items);
		printflags(btrfs_dev_stats_flags, args.flags,
			     "BTRFS_DEV_STATS_???");
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_GET_DEV_STATS, &args);
		printf("} => {nr_items=%" PRI__u64 ", flags=",
			args.nr_items);
		printflags(btrfs_dev_stats_flags, args.flags,
			   "BTRFS_DEV_STATS_???");
		printf(", [");
		for (i = 0; i < args.nr_items; i++) {
			const char *name = xlookup(btrfs_dev_stats_values, i);
			if (i)
				printf(", ");
			printf("%" PRI__u64, args.values[i]);
			if (name)
				printf(" /* %s */", name);
		}
		printf("]}) = 0\n");
	}
}

/*
 * Consumes argument, returns argument:
 * - BTRFS_IOC_DEV_REPLACE
 *
 * Test environment for this is more difficult; It's better to do it by hand.
 */
static void
btrfs_test_dev_replace_ioctl(void)
{
	struct btrfs_ioctl_dev_replace_args args = {
		.cmd = BTRFS_IOCTL_DEV_REPLACE_CMD_START,
	};
	args.start.srcdevid = 1;
	strcpy((char *)args.start.srcdev_name, "/dev/sda1");
	strcpy((char *)args.start.tgtdev_name, "/dev/sdb1");

	/* struct btrfs_ioctl_dev_replace_args */
	ioctl(-1, BTRFS_IOC_DEV_REPLACE, NULL);
	printf("ioctl(-1, BTRFS_IOC_DEV_REPLACE, NULL) = -1 EBADF (%m)\n");

	ioctl(-1, BTRFS_IOC_DEV_REPLACE, &args);
	printf("ioctl(-1, BTRFS_IOC_DEV_REPLACE, "
	       "{cmd=BTRFS_IOCTL_DEV_REPLACE_CMD_START, start={srcdevid=%"
	       PRI__u64", cont_reading_from_srcdev_mode=%" PRI__u64
	       ", srcdev_name=\"%s\", tgtdev_name=\"%s\"}}) = -1 EBADF (%m)\n",
	       args.start.srcdevid,
	       args.start.cont_reading_from_srcdev_mode,
	       (char *)args.start.srcdev_name,
	       (char *)args.start.tgtdev_name);

	args.cmd = 1;
	ioctl(-1, BTRFS_IOC_DEV_REPLACE, &args);
	printf("ioctl(-1, BTRFS_IOC_DEV_REPLACE, "
	       "{cmd=BTRFS_IOCTL_DEV_REPLACE_CMD_STATUS}) = -1 EBADF (%m)\n");
}

static void
btrfs_test_extent_same_ioctl(void)
{
#ifdef BTRFS_IOC_FILE_EXTENT_SAME
	struct file_dedupe_range args = {
		.src_offset = 1024,
		.src_length = 10240,
	};
	struct file_dedupe_range *argsp;

	ioctl(-1, BTRFS_IOC_FILE_EXTENT_SAME, NULL);
	printf("ioctl(-1, BTRFS_IOC_FILE_EXTENT_SAME or FIDEDUPERANGE, "
	       "NULL) = -1 EBADF (%m)\n");

	printf("ioctl(-1, BTRFS_IOC_FILE_EXTENT_SAME or FIDEDUPERANGE, "
	       "{src_offset=%" PRIu64
	       ", src_length=%" PRIu64
	       ", dest_count=%hu, info=[]",
		(uint64_t)args.src_offset,
		(uint64_t)args.src_length, args.dest_count);
	ioctl(-1, BTRFS_IOC_FILE_EXTENT_SAME, &args);
	printf("}) = -1 EBADF (%m)\n");

	argsp = malloc(sizeof(*argsp) + sizeof(argsp->info[0]) * 3);
	if (!argsp)
		perror_msg_and_fail("malloc failed");
	memset(argsp, 0, sizeof(*argsp) + sizeof(argsp->info[0]) * 3);

	*argsp = args;
	argsp->dest_count = 3;
	argsp->info[0].dest_fd = 2;
	argsp->info[0].dest_offset = 0;
	argsp->info[1].dest_fd = 2;
	argsp->info[1].dest_offset = 10240;
	argsp->info[2].dest_fd = 2;
	argsp->info[2].dest_offset = 20480;

	printf("ioctl(-1, BTRFS_IOC_FILE_EXTENT_SAME or FIDEDUPERANGE, "
	       "{src_offset=%" PRIu64
	       ", src_length=%" PRIu64
	       ", dest_count=%hu, info=",
		(int64_t)argsp->src_offset,
		(uint64_t)argsp->src_length, argsp->dest_count);
		printf("[{dest_fd=%" PRId64 ", dest_offset=%" PRIu64
		       "}, {dest_fd=%" PRId64 ", dest_offset=%"PRIu64 "}",
		       (int64_t)argsp->info[0].dest_fd,
		       (uint64_t)argsp->info[0].dest_offset,
		       (int64_t)argsp->info[1].dest_fd,
		       (uint64_t)argsp->info[1].dest_offset);
	if (verbose)
		printf(", {dest_fd=%" PRId64 ", dest_offset=%" PRIu64 "}",
		       (int64_t)argsp->info[2].dest_fd,
		       (uint64_t)argsp->info[2].dest_offset);
	else
		printf(", ...");
	printf("]");
	ioctl(-1, BTRFS_IOC_FILE_EXTENT_SAME, argsp);
	printf("}) = -1 EBADF (%m)\n");

	if (write_ok) {
		int fd1, fd2;
		char buf[16384];

		memset(buf, 0, sizeof(buf));

		fd1 = openat(btrfs_test_dir_fd, "file1", O_RDWR|O_CREAT, 0600);
		if (fd1 < 0)
			perror_msg_and_fail("open file1 failed");

		fd2 = openat(btrfs_test_dir_fd, "file2", O_RDWR|O_CREAT, 0600);
		if (fd2 < 0)
			perror_msg_and_fail("open file2 failed");

		if (write(fd1, buf, sizeof(buf)) < 0)
			perror_msg_and_fail("write: fd1");
		if (write(fd1, buf, sizeof(buf)) < 0)
			perror_msg_and_fail("write: fd1");
		if (write(fd2, buf, sizeof(buf)) < 0)
			perror_msg_and_fail("write: fd2");
		if (write(fd2, buf, sizeof(buf)) < 0)
			perror_msg_and_fail("write: fd2");

		close(fd2);
		fd2 = openat(btrfs_test_dir_fd, "file2", O_RDONLY);
		if (fd2 < 0)
			perror_msg_and_fail("open file2 failed");

		memset(argsp, 0, sizeof(*argsp) + sizeof(argsp->info[0]) * 3);

		argsp->src_offset = 0;
		argsp->src_length = 4096;
		argsp->dest_count = 3;
		argsp->info[0].dest_fd = fd2;
		argsp->info[0].dest_offset = 0;
		argsp->info[1].dest_fd = fd2;
		argsp->info[1].dest_offset = 10240;
		argsp->info[2].dest_fd = fd2;
		argsp->info[2].dest_offset = 20480;

		printf("ioctl(%d, BTRFS_IOC_FILE_EXTENT_SAME or FIDEDUPERANGE, "
		       "{src_offset=%" PRIu64 ", src_length=%" PRIu64
		       ", dest_count=%hu, info=", fd1,
		       (uint64_t)argsp->src_offset,
		       (uint64_t)argsp->src_length, argsp->dest_count);
		printf("[{dest_fd=%" PRId64 ", dest_offset=%" PRIu64
		       "}, {dest_fd=%" PRId64 ", dest_offset=%"PRIu64 "}",
		       (int64_t)argsp->info[0].dest_fd,
		       (uint64_t)argsp->info[0].dest_offset,
		       (int64_t)argsp->info[1].dest_fd,
		       (uint64_t)argsp->info[1].dest_offset);
		if (verbose)
			printf(", {dest_fd=%" PRId64
			       ", dest_offset=%" PRIu64 "}",
			       (int64_t)argsp->info[2].dest_fd,
			       (uint64_t)argsp->info[2].dest_offset);
		else
			printf(", ...");

		ioctl(fd1, BTRFS_IOC_FILE_EXTENT_SAME, argsp);
		printf("]} => {info=");
		printf("[{bytes_deduped=%" PRIu64 ", status=%d}, "
			"{bytes_deduped=%" PRIu64 ", status=%d}",
		       (uint64_t)argsp->info[0].bytes_deduped,
		       argsp->info[0].status,
		       (uint64_t)argsp->info[1].bytes_deduped,
		       argsp->info[1].status);
		if (verbose)
			printf(", {bytes_deduped=%" PRIu64 ", status=%d}",
			       (uint64_t)argsp->info[2].bytes_deduped,
			       argsp->info[2].status);
		else
			printf(", ...");
		printf("]}) = 0\n");
		close(fd1);
		close(fd2);
		unlinkat(btrfs_test_dir_fd, "file1", 0);
		unlinkat(btrfs_test_dir_fd, "file2", 0);
		close(fd1);
		close(fd2);
	}
	free(argsp);
#endif /* BTRFS_IOC_FILE_EXTENT_SAME */
}

static void
btrfs_print_features(struct btrfs_ioctl_feature_flags *flags)
{
	printf("{compat_flags=");
	printflags(btrfs_features_compat, flags->compat_flags,
		   "BTRFS_FEATURE_COMPAT_???");

	printf(", compat_ro_flags=");
	printflags(btrfs_features_compat_ro, flags->compat_ro_flags,
		   "BTRFS_FEATURE_COMPAT_RO_???");

	printf(", incompat_flags=");
	printflags(btrfs_features_incompat, flags->incompat_flags,
		   "BTRFS_FEATURE_INCOMPAT_???");
	printf("}");
}

/*
 * Consumes argument, returns nothing:
 * - BTRFS_IOC_SET_FEATURES
 *
 * Consumes nothing, returns argument:
 * - BTRFS_IOC_GET_FEATURES
 * - BTRFS_IOC_GET_SUPPORTED_FEATURES
 */
static void
btrfs_test_features_ioctls(void)
{
	struct btrfs_ioctl_feature_flags args[2] = {
		{
			.compat_flags = max_flags_plus_one(-1),
			.incompat_flags = max_flags_plus_one(9),
			.compat_ro_flags = max_flags_plus_one(0),
		}, {
			.compat_flags = max_flags_plus_one(-1),
			.incompat_flags = max_flags_plus_one(9),
			.compat_ro_flags = max_flags_plus_one(0),
		},
	};
	struct btrfs_ioctl_feature_flags supported_features[3];

	ioctl(-1, BTRFS_IOC_SET_FEATURES, NULL);
	printf("ioctl(-1, BTRFS_IOC_SET_FEATURES, NULL) = -1 EBADF (%m)\n");

	printf("ioctl(-1, BTRFS_IOC_SET_FEATURES, [");
	btrfs_print_features(&args[0]);
	printf(", ");
	btrfs_print_features(&args[1]);
	ioctl(-1, BTRFS_IOC_SET_FEATURES, &args);
	printf("]) = -1 EBADF (%m)\n");

	if (btrfs_test_root) {
		printf("ioctl(%d, BTRFS_IOC_GET_FEATURES, ",
		       btrfs_test_dir_fd);
		ioctl(btrfs_test_dir_fd, BTRFS_IOC_GET_FEATURES,
		      &supported_features);
		btrfs_print_features(&supported_features[0]);
		printf(") = 0\n");

		ioctl(btrfs_test_dir_fd, BTRFS_IOC_GET_SUPPORTED_FEATURES,
		      &supported_features);
		printf("ioctl(%d, BTRFS_IOC_GET_SUPPORTED_FEATURES, ",
		       btrfs_test_dir_fd);
		printf("[");
		btrfs_print_features(&supported_features[0]);
		printf(" /* supported */, ");
		btrfs_print_features(&supported_features[1]);
		printf(" /* safe to set */, ");
		btrfs_print_features(&supported_features[2]);
		printf(" /* safe to clear */]) = 0\n");
	}
}

static void
btrfs_test_read_ioctls(void)
{
	static const struct xlat btrfs_read_cmd[] = {
		XLAT(BTRFS_IOC_BALANCE_PROGRESS),
		XLAT(BTRFS_IOC_FS_INFO),
		XLAT(BTRFS_IOC_GET_FEATURES),
		XLAT(BTRFS_IOC_GET_FSLABEL),
		XLAT(BTRFS_IOC_GET_SUPPORTED_FEATURES),
		XLAT(BTRFS_IOC_QGROUP_LIMIT),
		XLAT(BTRFS_IOC_QUOTA_RESCAN_STATUS),
		XLAT(BTRFS_IOC_START_SYNC),
		XLAT(BTRFS_IOC_SUBVOL_GETFLAGS),
	};

	unsigned int i;
	for (i = 0; i < ARRAY_SIZE(btrfs_read_cmd); ++i) {
		ioctl(-1, (unsigned long) btrfs_read_cmd[i].val, 0);
		printf("ioctl(-1, %s, NULL) = -1 EBADF (%m)\n", btrfs_read_cmd[i].str);
	}
}

int
main(int argc, char *argv[])
{

	int opt;
	int ret;
	const char *path;

	while ((opt = getopt(argc, argv, "wv")) != -1) {
		switch (opt) {
		case 'v':
			/*
			 * These tests are incomplete, especially when
			 * printing arrays of objects are involved.
			 */
			verbose = true;
			break;
		case 'w':
			write_ok = true;
			break;
		default:
			error_msg_and_fail("usage: btrfs [-v] [-w] [path]");
		}
	}

	/*
	 * This will enable optional tests that require a valid file descriptor
	 */
	if (optind < argc) {
		int rootfd;
		struct statfs sfi;
		path = argv[optind];

		ret = statfs(path, &sfi);
		if (ret)
			perror_msg_and_fail("statfs(%s) failed", path);

		if ((unsigned) sfi.f_type != BTRFS_SUPER_MAGIC)
			error_msg_and_fail("%s is not a btrfs file system",
					   path);

		btrfs_test_root = path;
		rootfd = open(path, O_RDONLY|O_DIRECTORY);
		if (rootfd < 0)
			perror_msg_and_fail("open(%s) failed", path);

		ret = mkdirat(rootfd, "strace-test", 0755);
		if (ret < 0 && errno != EEXIST)
			perror_msg_and_fail("mkdirat(strace-test) failed");

		btrfs_test_dir_fd = openat(rootfd, "strace-test",
					   O_RDONLY|O_DIRECTORY);
		if (btrfs_test_dir_fd < 0)
			perror_msg_and_fail("openat(strace-test) failed");
		close(rootfd);
	} else
		write_ok = false;

	if (btrfs_test_root) {
		fprintf(stderr, "Testing live ioctls on %s (%s)\n",
			btrfs_test_root, write_ok ? "read/write" : "read only");
	}

	btrfs_test_read_ioctls();
	btrfs_test_trans_ioctls();
	btrfs_test_sync_ioctls();
	btrfs_test_subvol_ioctls();
	btrfs_test_balance_ioctls();
	btrfs_test_device_ioctls();
	btrfs_test_clone_ioctls();
	btrfs_test_defrag_ioctls();
	btrfs_test_search_ioctls();
	btrfs_test_ino_lookup_ioctl();
	btrfs_test_space_info_ioctl();
	btrfs_test_scrub_ioctls();
	btrfs_test_dev_info_ioctl();
	btrfs_test_ino_path_ioctls();
	btrfs_test_set_received_subvol_ioctl();
	btrfs_test_send_ioctl();
	btrfs_test_quota_ctl_ioctl();
	btrfs_test_qgroup_assign_ioctl();
	btrfs_test_qgroup_create_ioctl();
	btrfs_test_quota_rescan_ioctl();
	btrfs_test_label_ioctls();
	btrfs_test_get_dev_stats_ioctl();
	btrfs_test_dev_replace_ioctl();
	btrfs_test_extent_same_ioctl();
	btrfs_test_features_ioctls();

	puts("+++ exited with 0 +++");

	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_LINUX_BTRFS_H")

#endif
