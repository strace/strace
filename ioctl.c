/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2001 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <linux/ioctl.h>
#include "xlat/ioctl_dirs.h"

#if defined(SPARC) || defined(SPARC64)
/*
 * While Alpha, MIPS, PA-RISC, and POWER simply define _IOC_SIZEBITS to 13
 * and utilise 3 bits for _IOC_DIRBITS, SPARC tries to provide 14 bits
 * for the size field ("as on i386") by (ab)using the lowest direction bit.
 * Unfortunately, while doing so, they decide to define _IOC_SIZE to 0
 * when the direction doesn't have _IOC_READ/_IOC_WRITE bits set, which
 * breaks the invariant
 *
 *     _IOC_SIZE(_IOC(dir, type, nr, size)) == size
 *
 * for _IOC_DIR(val) that doesn't include _IOC_READ or _IOC_WRITE, which
 * is unacceptable for strace's use case.
 * So, let's redefine _IOC_SIZE in a way that is more suitable for us.
 */
# undef _IOC_SIZE
# define _IOC_SIZE(nr)						\
	((_IOC_DIR(nr) & (_IOC_WRITE | _IOC_READ))		\
		? (((nr) >> _IOC_SIZESHIFT) & _IOC_XSIZEMASK)	\
		: (((nr) >> _IOC_SIZESHIFT) & _IOC_SIZEMASK))	\
	/* end of _IOC_SIZE definition */
#endif

static int
compare(const void *a, const void *b)
{
	const unsigned int code1 = (const uintptr_t) a;
	const unsigned int code2 = ((struct_ioctlent *) b)->code;
	return (code1 > code2) ? 1 : (code1 < code2) ? -1 : 0;
}

static const struct_ioctlent *
ioctl_lookup(const unsigned int code)
{
	struct_ioctlent *iop;

	iop = bsearch((const void *) (const uintptr_t) code, ioctlent,
			nioctlents, sizeof(ioctlent[0]), compare);
	while (iop > ioctlent) {
		iop--;
		if (iop->code != code) {
			iop++;
			break;
		}
	}
	return iop;
}

static const struct_ioctlent *
ioctl_next_match(const struct_ioctlent *iop)
{
	const unsigned int code = iop->code;
	iop++;
	if (iop < ioctlent + nioctlents && iop->code == code)
		return iop;
	return NULL;
}

static void
ioctl_print_code(const unsigned int code)
{
	const bool abbrev = xlat_verbose(xlat_verbosity) != XLAT_STYLE_VERBOSE;

	tprints("_IOC(");
	printflags_ex(_IOC_DIR(code), abbrev ? "_IOC_???" : NULL,
		      abbrev ? XLAT_STYLE_DEFAULT : XLAT_STYLE_ABBREV,
		      ioctl_dirs, NULL);
	tprintf(", %#x, %#x, %#x)",
		_IOC_TYPE(code), _IOC_NR(code), _IOC_SIZE(code));
}

static int
evdev_decode_number(const unsigned int code)
{
	const unsigned int nr = _IOC_NR(code);
	const bool abbrev = xlat_verbose(xlat_verbosity) != XLAT_STYLE_VERBOSE;

	if (_IOC_DIR(code) == _IOC_WRITE) {
		if (nr >= 0xc0 && nr <= 0xc0 + 0x3f) {
			tprints("EVIOCSABS(");
			printxval_ex(evdev_abs, nr - 0xc0,
				     abbrev ? "ABS_???" : NULL,
				     abbrev ? XLAT_STYLE_DEFAULT
					    : XLAT_STYLE_ABBREV);
			tprints(")");
			return 1;
		}
	}

	if (_IOC_DIR(code) != _IOC_READ)
		return 0;

	if (nr >= 0x20 && nr <= 0x20 + 0x1f) {
		tprints("EVIOCGBIT(");
		if (nr == 0x20)
			tprints("0");
		else
			printxval_ex(evdev_ev, nr - 0x20,
				     abbrev ? "EV_???" : NULL,
				     abbrev ? XLAT_STYLE_DEFAULT
					    : XLAT_STYLE_ABBREV);
		tprintf(", %u)", _IOC_SIZE(code));
		return 1;
	} else if (nr >= 0x40 && nr <= 0x40 + 0x3f) {
		tprints("EVIOCGABS(");
		printxval_ex(evdev_abs, nr - 0x40, abbrev ? "ABS_???" : NULL,
			     abbrev ? XLAT_STYLE_DEFAULT : XLAT_STYLE_ABBREV);
		tprints(")");
		return 1;
	}

	switch (nr) {
	case 0x06:
		tprintf("EVIOCGNAME(%u)", _IOC_SIZE(code));
		return 1;
	case 0x07:
		tprintf("EVIOCGPHYS(%u)", _IOC_SIZE(code));
		return 1;
	case 0x08:
		tprintf("EVIOCGUNIQ(%u)", _IOC_SIZE(code));
		return 1;
	case 0x09:
		tprintf("EVIOCGPROP(%u)", _IOC_SIZE(code));
		return 1;
	case 0x0a:
		tprintf("EVIOCGMTSLOTS(%u)", _IOC_SIZE(code));
		return 1;
	case 0x18:
		tprintf("EVIOCGKEY(%u)", _IOC_SIZE(code));
		return 1;
	case 0x19:
		tprintf("EVIOCGLED(%u)", _IOC_SIZE(code));
		return 1;
	case 0x1a:
		tprintf("EVIOCGSND(%u)", _IOC_SIZE(code));
		return 1;
	case 0x1b:
		tprintf("EVIOCGSW(%u)", _IOC_SIZE(code));
		return 1;
	default:
		return 0;
	}
}

