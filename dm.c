/*
 * Support for decoding of DM_* ioctl commands.
 *
 * Copyright (c) 2016 Mikulas Patocka <mpatocka@redhat.com>
 * Copyright (c) 2016 Masatake Yamato <yamato@redhat.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"

#ifdef HAVE_LINUX_DM_IOCTL_H

# include <linux/dm-ioctl.h>
# include <linux/ioctl.h>

# if DM_VERSION_MAJOR == 4

/* Definitions for command which have been added later */

#  ifndef DM_LIST_VERSIONS
#   define DM_LIST_VERSIONS    _IOWR(DM_IOCTL, 0xd, struct dm_ioctl)
#  endif
#  ifndef DM_TARGET_MSG
#   define DM_TARGET_MSG       _IOWR(DM_IOCTL, 0xe, struct dm_ioctl)
#  endif
#  ifndef DM_DEV_SET_GEOMETRY
#   define DM_DEV_SET_GEOMETRY _IOWR(DM_IOCTL, 0xf, struct dm_ioctl)
#  endif


static void
dm_decode_device(const unsigned int code, const struct dm_ioctl *ioc)
{
	switch (code) {
	case DM_REMOVE_ALL:
	case DM_LIST_DEVICES:
	case DM_LIST_VERSIONS:
		break;
	default:
		if (ioc->dev) {
			tprints(", dev=");
			print_dev_t(ioc->dev);
		}
		if (ioc->name[0]) {
			tprints(", name=");
			print_quoted_string(ioc->name, DM_NAME_LEN,
					    QUOTE_0_TERMINATED);
		}
		if (ioc->uuid[0]) {
			tprints(", uuid=");
			print_quoted_string(ioc->uuid, DM_UUID_LEN,
					    QUOTE_0_TERMINATED);
		}
		break;
	}
}

static void
dm_decode_values(struct tcb *tcp, const unsigned int code,
		 const struct dm_ioctl *ioc)
{
	if (entering(tcp)) {
		switch (code) {
		case DM_TABLE_LOAD:
			tprintf(", target_count=%" PRIu32,
				ioc->target_count);
			break;
		case DM_DEV_SUSPEND:
			if (ioc->flags & DM_SUSPEND_FLAG)
				break;
			/* Fall through */
		case DM_DEV_RENAME:
		case DM_DEV_REMOVE:
		case DM_DEV_WAIT:
			tprintf(", event_nr=%" PRIu32,
				ioc->event_nr);
			break;
		}
	} else if (!syserror(tcp)) {
		switch (code) {
		case DM_DEV_CREATE:
		case DM_DEV_RENAME:
		case DM_DEV_SUSPEND:
		case DM_DEV_STATUS:
		case DM_DEV_WAIT:
		case DM_TABLE_LOAD:
		case DM_TABLE_CLEAR:
		case DM_TABLE_DEPS:
		case DM_TABLE_STATUS:
		case DM_TARGET_MSG:
			tprintf(", target_count=%" PRIu32,
				ioc->target_count);
			tprintf(", open_count=%" PRIu32,
				ioc->open_count);
			tprintf(", event_nr=%" PRIu32,
				ioc->event_nr);
			break;
		}
	}
}

#include "xlat/dm_flags.h"

static void
dm_decode_flags(const struct dm_ioctl *ioc)
{
	tprints(", flags=");
	printflags(dm_flags, ioc->flags, "DM_???");
}

