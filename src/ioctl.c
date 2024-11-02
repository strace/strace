/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2001 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999-2024 The strace developers.
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

	tprints_arg_begin("_IOC");

	tprint_flags_begin();
	printflags_ex(_IOC_DIR(code), abbrev ? "_IOC_???" : "",
		      abbrev ? XLAT_STYLE_DEFAULT : XLAT_STYLE_ABBREV,
		      ioctl_dirs, NULL);
	tprint_flags_end();
	tprint_arg_next();

	PRINT_VAL_X(_IOC_TYPE(code));
	tprint_arg_next();

	PRINT_VAL_X(_IOC_NR(code));
	tprint_arg_next();

	PRINT_VAL_X(_IOC_SIZE(code));
	tprint_arg_end();
}

static int
evdev_decode_number(const unsigned int code)
{
	const unsigned int nr = _IOC_NR(code);
	const bool abbrev = xlat_verbose(xlat_verbosity) != XLAT_STYLE_VERBOSE;

	if (_IOC_DIR(code) == _IOC_WRITE) {
		if (nr >= 0xc0 && nr <= 0xc0 + 0x3f) {
			tprints_arg_begin("EVIOCSABS");
			printxval_ex(evdev_abs, nr - 0xc0,
				     abbrev ? "ABS_???" : "",
				     abbrev ? XLAT_STYLE_DEFAULT
					    : XLAT_STYLE_ABBREV);
			tprint_arg_end();
			return 1;
		}
	}

	if (_IOC_DIR(code) != _IOC_READ)
		return 0;

	if (nr >= 0x20 && nr <= 0x20 + 0x1f) {
		tprints_arg_begin("EVIOCGBIT");
		if (nr == 0x20)
			PRINT_VAL_U(0);
		else
			printxval_ex(evdev_ev, nr - 0x20,
				     abbrev ? "EV_???" : "",
				     abbrev ? XLAT_STYLE_DEFAULT
					    : XLAT_STYLE_ABBREV);
		tprint_arg_next();
		PRINT_VAL_U(_IOC_SIZE(code));
		tprint_arg_end();
		return 1;
	} else if (nr >= 0x40 && nr <= 0x40 + 0x3f) {
		tprints_arg_begin("EVIOCGABS");
		printxval_ex(evdev_abs, nr - 0x40, abbrev ? "ABS_???" : "",
			     abbrev ? XLAT_STYLE_DEFAULT : XLAT_STYLE_ABBREV);
		tprint_arg_end();
		return 1;
	}

	switch (nr) {
	case 0x06:
		tprints_arg_begin("EVIOCGNAME");
		PRINT_VAL_U(_IOC_SIZE(code));
		tprint_arg_end();
		return 1;
	case 0x07:
		tprints_arg_begin("EVIOCGPHYS");
		PRINT_VAL_U(_IOC_SIZE(code));
		tprint_arg_end();
		return 1;
	case 0x08:
		tprints_arg_begin("EVIOCGUNIQ");
		PRINT_VAL_U(_IOC_SIZE(code));
		tprint_arg_end();
		return 1;
	case 0x09:
		tprints_arg_begin("EVIOCGPROP");
		PRINT_VAL_U(_IOC_SIZE(code));
		tprint_arg_end();
		return 1;
	case 0x0a:
		tprints_arg_begin("EVIOCGMTSLOTS");
		PRINT_VAL_U(_IOC_SIZE(code));
		tprint_arg_end();
		return 1;
	case 0x18:
		tprints_arg_begin("EVIOCGKEY");
		PRINT_VAL_U(_IOC_SIZE(code));
		tprint_arg_end();
		return 1;
	case 0x19:
		tprints_arg_begin("EVIOCGLED");
		PRINT_VAL_U(_IOC_SIZE(code));
		tprint_arg_end();
		return 1;
	case 0x1a:
		tprints_arg_begin("EVIOCGSND");
		PRINT_VAL_U(_IOC_SIZE(code));
		tprint_arg_end();
		return 1;
	case 0x1b:
		tprints_arg_begin("EVIOCGSW");
		PRINT_VAL_U(_IOC_SIZE(code));
		tprint_arg_end();
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
			tprints_arg_begin("HIDIOCGRAWNAME");
			PRINT_VAL_U(_IOC_SIZE(code));
			tprint_arg_end();
			return 1;
		case 0x05:
			tprints_arg_begin("HIDIOCGRAWPHYS");
			PRINT_VAL_U(_IOC_SIZE(code));
			tprint_arg_end();
			return 1;
		case 0x06:
			tprints_arg_begin("HIDIOCSFEATURE");
			PRINT_VAL_U(_IOC_SIZE(code));
			tprint_arg_end();
			return 1;
		case 0x08:
			tprints_arg_begin("HIDIOCGRAWUNIQ");
			PRINT_VAL_U(_IOC_SIZE(code));
			tprint_arg_end();
			return 1;
		case 0x12:
			tprints_arg_begin("HIDIOCGPHYS");
			PRINT_VAL_U(_IOC_SIZE(code));
			tprint_arg_end();
			return 1;
		default:
			return 0;
		}
	} else if (_IOC_DIR(code) == (_IOC_READ | _IOC_WRITE)) {
		switch (_IOC_NR(code)) {
		case 0x06:
			tprints_arg_begin("HIDIOCSFEATURE");
			PRINT_VAL_U(_IOC_SIZE(code));
			tprint_arg_end();
			return 1;
		case 0x07:
			tprints_arg_begin("HIDIOCGFEATURE");
			PRINT_VAL_U(_IOC_SIZE(code));
			tprint_arg_end();
			return 1;
		default:
			return 0;
		}
	}

	return 0;
}