static int
hiddev_decode_number(const unsigned int code)
{
	if (_IOC_DIR(code) == _IOC_READ) {
		switch (_IOC_NR(code)) {
		case 0x04:
			tprintf("HIDIOCGRAWNAME(%u)", _IOC_SIZE(code));
			return 1;
		case 0x05:
			tprintf("HIDIOCGRAWPHYS(%u)", _IOC_SIZE(code));
			return 1;
		case 0x06:
			tprintf("HIDIOCSFEATURE(%u)", _IOC_SIZE(code));
			return 1;
		case 0x08:
			tprintf("HIDIOCGRAWUNIQ(%u)", _IOC_SIZE(code));
			return 1;
		case 0x12:
			tprintf("HIDIOCGPHYS(%u)", _IOC_SIZE(code));
			return 1;
		default:
			return 0;
		}
	} else if (_IOC_DIR(code) == (_IOC_READ | _IOC_WRITE)) {
		switch (_IOC_NR(code)) {
		case 0x06:
			tprintf("HIDIOCSFEATURE(%u)", _IOC_SIZE(code));
			return 1;
		case 0x07:
			tprintf("HIDIOCGFEATURE(%u)", _IOC_SIZE(code));
			return 1;
		default:
			return 0;
		}
	}

	return 0;
}

static int
ioctl_decode_command_number(struct tcb *tcp)
{
	const unsigned int code = tcp->u_arg[1];

	switch (_IOC_TYPE(code)) {
	case 'E':
		return evdev_decode_number(code);
	case 'H':
		return hiddev_decode_number(code);
	case 'M':
		if (_IOC_DIR(code) == _IOC_WRITE) {
			tprintf("MIXER_WRITE(%u)", _IOC_NR(code));
			return 1;
		} else if (_IOC_DIR(code) == _IOC_READ) {
			tprintf("MIXER_READ(%u)", _IOC_NR(code));
			return 1;
		}
		return 0;
	case 'U':
		if (_IOC_DIR(code) == _IOC_READ && _IOC_NR(code) == 0x2c) {
			tprintf("UI_GET_SYSNAME(%u)", _IOC_SIZE(code));
			return 1;
		}
		return 0;
	case 'j':
		if (_IOC_DIR(code) == _IOC_READ && _IOC_NR(code) == 0x13) {
			tprintf("JSIOCGNAME(%u)", _IOC_SIZE(code));
			return 1;
		}
		return 0;
	case 'k':
		if (_IOC_DIR(code) == _IOC_WRITE && _IOC_NR(code) == 0) {
			tprintf("SPI_IOC_MESSAGE(%u)", _IOC_SIZE(code));
			return 1;
		}
		return 0;
	default:
		return 0;
	}
}

/**
 * Decode arg parameter of the ioctl call.
 *
 * @return There are two flags of the return value important for the purposes of
 *         processing by SYS_FUNC(ioctl):
 *          - RVAL_IOCTL_DECODED: indicates that ioctl decoder code
 *                                has printed arg parameter;
 *          - RVAL_DECODED: indicates that decoding is done.
 *         As a result, the following behaviour is expected:
 *          - on entering:
 *            - 0: decoding should be continued on exiting;
 *            - RVAL_IOCTL_DECODED: decoding on exiting is not needed
 *                                  and decoder has printed arg value;
 *            - RVAL_DECODED: decoding on exiting is not needed
 *                            and generic handler should print arg value.
 *          - on exiting:
 *            - 0: generic handler should print arg value;
 *            - RVAL_IOCTL_DECODED: decoder has printed arg value.
 *
 *         Note that it makes no sense to return just RVAL_DECODED on exiting,
 *         but, of course, it is not prohibited (for example, it may be useful
 *         in cases where the return path is common on entering and on exiting
 *         the syscall).
 *
 *         SYS_FUNC(ioctl) converts RVAL_IOCTL_DECODED flag to RVAL_DECODED,
 *         and passes all other bits of ioctl_decode return value unchanged.
 */
