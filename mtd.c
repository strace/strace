/*
 * Copyright (c) 2012 Mike Frysinger <vapier@gentoo.org>
 * Copyright (c) 2012-2018 The strace developers.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#ifdef HAVE_STRUCT_MTD_WRITE_REQ

# include DEF_MPERS_TYPE(struct_mtd_oob_buf)

# include <linux/ioctl.h>
# include <mtd/mtd-abi.h>

typedef struct mtd_oob_buf struct_mtd_oob_buf;

#endif /* HAVE_STRUCT_MTD_WRITE_REQ */

#include MPERS_DEFS

#ifdef HAVE_STRUCT_MTD_WRITE_REQ

# include "xlat/mtd_mode_options.h"
# include "xlat/mtd_file_mode_options.h"
# include "xlat/mtd_type_options.h"
# include "xlat/mtd_flags_options.h"
# include "xlat/mtd_otp_options.h"
# include "xlat/mtd_nandecc_options.h"

static void
decode_erase_info_user(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct erase_info_user einfo;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &einfo))
		return;

	tprintf("{start=%#x, length=%#x}", einfo.start, einfo.length);
}

static void
decode_erase_info_user64(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct erase_info_user64 einfo64;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &einfo64))
		return;

	tprintf("{start=%#" PRIx64 ", length=%#" PRIx64 "}",
		(uint64_t) einfo64.start, (uint64_t) einfo64.length);
}

static void
decode_mtd_oob_buf(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct_mtd_oob_buf mbuf;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &mbuf))
		return;

	tprintf("{start=%#x, length=%#x, ptr=", mbuf.start, mbuf.length);
	printaddr(ptr_to_kulong(mbuf.ptr));
	tprints("}");
}

static void
decode_mtd_oob_buf64(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct mtd_oob_buf64 mbuf64;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &mbuf64))
		return;

	tprintf("{start=%#" PRIx64 ", length=%#x, usr_ptr=%#" PRIx64 "}",
		(uint64_t) mbuf64.start, mbuf64.length,
		(uint64_t) mbuf64.usr_ptr);
}

static void
decode_otp_info(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct otp_info oinfo;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &oinfo))
		return;

	tprintf("{start=%#x, length=%#x, locked=%u}",
		oinfo.start, oinfo.length, oinfo.locked);
}

static void
decode_otp_select(struct tcb *const tcp, const kernel_ulong_t addr)
{
	unsigned int i;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &i))
		return;

	tprints("[");
	printxval(mtd_otp_options, i, "MTD_OTP_???");
	tprints("]");
}

static void
decode_mtd_write_req(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct mtd_write_req mreq;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &mreq))
		return;

	tprintf("{start=%#" PRIx64 ", len=%#" PRIx64
		", ooblen=%#" PRIx64 ", usr_data=%#" PRIx64
		", usr_oob=%#" PRIx64 ", mode=",
		(uint64_t) mreq.start, (uint64_t) mreq.len,
		(uint64_t) mreq.ooblen, (uint64_t) mreq.usr_data,
		(uint64_t) mreq.usr_oob);
	printxval(mtd_mode_options, mreq.mode, "MTD_OPS_???");
	tprints("}");
}

static void
decode_mtd_info_user(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct mtd_info_user minfo;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &minfo))
		return;

	tprints("{type=");
	printxval(mtd_type_options, minfo.type, "MTD_???");
	tprints(", flags=");
	printflags(mtd_flags_options, minfo.flags, "MTD_???");
	tprintf(", size=%#x, erasesize=%#x, writesize=%#x, oobsize=%#x"
		", padding=%#" PRIx64 "}",
		minfo.size, minfo.erasesize, minfo.writesize, minfo.oobsize,
		(uint64_t) minfo.padding);
}