static void
dm_decode_dm_target_spec(struct tcb *const tcp, const kernel_ulong_t addr,
			 const struct dm_ioctl *const ioc)
{
	static const uint32_t target_spec_size =
		sizeof(struct dm_target_spec);
	uint32_t i;
	uint32_t offset = ioc->data_start;
	uint32_t offset_end = 0;

	if (abbrev(tcp)) {
		if (ioc->target_count)
			tprints(", ...");

		return;
	}

	for (i = 0; i < ioc->target_count; i++) {
		tprints(", ");

		if (i && offset <= offset_end)
			goto misplaced;

		offset_end = offset + target_spec_size;

		if (offset_end <= offset || offset_end > ioc->data_size)
			goto misplaced;

		if (i >= max_strlen) {
			tprints("...");
			break;
		}

		struct dm_target_spec s;

		if (umove_or_printaddr(tcp, addr + offset, &s))
			break;

		tprintf("{sector_start=%" PRI__u64 ", length=%" PRI__u64,
			s.sector_start, s.length);

		if (exiting(tcp))
			tprintf(", status=%" PRId32, s.status);

		tprints(", target_type=");
		print_quoted_string(s.target_type, DM_MAX_TYPE_NAME,
				    QUOTE_0_TERMINATED);

		tprints(", string=");
		printstr_ex(tcp, addr + offset_end, ioc->data_size - offset_end,
			     QUOTE_0_TERMINATED);
		tprints("}");

		if (entering(tcp))
			offset += s.next;
		else
			offset = ioc->data_start + s.next;
	}

	return;

misplaced:
	tprints("???");
	tprints_comment("misplaced struct dm_target_spec");
}

bool
dm_print_dev(struct tcb *tcp, void *dev_ptr, size_t dev_size, void *dummy)
{
	uint64_t *dev = (uint64_t *) dev_ptr;

	print_dev_t(*dev);

	return 1;
}

static void
dm_decode_dm_target_deps(struct tcb *const tcp, const kernel_ulong_t addr,
			 const struct dm_ioctl *const ioc)
{
	if (ioc->data_start == ioc->data_size)
		return;

	tprints(", ");

	if (abbrev(tcp)) {
		tprints("...");
		return;
	}

	static const uint32_t target_deps_dev_offs =
		offsetof(struct dm_target_deps, dev);
	uint64_t dev_buf;
	struct dm_target_deps s;
	uint32_t offset = ioc->data_start;
	uint32_t offset_end = offset + target_deps_dev_offs;
	uint32_t space;

	if (offset_end <= offset || offset_end > ioc->data_size)
		goto misplaced;

	if (umove_or_printaddr(tcp, addr + offset, &s))
		return;

	space = (ioc->data_size - offset_end) / sizeof(dev_buf);

	if (s.count > space)
		goto misplaced;

	tprintf("{count=%u, deps=", s.count);

	print_array(tcp, addr + offset_end, s.count, &dev_buf, sizeof(dev_buf),
		    umoven_or_printaddr, dm_print_dev, NULL);

	tprints("}");

	return;

misplaced:
	tprints("???");
	tprints_comment("misplaced struct dm_target_deps");
}

static void
dm_decode_dm_name_list(struct tcb *const tcp, const kernel_ulong_t addr,
		       const struct dm_ioctl *const ioc)
{
	static const uint32_t name_list_name_offs =
		offsetof(struct dm_name_list, name);
	struct dm_name_list s;
	uint32_t offset = ioc->data_start;
	uint32_t offset_end = 0;
	uint32_t count;

	if (ioc->data_start == ioc->data_size)
		return;

	if (abbrev(tcp)) {
		tprints(", ...");
		return;
	}

	for (count = 0;; count++) {
		tprints(", ");

		if (count && offset <= offset_end)
			goto misplaced;

		offset_end = offset + name_list_name_offs;

		if (offset_end <= offset || offset_end > ioc->data_size)
			goto misplaced;

		if (count >= max_strlen) {
			tprints("...");
			break;
		}

		if (umove_or_printaddr(tcp, addr + offset, &s))
			break;

		tprints("{dev=");
		print_dev_t(s.dev);

		tprints("name=");
		printstr_ex(tcp, addr + offset_end, ioc->data_size - offset_end,
			    QUOTE_0_TERMINATED);
		tprints("}");

		if (!s.next)
			break;

		offset += s.next;
	}

	return;

misplaced:
	tprints("???");
	tprints_comment("misplaced struct dm_name_list");
}

