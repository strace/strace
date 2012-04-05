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

#include <sys/ioctl.h>

/* The mtd api changes quickly, so we have to keep a local copy */
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(3, 3, 0)
# include "mtd-abi.h"
#else
# include <mtd/mtd-abi.h>
#endif

static const struct xlat mtd_mode_options[] = {
	{ MTD_OPS_PLACE_OOB,	"MTD_OPS_PLACE_OOB"	},
	{ MTD_OPS_AUTO_OOB,	"MTD_OPS_AUTO_OOB"	},
	{ MTD_OPS_RAW,		"MTD_OPS_RAW"		},
	{ 0,			NULL			},
};

static const struct xlat mtd_type_options[] = {
	{ MTD_ABSENT,		"MTD_ABSENT"		},
	{ MTD_RAM,		"MTD_RAM"		},
	{ MTD_ROM,		"MTD_ROM"		},
	{ MTD_NORFLASH,		"MTD_NORFLASH"		},
	{ MTD_NANDFLASH,	"MTD_NANDFLASH"		},
	{ MTD_DATAFLASH,	"MTD_DATAFLASH"		},
	{ MTD_UBIVOLUME,	"MTD_UBIVOLUME"		},
	{ MTD_MLCNANDFLASH,	"MTD_MLCNANDFLASH"	},
	{ 0,			NULL			},
};

static const struct xlat mtd_flags_options[] = {
	{ MTD_WRITEABLE,	"MTD_WRITEABLE"		},
	{ MTD_BIT_WRITEABLE,	"MTD_BIT_WRITEABLE"	},
	{ MTD_NO_ERASE,		"MTD_NO_ERASE"		},
	{ MTD_POWERUP_LOCK,	"MTD_POWERUP_LOCK"	},
	{ 0,			NULL			},
};

static const struct xlat mtd_otp_options[] = {
	{ MTD_OTP_OFF,		"MTD_OTP_OFF"		},
	{ MTD_OTP_FACTORY,	"MTD_OTP_FACTORY"	},
	{ MTD_OTP_USER,		"MTD_OTP_USER"		},
	{ 0,			NULL			},
};

static const struct xlat mtd_nandecc_options[] = {
	{ MTD_NANDECC_OFF,		"MTD_NANDECC_OFF"		},
	{ MTD_NANDECC_PLACE,		"MTD_NANDECC_PLACE"		},
	{ MTD_NANDECC_AUTOPLACE,	"MTD_NANDECC_AUTOPLACE"		},
	{ MTD_NANDECC_PLACEONLY,	"MTD_NANDECC_PLACEONLY"		},
	{ MTD_NANDECC_AUTOPL_USR,	"MTD_NANDECC_AUTOPL_USR"	},
	{ 0,				NULL				},
};