static int
ioctl_decode_command_number(struct tcb *tcp, const struct finfo *finfo)
{
	const unsigned int code = tcp->u_arg[1];

	switch (_IOC_TYPE(code)) {
	case '!': /* 0x21 */
		if (code == _IOC(_IOC_READ, '!', 2, sizeof(uint64_t))) {
			tprints_string("SECCOMP_IOCTL_NOTIF_ID_VALID_WRONG_DIR");
			return 1;
		}
		return 0;
	case 'E':
		return evdev_decode_number(code);
	case 'H':
		return hiddev_decode_number(code);
	case 'M':
		if (_IOC_DIR(code) == _IOC_WRITE) {
			tprints_arg_begin("MIXER_WRITE");
			PRINT_VAL_U(_IOC_NR(code));
			tprint_arg_end();
			return 1;
		} else if (_IOC_DIR(code) == _IOC_READ) {
			tprints_arg_begin("MIXER_READ");
			PRINT_VAL_U(_IOC_NR(code));
			tprint_arg_end();
			return 1;
		}
		return 0;
	case 'T':
		return term_ioctl_decode_command_number(tcp, finfo, code);
	case 'U':
		if (_IOC_DIR(code) == _IOC_READ && _IOC_NR(code) == 0x2c) {
			tprints_arg_begin("UI_GET_SYSNAME");
			PRINT_VAL_U(_IOC_SIZE(code));
			tprint_arg_end();
			return 1;
		}
		return 0;
	case 'j':
		if (_IOC_DIR(code) == _IOC_READ && _IOC_NR(code) == 0x13) {
			tprints_arg_begin("JSIOCGNAME");
			PRINT_VAL_U(_IOC_SIZE(code));
			tprint_arg_end();
			return 1;
		}
		return 0;
	case 'k':
		if (_IOC_DIR(code) == _IOC_WRITE && _IOC_NR(code) == 0) {
			tprints_arg_begin("SPI_IOC_MESSAGE");
			PRINT_VAL_U(_IOC_SIZE(code));
			tprint_arg_end();
			return 1;
		}
		return 0;
	default:
		return 0;
	}
}

static int
f_ioctl(struct tcb *tcp, const unsigned int code, const kernel_ulong_t arg)
{
	int rc = fs_f_ioctl(tcp, code, arg);
#if defined ALPHA
	if (rc == RVAL_DECODED)
		rc = sock_ioctl(tcp, code, arg);
	if (rc == RVAL_DECODED)
		rc = term_ioctl(tcp, code, arg);
#elif defined MIPS || defined SH || defined XTENSA
	if (rc == RVAL_DECODED)
		rc = sock_ioctl(tcp, code, arg);
#elif defined POWERPC
	if (rc == RVAL_DECODED)
		rc = term_ioctl(tcp, code, arg);
#endif
	return rc;
}

/**
 * Decode arg parameter for unknown ioctl types. */
static int
ioctl_decode_unknown_type(struct tcb *const tcp, const unsigned int code,
                          const kernel_ulong_t arg)
{
	bool print_before = false, print_after = false;
	int ret = 0;

	if (abbrev(tcp) || (_IOC_SIZE(code) == 0) || (arg == 0)) {
		/* Let the generic handler print arg value.  */
		return 0;
	}

	switch (_IOC_DIR(code)) {
	case _IOC_WRITE:
		print_before = true;
		break;
	case _IOC_READ:
		print_after = true;
		break;
	case _IOC_NONE:
	case _IOC_READ | _IOC_WRITE:
	default:
		print_before = true;
		print_after = true;
		break;
	}

	if (entering(tcp)) {
		tprint_arg_next();
		if (print_before)
			ret = printstr_ex(tcp, arg, _IOC_SIZE(code),
			                  QUOTE_FORCE_HEX);

		if (!print_after || ret)
			ret = RVAL_IOCTL_DECODED;
	} else {
		if (print_before && print_after)
			tprint_value_changed();

		if (print_after)
			printstr_ex(tcp, arg, _IOC_SIZE(code), QUOTE_FORCE_HEX);

		ret = RVAL_IOCTL_DECODED;
	}

	return ret;
}

