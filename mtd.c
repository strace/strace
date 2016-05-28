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

/* The mtd api changes quickly, so we have to keep a local copy */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)
# include "mtd-abi.h"
#else
# include <mtd/mtd-abi.h>
#endif

#include "xlat/mtd_mode_options.h"
#include "xlat/mtd_file_mode_options.h"
#include "xlat/mtd_type_options.h"
#include "xlat/mtd_flags_options.h"
#include "xlat/mtd_otp_options.h"
#include "xlat/mtd_nandecc_options.h"

int
mtd_ioctl(struct tcb *tcp, const unsigned int code, const long arg)
{
	if (!verbose(tcp))
		return RVAL_DECODED;

	switch (code) {
	case MEMERASE:
	case MEMLOCK:
	case MEMUNLOCK:
	case MEMISLOCKED: {
		struct erase_info_user einfo;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &einfo))
			break;

		tprintf("{start=%#" PRIx32 ", length=%#" PRIx32 "}",
			einfo.start, einfo.length);
		break;
	}

	case MEMERASE64: {
		struct erase_info_user64 einfo64;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &einfo64))
			break;

		tprintf("{start=%#" PRIx64 ", length=%#" PRIx64 "}",
			(uint64_t) einfo64.start, (uint64_t) einfo64.length);
		break;
	}

	case MEMWRITEOOB:
	case MEMREADOOB: {
		struct mtd_oob_buf mbuf;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &mbuf))
			break;

		tprintf("{start=%#" PRIx32 ", length=%#" PRIx32 ", ptr=...}",
			mbuf.start, mbuf.length);
		break;
	}

	case MEMWRITEOOB64:
	case MEMREADOOB64: {
		struct mtd_oob_buf64 mbuf64;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &mbuf64))
			break;

		tprintf("{start=%#" PRIx64 ", length=%#" PRIx64 ", ptr=...}",
			(uint64_t) mbuf64.start, (uint64_t) mbuf64.length);
		break;
	}

	case MEMGETREGIONINFO: {
		struct region_info_user rinfo;

		if (entering(tcp)) {
			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &rinfo))
				break;
			tprintf("{regionindex=%#x", rinfo.regionindex);
			return 1;
		} else {
			if (syserror(tcp)) {
				tprints("}");
				break;
			}
			if (umove(tcp, arg, &rinfo) < 0) {
				tprints(", ???}");
				break;
			}
			tprintf(", offset=%#x, erasesize=%#x, numblocks=%#x}",
				rinfo.offset, rinfo.erasesize, rinfo.numblocks);
			break;
		}
	}

	case OTPLOCK: {
		struct otp_info oinfo;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &oinfo))
			break;

		tprintf("{start=%#" PRIx32 ", length=%#" PRIx32 ", locked=%" PRIu32 "}",
			oinfo.start, oinfo.length, oinfo.locked);
		break;
	}

	case MEMWRITE: {
		struct mtd_write_req mreq;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &mreq))
			break;

		tprintf("{start=%#" PRIx64 ", len=%#" PRIx64,
			(uint64_t) mreq.start, (uint64_t) mreq.len);
		tprintf(", ooblen=%#" PRIx64 ", usr_data=%#" PRIx64,
			(uint64_t) mreq.ooblen, (uint64_t) mreq.usr_data);
		tprintf(", usr_oob=%#" PRIx64 ", mode=",
			(uint64_t) mreq.usr_oob);
		printxval(mtd_mode_options, mreq.mode, "MTD_OPS_???");
		tprints(", padding=...}");
		break;
	}

	case OTPSELECT: {
		unsigned int i;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &i))
			break;

		tprints("[");
		printxval(mtd_otp_options, i, "MTD_OTP_???");
		tprints("]");
		break;
	}

	case MTDFILEMODE:
		tprints(", ");
		printxval_long(mtd_file_mode_options, arg, "MTD_FILE_MODE_???");
		break;

	case MEMGETBADBLOCK:
	case MEMSETBADBLOCK:
		tprints(", ");
		printnum_int64(tcp, arg, "%" PRIu64);
		break;

	case MEMGETINFO: {
		struct mtd_info_user minfo;

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &minfo))
			break;

		tprints("{type=");
		printxval(mtd_type_options, minfo.type, "MTD_???");
		tprints(", flags=");
		printflags(mtd_flags_options, minfo.flags, "MTD_???");
		tprintf(", size=%#" PRIx32 ", erasesize=%#" PRIx32,
			minfo.size, minfo.erasesize);
		tprintf(", writesize=%#" PRIx32 ", oobsize=%#" PRIx32,
			minfo.writesize, minfo.oobsize);
		tprintf(", padding=%#" PRIx64 "}",
			(uint64_t) minfo.padding);
		break;
	}

	case MEMGETOOBSEL: {
		struct nand_oobinfo ninfo;
		unsigned int i;

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &ninfo))
			break;

		tprints("{useecc=");
		printxval(mtd_nandecc_options, ninfo.useecc, "MTD_NANDECC_???");
		tprintf(", eccbytes=%#" PRIx32, ninfo.eccbytes);

		tprints(", oobfree={");
		for (i = 0; i < ARRAY_SIZE(ninfo.oobfree); ++i) {
			unsigned int j;

			if (i)
				tprints("}, ");
			tprints("{");
			for (j = 0; j < ARRAY_SIZE(ninfo.oobfree[0]); ++j) {
				if (j)
					tprints(", ");
				tprintf("%#" PRIx32, ninfo.oobfree[i][j]);
			}
		}

		tprints("}}, eccpos={");
		for (i = 0; i < ARRAY_SIZE(ninfo.eccpos); ++i) {
			if (i)
				tprints(", ");
			tprintf("%#" PRIx32, ninfo.eccpos[i]);
		}

		tprints("}");
		break;
	}

	case OTPGETREGIONINFO: {
		struct otp_info oinfo;

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &oinfo))
			break;

		tprintf("{start=%#" PRIx32 ", length=%#" PRIx32 ", locked=%" PRIu32 "}",
			oinfo.start, oinfo.length, oinfo.locked);
		break;
	}

	case ECCGETLAYOUT: {
		struct nand_ecclayout_user nlay;
		unsigned int i;

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &nlay))
			break;

		tprintf("{eccbytes=%#" PRIx32 ", eccpos={", nlay.eccbytes);
		for (i = 0; i < ARRAY_SIZE(nlay.eccpos); ++i) {
			if (i)
				tprints(", ");
			tprintf("%#" PRIx32, nlay.eccpos[i]);
		}
		tprintf("}, oobavail=%#" PRIx32 ", oobfree={", nlay.oobavail);
		for (i = 0; i < ARRAY_SIZE(nlay.oobfree); ++i) {
			if (i)
				tprints(", ");
			tprintf("{offset=%#" PRIx32 ", length=%#" PRIx32 "}",
				nlay.oobfree[i].offset, nlay.oobfree[i].length);
		}
		tprints("}");
		break;
	}

	case ECCGETSTATS: {
		struct mtd_ecc_stats estat;

		if (entering(tcp))
			return 0;

		tprints(", ");
		if (umove_or_printaddr(tcp, arg, &estat))
			break;

		tprintf("{corrected=%#" PRIx32 ", failed=%#" PRIx32,
			estat.corrected, estat.failed);
		tprintf(", badblocks=%#" PRIx32 ", bbtblocks=%#" PRIx32 "}",
			estat.badblocks, estat.bbtblocks);
		break;
	}

	case OTPGETREGIONCOUNT:
		if (entering(tcp))
			return 0;

		tprints(", ");
		printnum_int(tcp, arg, "%u");
		break;

	case MEMGETREGIONCOUNT:
		if (entering(tcp))
			return 0;

		tprints(", ");
		printnum_int(tcp, arg, "%d");
		break;

	default:
		return RVAL_DECODED;
	}

	return RVAL_DECODED | 1;
}
