/*
 * Check decoding of linux/fs.h 0x15 ioctl commands.
 *
 * Copyright (c) 2020-2025 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <linux/fs.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>

static const char *errstr;

static int
do_ioctl(kernel_ulong_t cmd, kernel_ulong_t arg)
{
	int rc = ioctl(-1, cmd, arg);
	errstr = sprintrc(rc);

#ifdef INJECT_RETVAL
	if (rc != INJECT_RETVAL)
		error_msg_and_fail("Return value [%d] does not match"
				   " expectations [%d]", rc, INJECT_RETVAL);

	static char inj_errstr[4096];

	snprintf(inj_errstr, sizeof(inj_errstr), "%s (INJECTED)", errstr);
	errstr = inj_errstr;
#endif

	return rc;
}

static int
do_ioctl_ptr(kernel_ulong_t cmd, const void *arg)
{
	return do_ioctl(cmd, (uintptr_t) arg);
}

static const struct strval32 hex_arg_cmds[] = {
	{ _IO(0x15, 0xff), "_IOC(_IOC_NONE, 0x15, 0xff, 0)" },
};

#ifdef INJECT_RETVAL
static void
skip_ioctls(int argc, const char *argv[])
{
	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	unsigned long num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; ++i) {
		int rc = ioctl(-1, hex_arg_cmds[0].val, 0);

		printf("ioctl(-1, " XLAT_FMT ", 0) = %s%s\n",
		       XLAT_SEL(hex_arg_cmds[0].val, hex_arg_cmds[0].str),
		       sprintrc(rc),
		       rc == INJECT_RETVAL ? " (INJECTED)" : "");

		if (rc == INJECT_RETVAL)
			return;
	}

	error_msg_and_fail("Issued %lu ioctl syscalls but failed"
			   " to detect an injected return code %d",
			   num_skip, INJECT_RETVAL);
}
#endif /* INJECT_RETVAL */

int
main(int argc, const char *argv[])
{
#ifdef INJECT_RETVAL
	skip_ioctls(argc, argv);
#endif

	static const unsigned long hex_args[] = {
		0,
		(unsigned long) 0xbadc0deddeadc0deULL,
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(hex_arg_cmds); ++i) {
		for (unsigned int j = 0; j < ARRAY_SIZE(hex_args); ++j) {
			do_ioctl(hex_arg_cmds[i].val, hex_args[j]);
			printf("ioctl(-1, " XLAT_FMT ", %#lx) = %s\n",
			       XLAT_SEL(hex_arg_cmds[i].val, hex_arg_cmds[i].str),
			       hex_args[j], errstr);
		}
	}

	static const struct strval32 null_arg_cmds[] = {
		{ ARG_STR(FS_IOC_GETFSUUID) },
		{ ARG_STR(FS_IOC_GETFSSYSFSPATH) },
		{ ARG_STR(FS_IOC_GETLBMD_CAP) },
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(null_arg_cmds); ++i) {
		do_ioctl(null_arg_cmds[i].val, 0);
		printf("ioctl(-1, " XLAT_FMT ", NULL) = %s\n",
		       XLAT_SEL(null_arg_cmds[i].val, null_arg_cmds[i].str),
		       errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct fsuuid2, p_uuid);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct fs_sysfs_path, p_path);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct logical_block_metadata_cap, p_cap);

	const struct {
		uint32_t val;
		const char *str;
		const void *ptr;
	} efault_arg_cmds[] = {
		{ ARG_STR(FS_IOC_GETFSUUID), (char *) p_uuid + 1 },
		{ ARG_STR(FS_IOC_GETFSSYSFSPATH), (char *) p_path + 1 },
		{ ARG_STR(FS_IOC_GETLBMD_CAP), (char *) p_cap + 1 },
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(efault_arg_cmds); ++i) {
		do_ioctl_ptr(efault_arg_cmds[i].val, efault_arg_cmds[i].ptr);
		printf("ioctl(-1, " XLAT_FMT ", %p) = %s\n",
		       XLAT_SEL(efault_arg_cmds[i].val, efault_arg_cmds[i].str),
		       efault_arg_cmds[i].ptr, errstr);
	}

	/* FS_IOC_GETFSUUID */

	static const unsigned int uuid_lens[] = {
		sizeof(p_uuid->uuid) - 1,
		sizeof(p_uuid->uuid),
		sizeof(p_uuid->uuid) + 1,
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(uuid_lens); ++i) {
		const unsigned int uuid_len =
			MIN(uuid_lens[i], (unsigned int) sizeof(p_uuid->uuid));
		p_uuid->len = uuid_lens[i];
		fill_memory(p_uuid->uuid, sizeof(p_uuid->uuid));

		if (do_ioctl_ptr(FS_IOC_GETFSUUID, p_uuid) < 0) {
			printf("ioctl(-1, %s, %p) = %s\n",
			       XLAT_STR(FS_IOC_GETFSUUID), p_uuid, errstr);
		} else {
			printf("ioctl(-1, %s, {len=%u, uuid=",
			       XLAT_STR(FS_IOC_GETFSUUID), p_uuid->len);
			print_quoted_hex(p_uuid->uuid, uuid_len);
			printf("}) = %s\n", errstr);
		}
	}

	/* FS_IOC_GETFSSYSFSPATH */

	static const unsigned int name_lens[] = {
		0,
		1,
		sizeof(p_path->name) - 1,
		sizeof(p_path->name),
		sizeof(p_path->name) + 1,
	};

	for (unsigned int i = 0; i < ARRAY_SIZE(name_lens); ++i) {
		p_path->len = name_lens[i];
		fill_memory_ex(p_path->name, sizeof(p_path->name),
			       '#', 'Z' - '#' + 1);

		if (do_ioctl_ptr(FS_IOC_GETFSSYSFSPATH, p_path) < 0) {
			printf("ioctl(-1, %s, %p) = %s\n",
			       XLAT_STR(FS_IOC_GETFSSYSFSPATH), p_path, errstr);
		} else {
			printf("ioctl(-1, %s, {len=%u, name=\"%.*s\"...}) = %s\n",
			       XLAT_STR(FS_IOC_GETFSSYSFSPATH), p_path->len,
			       (int) MIN(p_path->len, sizeof(p_path->name) - 1),
			       p_path->name, errstr);

			if (p_path->len < sizeof(p_path->name)) {
				p_path->name[p_path->len] = '\0';
				do_ioctl_ptr(FS_IOC_GETFSSYSFSPATH, p_path);

				printf("ioctl(-1, %s, {len=%u, name=\"%.*s\"})"
				       " = %s\n",
				       XLAT_STR(FS_IOC_GETFSSYSFSPATH),
				       p_path->len, (int) p_path->len,
				       p_path->name, errstr);
			}
		}

	}

	/* FS_IOC_GETLBMD_CAP */