static int
ioctl_decode(struct tcb *tcp)
{
	const unsigned int code = tcp->u_arg[1];
	const kernel_ulong_t arg = tcp->u_arg[2];

	switch (_IOC_TYPE(code)) {
	case 0x03:
		return hdio_ioctl(tcp, code, arg);
	case 0x12:
		return block_ioctl(tcp, code, arg);
	case '"': /* 0x22 */
		return scsi_ioctl(tcp, code, arg);
	case '$': /* 0x24 */
		return perf_ioctl(tcp, code, arg);
#ifdef HAVE_STRUCT_PTP_SYS_OFFSET
	case '=': /* 0x3d */
		return ptp_ioctl(tcp, code, arg);
#endif
#ifdef HAVE_LINUX_INPUT_H
	case 'E':
		return evdev_ioctl(tcp, code, arg);
#endif
	case 'I':
		return inotify_ioctl(tcp, code, arg);
	case 'L':
		return loop_ioctl(tcp, code, arg);
#ifdef HAVE_STRUCT_MTD_WRITE_REQ
	case 'M':
		return mtd_ioctl(tcp, code, arg);
#endif
#ifdef HAVE_STRUCT_UBI_ATTACH_REQ_MAX_BEB_PER1024
	case 'O':
		return ubi_ioctl(tcp, code, arg);
#endif
	case 'R':
		return random_ioctl(tcp, code, arg);
	case 'T':
		return term_ioctl(tcp, code, arg);
	case 'V':
		return v4l2_ioctl(tcp, code, arg);
	case 'W':
		return watchdog_ioctl(tcp, code, arg);
	case 'X':
		return fs_x_ioctl(tcp, code, arg);
	case 'f': {
#if defined(ALPHA) || defined(POWERPC)
		int ret = file_ioctl(tcp, code, arg);
		if (ret != RVAL_DECODED)
			return ret;
		return term_ioctl(tcp, code, arg);
#else /* !(ALPHA || POWERPC) */
		return file_ioctl(tcp, code, arg);
#endif /* (ALPHA || POWERPC) */
	}
#ifdef HAVE_STRUCT_UBI_ATTACH_REQ_MAX_BEB_PER1024
	case 'o':
		return ubi_ioctl(tcp, code, arg);
#endif
	case 'p':
		return rtc_ioctl(tcp, code, arg);
#if defined(ALPHA) || defined(POWERPC)
	case 't':
		return term_ioctl(tcp, code, arg);
#endif /* !ALPHA */
	case 0x89:
		return sock_ioctl(tcp, code, arg);
#ifdef HAVE_LINUX_BTRFS_H
	case 0x94:
		return btrfs_ioctl(tcp, code, arg);
#endif
	case 0xa4:
		return tee_ioctl(tcp, code, arg);
#ifdef HAVE_LINUX_USERFAULTFD_H
	case 0xaa:
		return uffdio_ioctl(tcp, code, arg);
#endif
	case 0xab:
		return nbd_ioctl(tcp, code, arg);
#ifdef HAVE_LINUX_KVM_H
	case 0xae:
		return kvm_ioctl(tcp, code, arg);
#endif
	case 0xb7:
		return nsfs_ioctl(tcp, code, arg);
#ifdef HAVE_LINUX_DM_IOCTL_H
	case 0xfd:
		return dm_ioctl(tcp, code, arg);
#endif
	default:
		break;
	}
	return 0;
}

SYS_FUNC(ioctl)
{
	const struct_ioctlent *iop;
	int ret;

	if (entering(tcp)) {
		printfd(tcp, tcp->u_arg[0]);
		tprints(", ");

		if (xlat_verbosity != XLAT_STYLE_ABBREV)
			tprintf("%#x", (unsigned int) tcp->u_arg[1]);
		if (xlat_verbosity == XLAT_STYLE_VERBOSE)
			tprints(" /* ");
		if (xlat_verbosity != XLAT_STYLE_RAW) {
			ret = ioctl_decode_command_number(tcp);
			if (!(ret & IOCTL_NUMBER_STOP_LOOKUP)) {
				iop = ioctl_lookup(tcp->u_arg[1]);
				if (iop) {
					if (ret)
						tprints(" or ");
					tprints(iop->symbol);
					while ((iop = ioctl_next_match(iop)))
						tprintf(" or %s", iop->symbol);
				} else if (!ret) {
					ioctl_print_code(tcp->u_arg[1]);
				}
			}
		}
		if (xlat_verbosity == XLAT_STYLE_VERBOSE)
			tprints(" */");

		ret = ioctl_decode(tcp);
	} else {
		ret = ioctl_decode(tcp) | RVAL_DECODED;
	}

	if (ret & RVAL_IOCTL_DECODED) {
		ret &= ~RVAL_IOCTL_DECODED;
		ret |= RVAL_DECODED;
	} else if (ret & RVAL_DECODED) {
		tprintf(", %#" PRI_klx, tcp->u_arg[2]);
	}

	return ret;
}