/**
 * Decode arg parameter of the ioctl call.
 *
 * @param finfo The target file descriptor related information.
 *              finfo is NULL when
 *              - ioctl_decode() is called in leaving stages, or
 *              - the file descriptor is not valid (e.g. -1).
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
ioctl_decode(struct tcb *tcp, const struct finfo *finfo)
{
	const unsigned int code = tcp->u_arg[1];
	const kernel_ulong_t arg = tcp->u_arg[2];

	switch (_IOC_TYPE(code)) {
	case 0x03:
		return hdio_ioctl(tcp, code, arg);
	case 0x12:
		return block_ioctl(tcp, code, arg);
	case '!': /* 0x21 */
		return seccomp_ioctl(tcp, code, arg);
	case '"': /* 0x22 */
		return scsi_ioctl(tcp, code, arg);
	case '$': /* 0x24 */
		return perf_ioctl(tcp, code, arg);
	case '=': /* 0x3d */
		return ptp_ioctl(tcp, code, arg);
	case '>': /* 0x3e */
		return counter_ioctl(tcp, code, arg);
	case 'E':
		return evdev_ioctl(tcp, code, arg);
	case 'I':
		return inotify_ioctl(tcp, code, arg);
	case 'K':
		return kd_ioctl(tcp, code, arg);
	case 'L':
		return loop_ioctl(tcp, code, arg);
	case 'M':
		return mtd_ioctl(tcp, code, arg);
	case 'O':
		return ubi_ioctl(tcp, code, arg);
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
	case 'f':
		return f_ioctl(tcp, code, arg);
	case 'i':
		return lirc_ioctl(tcp, code, arg);
	case 'o':
		return ubi_ioctl(tcp, code, arg);
	case 'p':
		return rtc_ioctl(tcp, code, arg);
#if defined ALPHA || defined MIPS || defined SH || defined XTENSA
	case 's':
		return sock_ioctl(tcp, code, arg);
#endif
#if defined(ALPHA) || defined(MIPS) || defined(POWERPC) \
 || defined(SPARC) || defined(SPARC64)
	case 't':
		return term_ioctl(tcp, code, arg);
#endif
	case 0x89:
		return sock_ioctl(tcp, code, arg);
	case 0x8a:
		return epoll_ioctl(tcp, code, arg);
	case 0x94:
		return fs_0x94_ioctl(tcp, code, arg);
	case 0xa4:
		return tee_ioctl(tcp, code, arg);
	case 0xaa:
		return uffdio_ioctl(tcp, code, arg);
	case 0xab:
		return nbd_ioctl(tcp, code, arg);
#ifdef HAVE_LINUX_KVM_H
	case 0xae:
		return kvm_ioctl(tcp, code, arg);
#endif
	case 0xb4:
		return gpio_ioctl(tcp, code, arg);
	case 0xb7:
		return nsfs_ioctl(tcp, code, arg);
	case 0xfd:
		return dm_ioctl(tcp, code, arg);
	default:
		return ioctl_decode_unknown_type(tcp, code, arg);
	}
	return 0;
}

/*
 * Return true if the specified ioctl command may overlap.
 */
static bool
ioctl_command_overlaps(unsigned int code)
{
	/* see <asm-generic/ioctls.h> and <linux/soundcard.h> */
	return (0x5401 <= code && code <= 0x5408);
}

SYS_FUNC(ioctl)
{
	const struct_ioctlent *iop;
	int ret;

	if (entering(tcp)) {
		struct finfo finfoa;
		struct finfo *finfo = NULL;
		char path[PATH_MAX + 1];
		bool deleted;
		if (ioctl_command_overlaps(tcp->u_arg[1]) &&
		    getfdpath_pid(tcp->pid, tcp->u_arg[0], path, sizeof(path),
				  &deleted) >= 0) {
			finfo = get_finfo_for_dev(tcp->pid, tcp->u_arg[0], path, &finfoa);
			finfo->deleted = deleted;
			printfd_with_finfo(tcp, tcp->u_arg[0], finfo);
		} else
			printfd(tcp, tcp->u_arg[0]);

		tprint_arg_next();

		if (xlat_verbosity != XLAT_STYLE_ABBREV)
			PRINT_VAL_X((unsigned int) tcp->u_arg[1]);
		if (xlat_verbosity == XLAT_STYLE_VERBOSE)
			tprint_comment_begin();
		if (xlat_verbosity != XLAT_STYLE_RAW) {
			ret = ioctl_decode_command_number(tcp, finfo);
			if (!(ret & IOCTL_NUMBER_STOP_LOOKUP)) {
				iop = ioctl_lookup(tcp->u_arg[1]);
				if (iop) {
					if (ret)
						tprint_alternative_value();
					tprints_string(iop->symbol);
					while ((iop = ioctl_next_match(iop))) {
						tprint_alternative_value();
						tprints_string(iop->symbol);
					}
				} else if (!ret) {
					ioctl_print_code(tcp->u_arg[1]);
				}
			}
		}
		if (xlat_verbosity == XLAT_STYLE_VERBOSE)
			tprint_comment_end();

		ret = ioctl_decode(tcp, finfo);
	} else {
		ret = ioctl_decode(tcp, NULL) | RVAL_DECODED;
	}

	if (ret & RVAL_IOCTL_DECODED) {
		ret &= ~RVAL_IOCTL_DECODED;
		ret |= RVAL_DECODED;
	} else if (ret & RVAL_DECODED) {
		tprint_arg_next();
		PRINT_VAL_X(tcp->u_arg[2]);
	}

	return ret;
}
