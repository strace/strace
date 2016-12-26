/*
 * Copyright (c) 2012 Mike Frysinger <vapier@gentoo.org>
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

#include <linux/ioctl.h>

/* The UBI api changes, so we have to keep a local copy */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 7, 0)
# include "ubi-user.h"
#else
# include <mtd/ubi-user.h>
#endif

#include "xlat/ubi_volume_types.h"
#include "xlat/ubi_volume_props.h"

int
ubi_ioctl(struct tcb *const tcp, const unsigned int code,
	  const kernel_ulong_t arg)
{
	if (!verbose(tcp))
		return RVAL_DECODED;

	switch (code) {
	case UBI_IOCMKVOL:
		if (entering(tcp)) {
			struct ubi_mkvol_req mkvol;

			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &mkvol))
				break;

			tprintf("{vol_id=%" PRIi32 ", alignment=%" PRIi32
				", bytes=%" PRIi64 ", vol_type=", mkvol.vol_id,
				mkvol.alignment, (int64_t)mkvol.bytes);
			printxval(ubi_volume_types,
				    (uint8_t) mkvol.vol_type, "UBI_???_VOLUME");
			tprintf(", name_len=%" PRIi16 ", name=", mkvol.name_len);
			if (print_quoted_string(mkvol.name,
					CLAMP(mkvol.name_len, 0, UBI_MAX_VOLUME_NAME),
					QUOTE_0_TERMINATED) > 0) {
				tprints("...");
			}
			tprints("}");
			return 1;
		}
		if (!syserror(tcp)) {
			tprints(" => ");
			printnum_int(tcp, arg, "%d");
		}
		break;

	case UBI_IOCRSVOL: {
		struct ubi_rsvol_req rsvol;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &rsvol))
			break;

		tprintf("{vol_id=%" PRIi32 ", bytes=%" PRIi64 "}",
			rsvol.vol_id, (int64_t)rsvol.bytes);
		break;
	}

	case UBI_IOCRNVOL: {
		struct ubi_rnvol_req rnvol;
		int c;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &rnvol))
			break;

		tprintf("{count=%" PRIi32 ", ents=[", rnvol.count);
		for (c = 0; c < CLAMP(rnvol.count, 0, UBI_MAX_RNVOL); ++c) {
			if (c)
				tprints(", ");
			tprintf("{vol_id=%" PRIi32 ", name_len=%" PRIi16
				", name=", rnvol.ents[c].vol_id,
				rnvol.ents[c].name_len);
			if (print_quoted_string(rnvol.ents[c].name,
					CLAMP(rnvol.ents[c].name_len, 0, UBI_MAX_VOLUME_NAME),
					QUOTE_0_TERMINATED) > 0) {
				tprints("...");
			}
			tprints("}");
		}
		tprints("]}");
		break;
	}

	case UBI_IOCEBCH: {
		struct ubi_leb_change_req leb;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &leb))
			break;

		tprintf("{lnum=%d, bytes=%d}", leb.lnum, leb.bytes);
		break;
	}

	case UBI_IOCATT:
		if (entering(tcp)) {
			struct ubi_attach_req attach;

			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &attach))
				break;

			tprintf("{ubi_num=%" PRIi32 ", mtd_num=%" PRIi32
				", vid_hdr_offset=%" PRIi32
				", max_beb_per1024=%" PRIi16 "}",
				attach.ubi_num, attach.mtd_num,
				attach.vid_hdr_offset, attach.max_beb_per1024);
			return 1;
		}
		if (!syserror(tcp)) {
			tprints(" => ");
			printnum_int(tcp, arg, "%d");
		}
		break;

	case UBI_IOCEBMAP: {
		struct ubi_map_req map;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &map))
			break;

		tprintf("{lnum=%" PRIi32 ", dtype=%" PRIi8 "}",
			map.lnum, map.dtype);
		break;
	}

	case UBI_IOCSETVOLPROP: {
		struct ubi_set_vol_prop_req prop;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &prop))
			break;

		tprints("{property=");
		printxval(ubi_volume_props, prop.property, "UBI_VOL_PROP_???");
		tprintf(", value=%#" PRIx64 "}", (uint64_t)prop.value);
		break;
	}


	case UBI_IOCVOLUP:
		tprints(", ");
		printnum_int64(tcp, arg, "%" PRIi64);
		break;

	case UBI_IOCDET:
	case UBI_IOCEBER:
	case UBI_IOCEBISMAP:
	case UBI_IOCEBUNMAP:
	case UBI_IOCRMVOL:
		tprints(", ");
		printnum_int(tcp, arg, "%d");
		break;

#ifdef UBI_IOCVOLCRBLK
	case UBI_IOCVOLCRBLK:
#endif
#ifdef UBI_IOCVOLRMBLK
	case UBI_IOCVOLRMBLK:
#endif
		/* no arguments */
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_DECODED | 1;
}