static void
dm_decode_dm_target_versions(struct tcb *const tcp, const kernel_ulong_t addr,
			     const struct dm_ioctl *const ioc)
{
	static const uint32_t target_vers_name_offs =
		offsetof(struct dm_target_versions, name);
	struct dm_target_versions s;
	uint32_t offset = ioc->data_start;
	uint32_t offset_end = 0;
	uint32_t count;

	if (ioc->data_start == ioc->data_size)
		return;

	if (abbrev(tcp)) {
		tprints(", ...");
		return;
	}

	for (count = 0;; count++) {
		tprints(", ");

		if (count && offset <= offset_end)
			goto misplaced;

		offset_end = offset + target_vers_name_offs;

		if (offset_end <= offset || offset_end > ioc->data_size)
			goto misplaced;

		if (count >= max_strlen) {
			tprints("...");
			break;
		}

		if (umove_or_printaddr(tcp, addr + offset, &s))
			break;

		tprints("{name=");
		printstr_ex(tcp, addr + offset_end, ioc->data_size - offset_end,
			    QUOTE_0_TERMINATED);
		tprintf(", version=%" PRIu32 ".%" PRIu32 ".%" PRIu32 "}",
			s.version[0], s.version[1], s.version[2]);

		if (!s.next)
			break;

		offset += s.next;
	}

	return;

misplaced:
	tprints("???");
	tprints_comment("misplaced struct dm_target_versions");
}

static void
dm_decode_dm_target_msg(struct tcb *const tcp, const kernel_ulong_t addr,
		        const struct dm_ioctl *const ioc)
{
	if (ioc->data_start == ioc->data_size)
		return;

	tprints(", ");

	if (abbrev(tcp)) {
		tprints("...");
		return;
	}

	static const uint32_t target_msg_message_offs =
		offsetof(struct dm_target_msg, message);
	uint32_t offset = ioc->data_start;
	uint32_t offset_end = offset + target_msg_message_offs;

	if (offset_end > offset && offset_end <= ioc->data_size) {
		struct dm_target_msg s;

		if (umove_or_printaddr(tcp, addr + offset, &s))
			return;

		tprintf("{sector=%" PRI__u64 ", message=", s.sector);
		printstr_ex(tcp, addr + offset_end, ioc->data_size - offset_end,
			    QUOTE_0_TERMINATED);
		tprints("}");
	} else {
		tprints("???");
		tprints_comment("misplaced struct dm_target_msg");
	}
}

static void
dm_decode_string(struct tcb *const tcp, const kernel_ulong_t addr,
		 const struct dm_ioctl *const ioc)
{
	tprints(", ");

	if (abbrev(tcp)) {
		tprints("...");
		return;
	}

	uint32_t offset = ioc->data_start;

	if (offset <= ioc->data_size) {
		tprints("string=");
		printstr_ex(tcp, addr + offset, ioc->data_size - offset,
			    QUOTE_0_TERMINATED);
	} else {
		tprints("???");
		tprints_comment("misplaced string");
	}
}

static inline bool
dm_ioctl_has_params(const unsigned int code)
{
	switch (code) {
	case DM_VERSION:
	case DM_REMOVE_ALL:
	case DM_DEV_CREATE:
	case DM_DEV_REMOVE:
	case DM_DEV_SUSPEND:
	case DM_DEV_STATUS:
	case DM_TABLE_CLEAR:
		return false;
	}

	return true;
}