static void
decode_nand_oobinfo(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct nand_oobinfo ninfo;
	unsigned int i, j;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &ninfo))
		return;

	tprints("{useecc=");
	printxval(mtd_nandecc_options, ninfo.useecc, "MTD_NANDECC_???");
	tprintf(", eccbytes=%#x", ninfo.eccbytes);

	tprints(", oobfree={");
	for (i = 0; i < ARRAY_SIZE(ninfo.oobfree); ++i) {
		if (i)
			tprints("}, ");
		tprints("{");
		for (j = 0; j < ARRAY_SIZE(ninfo.oobfree[0]); ++j) {
			if (j)
				tprints(", ");
			tprintf("%#x", ninfo.oobfree[i][j]);
		}
	}

	tprints("}}, eccpos={");
	for (i = 0; i < ARRAY_SIZE(ninfo.eccpos); ++i) {
		if (i)
			tprints(", ");
		tprintf("%#x", ninfo.eccpos[i]);
	}

	tprints("}");
}

static void
decode_nand_ecclayout_user(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct nand_ecclayout_user nlay;
	unsigned int i;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &nlay))
		return;

	tprintf("{eccbytes=%#x, eccpos={", nlay.eccbytes);
	for (i = 0; i < ARRAY_SIZE(nlay.eccpos); ++i) {
		if (i)
			tprints(", ");
		tprintf("%#x", nlay.eccpos[i]);
	}
	tprintf("}, oobavail=%#x, oobfree={", nlay.oobavail);
	for (i = 0; i < ARRAY_SIZE(nlay.oobfree); ++i) {
		if (i)
			tprints(", ");
		tprintf("{offset=%#x, length=%#x}",
			nlay.oobfree[i].offset, nlay.oobfree[i].length);
	}
	tprints("}");
}

static void
decode_mtd_ecc_stats(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct mtd_ecc_stats es;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &es))
		return;

	tprintf("{corrected=%#x, failed=%#x, badblocks=%#x, bbtblocks=%#x}",
		es.corrected, es.failed, es.badblocks, es.bbtblocks);
}

MPERS_PRINTER_DECL(int, mtd_ioctl, struct tcb *const tcp,
		   const unsigned int code, const kernel_ulong_t arg)
{
	switch (code) {
	case MEMERASE:
	case MEMLOCK:
	case MEMUNLOCK:
	case MEMISLOCKED:
		decode_erase_info_user(tcp, arg);
		break;

	case MEMERASE64:
		decode_erase_info_user64(tcp, arg);
		break;

	case MEMWRITEOOB:
	case MEMREADOOB:
		decode_mtd_oob_buf(tcp, arg);
		break;

	case MEMWRITEOOB64:
	case MEMREADOOB64:
		decode_mtd_oob_buf64(tcp, arg);
		break;

	case MEMWRITE:
		decode_mtd_write_req(tcp, arg);
		break;

	case OTPGETREGIONINFO:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case OTPLOCK:
		decode_otp_info(tcp, arg);
		break;

	case OTPSELECT:
		decode_otp_select(tcp, arg);
		break;

	case MTDFILEMODE:
		tprints(", ");
		printxval64(mtd_file_mode_options, arg, "MTD_FILE_MODE_???");
		break;

	case MEMGETBADBLOCK:
	case MEMSETBADBLOCK:
		tprints(", ");
		printnum_int64(tcp, arg, "%" PRIu64);
		break;

	case MEMGETINFO:
		if (entering(tcp))
			return 0;
		decode_mtd_info_user(tcp, arg);
		break;

	case MEMGETOOBSEL:
		if (entering(tcp))
			return 0;
		decode_nand_oobinfo(tcp, arg);
		break;

	case ECCGETLAYOUT:
		if (entering(tcp))
			return 0;
		decode_nand_ecclayout_user(tcp, arg);
		break;

	case ECCGETSTATS:
		if (entering(tcp))
			return 0;
		decode_mtd_ecc_stats(tcp, arg);
		break;

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

	case MEMGETREGIONINFO:
		if (entering(tcp)) {
			struct region_info_user rinfo;

			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &rinfo))
				break;
			tprintf("{regionindex=%#x", rinfo.regionindex);
			return 0;
		} else {
			struct region_info_user rinfo;

			if (!syserror(tcp) && !umove(tcp, arg, &rinfo))
				tprintf(", offset=%#x"
					", erasesize=%#x"
					", numblocks=%#x}",
					rinfo.offset,
					rinfo.erasesize,
					rinfo.numblocks);
			tprints("}");
			break;
		}

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}

#endif /* HAVE_STRUCT_MTD_WRITE_REQ */