int mtd_ioctl(struct tcb *tcp, long code, long arg)
{
	struct mtd_info_user minfo;
	struct erase_info_user einfo;
	struct erase_info_user64 einfo64;
	struct mtd_oob_buf mbuf;
	struct mtd_oob_buf64 mbuf64;
	struct region_info_user rinfo;
	struct otp_info oinfo;
	struct mtd_ecc_stats estat;
	struct mtd_write_req mreq;
	struct nand_oobinfo ninfo;
	struct nand_ecclayout_user nlay;
	int i, j;

	if (entering(tcp))
		return 0;

	switch (code) {

	case MEMGETINFO:
		if (!verbose(tcp) || umove(tcp, arg, &minfo) < 0)
			return 0;

		tprints(", {type=");
		printxval(mtd_type_options, minfo.type, "MTD_???");
		tprints(", flags=");
		printflags(mtd_flags_options, minfo.flags, "MTD_???");
		tprintf(", size=%#" PRIx32 ", erasesize=%#" PRIx32,
			minfo.size, minfo.erasesize);
		tprintf(", writesize=%#" PRIx32 ", oobsize=%#" PRIx32,
			minfo.writesize, minfo.oobsize);
		tprintf(", padding=%#" PRIx64 "}",
			(uint64_t) minfo.padding);
		return 1;

	case MEMERASE:
	case MEMLOCK:
	case MEMUNLOCK:
	case MEMISLOCKED:
		if (!verbose(tcp) || umove(tcp, arg, &einfo) < 0)
			return 0;

		tprintf(", {start=%#" PRIx32 ", length=%#" PRIx32 "}",
			einfo.start, einfo.length);
		return 1;

	case MEMERASE64:
		if (!verbose(tcp) || umove(tcp, arg, &einfo64) < 0)
			return 0;

		tprintf(", {start=%#" PRIx64 ", length=%#" PRIx64 "}",
			(uint64_t) einfo64.start, (uint64_t) einfo64.length);
		return 1;

	case MEMWRITEOOB:
	case MEMREADOOB:
		if (!verbose(tcp) || umove(tcp, arg, &mbuf) < 0)
			return 0;

		tprintf(", {start=%#" PRIx32 ", length=%#" PRIx32 ", ptr=...}",
			mbuf.start, mbuf.length);
		return 1;

	case MEMWRITEOOB64:
	case MEMREADOOB64:
		if (!verbose(tcp) || umove(tcp, arg, &mbuf64) < 0)
			return 0;

		tprintf(", {start=%#" PRIx64 ", length=%#" PRIx64 ", ptr=...}",
			(uint64_t) mbuf64.start, (uint64_t) mbuf64.length);
		return 1;

	case MEMGETREGIONINFO:
		if (!verbose(tcp) || umove(tcp, arg, &rinfo) < 0)
			return 0;

		tprintf(", {offset=%#" PRIx32 ", erasesize=%#" PRIx32,
			rinfo.offset, rinfo.erasesize);
		tprintf(", numblocks=%#" PRIx32 ", regionindex=%#" PRIx32 "}",
			rinfo.numblocks, rinfo.regionindex);
		return 1;

	case MEMGETOOBSEL:
		if (!verbose(tcp) || umove(tcp, arg, &ninfo) < 0)
			return 0;

		tprints(", {useecc=");
		printxval(mtd_nandecc_options, ninfo.useecc, "MTD_NANDECC_???");
		tprintf(", eccbytes=%#" PRIx32, ninfo.eccbytes);

		tprints(", oobfree={");
		for (i = 0; i < ARRAY_SIZE(ninfo.oobfree); ++i) {
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
		return 1;

	case OTPGETREGIONINFO:
	case OTPLOCK:
		if (!verbose(tcp) || umove(tcp, arg, &oinfo) < 0)
			return 0;

		tprintf(", {start=%#" PRIx32 ", length=%#" PRIx32 ", locked=%" PRIu32 "}",
			oinfo.start, oinfo.length, oinfo.locked);
		return 1;

	case ECCGETLAYOUT:
		if (!verbose(tcp) || umove(tcp, arg, &nlay) < 0)
			return 0;

		tprintf(", {eccbytes=%#" PRIx32 ", eccpos={", nlay.eccbytes);
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
		return 1;

	case ECCGETSTATS:
		if (!verbose(tcp) || umove(tcp, arg, &estat) < 0)
			return 0;

		tprintf(", {corrected=%#" PRIx32 ", failed=%#" PRIx32,
			estat.corrected, estat.failed);
		tprintf(", badblocks=%#" PRIx32 ", bbtblocks=%#" PRIx32 "}",
			estat.badblocks, estat.bbtblocks);
		return 1;

	case MEMWRITE:
		if (!verbose(tcp) || umove(tcp, arg, &mreq) < 0)
			return 0;

		tprintf(", {start=%#" PRIx64 ", len=%#" PRIx64,
			(uint64_t) mreq.start, (uint64_t) mreq.len);
		tprintf(", ooblen=%#" PRIx64 ", usr_data=%#" PRIx64,
			(uint64_t) mreq.ooblen, (uint64_t) mreq.usr_data);
		tprintf(", usr_oob=%#" PRIx64 ", mode=",
			(uint64_t) mreq.usr_oob);
		printxval(mtd_mode_options, mreq.mode, "MTD_OPS_???");
		tprints(", padding=...}");
		return 1;

	case OTPSELECT:
		if (!verbose(tcp) || umove(tcp, arg, &i) < 0)
			return 0;

		tprints(", [");
		printxval(mtd_otp_options, i, "MTD_OTP_???");
		tprints("]");
		return 1;

	case MEMGETBADBLOCK:
	case MEMSETBADBLOCK:
		if (!verbose(tcp))
			return 0;

		tprints(", ");
		print_loff_t(tcp, arg);
		return 1;

	case OTPGETREGIONCOUNT:
		if (!verbose(tcp) || umove(tcp, arg, &i) < 0)
			return 0;

		tprintf(", [%i]", i);
		return 1;

	case MTDFILEMODE:
		/* XXX: process return value as enum mtd_file_modes */

	case MEMGETREGIONCOUNT:
		/* These ones take simple args, so let default printer handle it */

	default:
		return 0;
	}
}