#define	VALID_LBMD_FLAGS	0x3
#define VALID_LBMD_FLAGS_STR	"LBMD_PI_CAP_INTEGRITY|LBMD_PI_CAP_REFTAG"
#define	INVALID_LBMD_FLAGS	0xfffffffc

#define VALID_LBMD_TYPE		0x1
#define VALID_LBMD_TYPE_STR	"LBMD_PI_CSUM_IP"
#define INVALID_LBMD_TYPE	0x8

	fill_memory(p_cap, sizeof(*p_cap));
	p_cap->lbmd_flags = VALID_LBMD_FLAGS;
	p_cap->lbmd_guard_tag_type = VALID_LBMD_TYPE;

	if (do_ioctl_ptr(FS_IOC_GETLBMD_CAP, p_cap) < 0) {
		printf("ioctl(-1, %s, %p) = %s\n",
		       XLAT_STR(FS_IOC_GETLBMD_CAP), p_cap, errstr);
	} else {
		printf("ioctl(-1, %s, {lbmd_flags=%s"
		       ", lbmd_interval=%u"
		       ", lbmd_size=%u"
		       ", lbmd_opaque_size=%u"
		       ", lbmd_opaque_offset=%u"
		       ", lbmd_pi_size=%u"
		       ", lbmd_pi_offset=%u"
		       ", lbmd_guard_tag_type=%s"
		       ", lbmd_app_tag_size=%u"
		       ", lbmd_ref_tag_size=%u"
		       ", lbmd_storage_tag_size=%u}) = %s\n",
		       XLAT_STR(FS_IOC_GETLBMD_CAP),
		       XLAT_KNOWN(VALID_LBMD_FLAGS, VALID_LBMD_FLAGS_STR),
		       p_cap->lbmd_interval,
		       p_cap->lbmd_size,
		       p_cap->lbmd_opaque_size,
		       p_cap->lbmd_opaque_offset,
		       p_cap->lbmd_pi_size,
		       p_cap->lbmd_pi_offset,
		       XLAT_KNOWN(VALID_LBMD_TYPE, VALID_LBMD_TYPE_STR),
		       p_cap->lbmd_app_tag_size,
		       p_cap->lbmd_ref_tag_size,
		       p_cap->lbmd_storage_tag_size,
		       errstr);
	}

	p_cap->lbmd_flags = INVALID_LBMD_FLAGS;
	p_cap->lbmd_guard_tag_type = INVALID_LBMD_TYPE;

	if (do_ioctl_ptr(FS_IOC_GETLBMD_CAP, p_cap) < 0) {
		printf("ioctl(-1, %s, %p) = %s\n",
		       XLAT_STR(FS_IOC_GETLBMD_CAP), p_cap, errstr);
	} else {
		printf("ioctl(-1, %s, {lbmd_flags=%s"
		       ", lbmd_interval=%u"
		       ", lbmd_size=%u"
		       ", lbmd_opaque_size=%u"
		       ", lbmd_opaque_offset=%u"
		       ", lbmd_pi_size=%u"
		       ", lbmd_pi_offset=%u"
		       ", lbmd_guard_tag_type=%s"
		       ", lbmd_app_tag_size=%u"
		       ", lbmd_ref_tag_size=%u"
		       ", lbmd_storage_tag_size=%u}) = %s\n",
		       XLAT_STR(FS_IOC_GETLBMD_CAP),
		       XLAT_UNKNOWN(INVALID_LBMD_FLAGS, "LBMD_PI_CAP_???"),
		       p_cap->lbmd_interval,
		       p_cap->lbmd_size,
		       p_cap->lbmd_opaque_size,
		       p_cap->lbmd_opaque_offset,
		       p_cap->lbmd_pi_size,
		       p_cap->lbmd_pi_offset,
		       XLAT_UNKNOWN(INVALID_LBMD_TYPE, "LBMD_PI_CSUM_???"),
		       p_cap->lbmd_app_tag_size,
		       p_cap->lbmd_ref_tag_size,
		       p_cap->lbmd_storage_tag_size,
		       errstr);
	}

	puts("+++ exited with 0 +++");
	return 0;
}