static int
dm_known_ioctl(struct tcb *const tcp, const unsigned int code,
	       const kernel_ulong_t arg)
{
	struct dm_ioctl *ioc = NULL;
	struct dm_ioctl *entering_ioc = NULL;
	bool ioc_changed = false;

	if (entering(tcp)) {
		ioc = malloc(sizeof(*ioc));
		if (!ioc)
			return 0;
	} else {
		ioc = alloca(sizeof(*ioc));
	}

	if ((umoven(tcp, arg, offsetof(struct dm_ioctl, data), ioc) < 0) ||
	    (ioc->data_size < offsetof(struct dm_ioctl, data_size))) {
		if (entering(tcp))
			free(ioc);
		return 0;
	}
	if (entering(tcp))
		set_tcb_priv_data(tcp, ioc, free);
	else {
		entering_ioc = get_tcb_priv_data(tcp);

		/*
		 * retrieve_status, __dev_status called only in case of success,
		 * so it looks like there's no need to check open_count,
		 * event_nr, target_count, dev fields for change (they are
		 * printed only in case of absence of errors).
		 */
		if (!entering_ioc ||
		    (ioc->version[0] != entering_ioc->version[0]) ||
		    (ioc->version[1] != entering_ioc->version[1]) ||
		    (ioc->version[2] != entering_ioc->version[2]) ||
		    (ioc->data_size != entering_ioc->data_size) ||
		    (ioc->data_start != entering_ioc->data_start) ||
		    (ioc->flags != entering_ioc->flags))
			ioc_changed = true;
	}

	if (exiting(tcp) && syserror(tcp) && !ioc_changed)
		return 1;

	/*
	 * device mapper code uses %d in some places and %u in another, but
	 * fields themselves are declared as __u32.
	 */
	tprintf("%s{version=%u.%u.%u",  entering(tcp) ? ", " : " => ",
		ioc->version[0], ioc->version[1], ioc->version[2]);
	/*
	 * if we use a different version of ABI, do not attempt to decode
	 * ioctl fields
	 */
	if (ioc->version[0] != DM_VERSION_MAJOR) {
		tprints_comment("unsupported device mapper ABI version");
		goto skip;
	}

	tprintf(", data_size=%u", ioc->data_size);

	if (ioc->data_size < offsetof(struct dm_ioctl, data)) {
		tprints_comment("data_size too small");
		goto skip;
	}

	if (dm_ioctl_has_params(code))
		tprintf(", data_start=%u", ioc->data_start);

	dm_decode_device(code, ioc);
	dm_decode_values(tcp, code, ioc);
	dm_decode_flags(ioc);

	switch (code) {
	case DM_DEV_WAIT:
	case DM_TABLE_STATUS:
		if (entering(tcp) || syserror(tcp))
			break;
		dm_decode_dm_target_spec(tcp, arg, ioc);
		break;
	case DM_TABLE_LOAD:
		if (exiting(tcp))
			break;
		dm_decode_dm_target_spec(tcp, arg, ioc);
		break;
	case DM_TABLE_DEPS:
		if (entering(tcp) || syserror(tcp))
			break;
		dm_decode_dm_target_deps(tcp, arg, ioc);
		break;
	case DM_LIST_DEVICES:
		if (entering(tcp) || syserror(tcp))
			break;
		dm_decode_dm_name_list(tcp, arg, ioc);
		break;
	case DM_LIST_VERSIONS:
		if (entering(tcp) || syserror(tcp))
			break;
		dm_decode_dm_target_versions(tcp, arg, ioc);
		break;
	case DM_TARGET_MSG:
		if (entering(tcp))
			dm_decode_dm_target_msg(tcp, arg, ioc);
		else if (!syserror(tcp) && ioc->flags & DM_DATA_OUT_FLAG)
			dm_decode_string(tcp, arg, ioc);
		break;
	case DM_DEV_RENAME:
	case DM_DEV_SET_GEOMETRY:
		if (exiting(tcp))
			break;
		dm_decode_string(tcp, arg, ioc);
		break;
	}

 skip:
	tprints("}");
	return 1;
}

int
dm_ioctl(struct tcb *const tcp, const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case DM_VERSION:
	case DM_REMOVE_ALL:
	case DM_LIST_DEVICES:
	case DM_DEV_CREATE:
	case DM_DEV_REMOVE:
	case DM_DEV_RENAME:
	case DM_DEV_SUSPEND:
	case DM_DEV_STATUS:
	case DM_DEV_WAIT:
	case DM_TABLE_LOAD:
	case DM_TABLE_CLEAR:
	case DM_TABLE_DEPS:
	case DM_TABLE_STATUS:
	case DM_LIST_VERSIONS:
	case DM_TARGET_MSG:
	case DM_DEV_SET_GEOMETRY:
		return dm_known_ioctl(tcp, code, arg);
	default:
		return 0;
	}
}

# else /* !(DM_VERSION_MAJOR == 4) */

int
dm_ioctl(struct tcb *const tcp, const unsigned int code, const kernel_ulong_t arg)
{
	return 0;
}

# endif /* DM_VERSION_MAJOR == 4 */
#endif /* HAVE_LINUX_DM_IOCTL_H */
