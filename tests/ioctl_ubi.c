/*
 * Check decoding of UBI ioctl commands.
 *
 * Copyright (c) 2016-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <mtd/ubi-user.h>

static const unsigned long long llmagic = 0xdeadbeefbadc0dedULL;
static const unsigned long lmagic = (unsigned long) 0xdeadbeefbadc0dedULL;
static const unsigned int imagic = 0xdeadbeef;

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

#ifdef INJECT_RETVAL
static void
skip_ioctls(int argc, const char *argv[])
{
	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	unsigned long num_skip = strtoul(argv[1], NULL, 0);

	for (size_t i = 0; i < num_skip; ++i) {
		int rc = ioctl(-1, UBI_IOCATT, 0);
		printf("ioctl(-1, UBI_IOCATT, NULL) = %s%s\n", sprintrc(rc),
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

	static const struct {
		unsigned int cmd;
		const char *str;
	}
	noarg_cmds[] = {
		{ ARG_STR(UBI_IOCVOLCRBLK) },
		{ ARG_STR(UBI_IOCVOLRMBLK) },
	},
	pint_cmds[] = {
		{ ARG_STR(UBI_IOCDET) },
		{ ARG_STR(UBI_IOCEBER) },
		{ ARG_STR(UBI_IOCEBISMAP) },
		{ ARG_STR(UBI_IOCEBUNMAP) },
		{ ARG_STR(UBI_IOCRMVOL) },
		{ ARG_STR(UBI_IOCRPEB) },
		{ ARG_STR(UBI_IOCSPEB) },
	}, pint64_cmds[] = {
		{ ARG_STR(UBI_IOCVOLUP) },
	}, ptr_cmds[] = {
		{ ARG_STR(UBI_IOCATT) },
		{ ARG_STR(UBI_IOCEBCH) },
		{ ARG_STR(UBI_IOCEBMAP) },
		{ ARG_STR(UBI_IOCMKVOL) },
		{ ARG_STR(UBI_IOCRNVOL) },
		{ ARG_STR(UBI_IOCRSVOL) },
		{ ARG_STR(UBI_IOCSETVOLPROP) },
	}, attach_cmds[] = {
		{ ARG_STR(UBI_IOCATT) },
	}, leb_cmds[] = {
		{ ARG_STR(UBI_IOCEBCH) },
	}, map_cmds[] = {
		{ ARG_STR(UBI_IOCEBMAP) },
	}, mkvol_cmds[] = {
		{ ARG_STR(UBI_IOCMKVOL) },
	}, rnvol_cmds[] = {
		{ ARG_STR(UBI_IOCRNVOL) },
	}, rsvol_cmds[] = {
		{ ARG_STR(UBI_IOCRSVOL) },
	}, prop_cmds[] = {
		{ ARG_STR(UBI_IOCSETVOLPROP) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(noarg_cmds); ++i) {
		do_ioctl(noarg_cmds[i].cmd, lmagic);
		printf("ioctl(-1, %s) = %s\n",
		       noarg_cmds[i].str, errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(int, pint);
	*pint = imagic;

	for (size_t i = 0; i < ARRAY_SIZE(pint_cmds); ++i) {
		do_ioctl_ptr(pint_cmds[i].cmd, pint);
		printf("ioctl(-1, %s, [%d]) = %s\n",
		       pint_cmds[i].str, *pint, errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(int64_t, pint64);
	*pint64 = (int64_t) llmagic;

	for (size_t i = 0; i < ARRAY_SIZE(pint64_cmds); ++i) {
		do_ioctl_ptr(pint64_cmds[i].cmd, pint64);
		printf("ioctl(-1, %s, [%jd]) = %s\n",
		       pint64_cmds[i].str, (intmax_t) *pint64, errstr);
	}

	void *const efault = tail_alloc(1);

	for (size_t i = 0; i < ARRAY_SIZE(ptr_cmds); ++i) {
		do_ioctl(ptr_cmds[i].cmd, 0);
		printf("ioctl(-1, %s, NULL) = %s\n",
		       ptr_cmds[i].str, errstr);
		do_ioctl_ptr(ptr_cmds[i].cmd, efault);
		printf("ioctl(-1, %s, %p) = %s\n",
		       ptr_cmds[i].str, efault, errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ubi_attach_req, attach);
	fill_memory(attach, sizeof(*attach));

	for (size_t i = 0; i < ARRAY_SIZE(attach_cmds); ++i) {
		int rc = do_ioctl_ptr(attach_cmds[i].cmd, attach);
		printf("ioctl(-1, %s, {ubi_num=%d, mtd_num=%d"
		       ", vid_hdr_offset=%d, max_beb_per1024=%hd}",
		       attach_cmds[i].str, attach->ubi_num,
		       attach->mtd_num, attach->vid_hdr_offset,
		       attach->max_beb_per1024);
		if (rc >= 0)
			printf(" => [%d]", attach->ubi_num);
		printf(") = %s\n", errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ubi_leb_change_req, leb);
	fill_memory(leb, sizeof(*leb));
	leb->dtype = 3;

	for (size_t i = 0; i < ARRAY_SIZE(leb_cmds); ++i) {
		do_ioctl_ptr(leb_cmds[i].cmd, leb);
		printf("ioctl(-1, %s, {lnum=%d, bytes=%d, dtype=%s}) = %s\n",
		       leb_cmds[i].str, leb->lnum, leb->bytes, "UBI_UNKNOWN",
		       errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ubi_map_req, map);
	fill_memory(map, sizeof(*map));
	map->dtype = 3;

	for (size_t i = 0; i < ARRAY_SIZE(map_cmds); ++i) {
		do_ioctl_ptr(map_cmds[i].cmd, map);
		printf("ioctl(-1, %s, {lnum=%d, dtype=%s}) = %s\n",
		       map_cmds[i].str, map->lnum, "UBI_UNKNOWN", errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ubi_mkvol_req, mkvol);
	fill_memory(mkvol, sizeof(*mkvol));
	mkvol->vol_type = 4;
	mkvol->flags = 1;
	fill_memory_ex(mkvol->name, sizeof(mkvol->name), '0', 10);

	for (size_t i = 0; i < ARRAY_SIZE(mkvol_cmds); ++i) {
		static const int lens[] = {
			-1, 0, 1, UBI_MAX_VOLUME_NAME,
			UBI_MAX_VOLUME_NAME + 1
		};
		for (size_t j = 0; j < ARRAY_SIZE(lens); ++j) {
			mkvol->name_len = lens[j];
			int len = CLAMP(mkvol->name_len, 0,
					UBI_MAX_VOLUME_NAME);
			int rc = do_ioctl_ptr(mkvol_cmds[i].cmd, mkvol);
			printf("ioctl(-1, %s, {vol_id=%d, alignment=%d"
			       ", bytes=%jd, vol_type=%s, flags=%s"
			       ", name_len=%hd, name=\"%.*s\"...}",
			       mkvol_cmds[i].str, mkvol->vol_id,
			       mkvol->alignment, (intmax_t) mkvol->bytes,
			       "UBI_STATIC_VOLUME",
			       "UBI_VOL_SKIP_CRC_CHECK_FLG",
			       mkvol->name_len, len, mkvol->name);
			if (rc >= 0)
				printf(" => [%d]", mkvol->vol_id);
			printf(") = %s\n", errstr);
		}
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ubi_rnvol_req, rnvol);

	for (size_t i = 0; i < ARRAY_SIZE(rnvol_cmds); ++i) {
		fill_memory(rnvol, sizeof(*rnvol));
		do_ioctl_ptr(rnvol_cmds[i].cmd, rnvol);
		printf("ioctl(-1, %s, {count=%d, ents=[]}) = %s\n",
		       rnvol_cmds[i].str, rnvol->count, errstr);

		rnvol->count = 1;
		do_ioctl_ptr(rnvol_cmds[i].cmd, rnvol);
		printf("ioctl(-1, %s, {count=%d, ents=[{vol_id=%d"
		       ", name_len=%hd, name=\"\"...}]}) = %s\n",
		       rnvol_cmds[i].str, rnvol->count,
		       rnvol->ents[0].vol_id, rnvol->ents[0].name_len, errstr);

		rnvol->count = UBI_MAX_RNVOL + 1;
		for (size_t j = 0; j < UBI_MAX_RNVOL; ++j) {
			fill_memory_ex(rnvol->ents[j].name,
				       sizeof(rnvol->ents[j].name), '0', 10);
		}
		rnvol->ents[0].name_len = 0;
		rnvol->ents[1].name_len = 1;
		rnvol->ents[2].name_len = 1;
		rnvol->ents[2].name[1] = '\0';
		rnvol->ents[3].name_len = 2;
		rnvol->ents[3].name[1] = '\0';
		rnvol->ents[4].name_len = UBI_MAX_VOLUME_NAME;
		rnvol->ents[5].name_len = UBI_MAX_VOLUME_NAME;
		rnvol->ents[5].name[UBI_MAX_VOLUME_NAME] = '\0';
		rnvol->ents[6].name_len = UBI_MAX_VOLUME_NAME;
		rnvol->ents[6].name[UBI_MAX_VOLUME_NAME - 1] = '\0';
		rnvol->ents[7].name_len = UBI_MAX_VOLUME_NAME + 1;
		rnvol->ents[8].name_len = UBI_MAX_VOLUME_NAME + 1;
		rnvol->ents[8].name[UBI_MAX_VOLUME_NAME] = '\0';
		do_ioctl_ptr(rnvol_cmds[i].cmd, rnvol);
		printf("ioctl(-1, %s, {count=%d, ents=[",
		       rnvol_cmds[i].str, rnvol->count);
		for (size_t j = 0; j < UBI_MAX_RNVOL; ++j) {
			int len = CLAMP(rnvol->ents[j].name_len, 0,
					UBI_MAX_VOLUME_NAME);
			bool dots = rnvol->ents[j].name[len] &&
				    (len <= 0 || rnvol->ents[j].name[len - 1]);
			printf("%s{vol_id=%d, name_len=%hd, name=\"%.*s\"%s}",
			       j ? ", " : "",
			       rnvol->ents[j].vol_id,
			       rnvol->ents[j].name_len,
			       len, rnvol->ents[j].name,
			       dots ? "..." : "");
		}
		printf("]}) = %s\n", errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ubi_rsvol_req, rsvol);
	fill_memory(rsvol, sizeof(*rsvol));

	for (size_t i = 0; i < ARRAY_SIZE(rsvol_cmds); ++i) {
		do_ioctl_ptr(rsvol_cmds[i].cmd, rsvol);
		printf("ioctl(-1, %s, {bytes=%jd, vol_id=%d}) = %s\n",
		       rsvol_cmds[i].str, (intmax_t) rsvol->bytes,
		       rsvol->vol_id, errstr);
	}

	TAIL_ALLOC_OBJECT_CONST_PTR(struct ubi_set_vol_prop_req, prop);
	fill_memory(prop, sizeof(*prop));
	prop->property = UBI_VOL_PROP_DIRECT_WRITE;

	for (size_t i = 0; i < ARRAY_SIZE(prop_cmds); ++i) {
		do_ioctl_ptr(prop_cmds[i].cmd, prop);
		printf("ioctl(-1, %s, {property=%s, value=%#jx}) = %s\n",
		       prop_cmds[i].str, "UBI_VOL_PROP_DIRECT_WRITE",
		       (uintmax_t) prop->value, errstr);
	}

	do_ioctl(_IO(0x6f, 0x35), lmagic);
	printf("ioctl(-1, %s, %#lx) = %s\n", "NET_REMOVE_IF", lmagic, errstr);

	puts("+++ exited with 0 +++");
	return 0;
}
