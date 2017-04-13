/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2001 Wichert Akkerman <wichert@cistron.nl>
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
#include <linux/ioctl.h>
#include "xlat/ioctl_dirs.h"

#ifdef HAVE_LINUX_INPUT_H
# include <linux/input.h>
#endif

#include "xlat/evdev_abs.h"
#include "xlat/evdev_ev.h"

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
	tprints("_IOC(");
	printflags(ioctl_dirs, _IOC_DIR(code), "_IOC_???");
	tprintf(", %#x, %#x, %#x)",
		_IOC_TYPE(code), _IOC_NR(code), _IOC_SIZE(code));
}

static int
evdev_decode_number(const unsigned int code)
{
	const unsigned int nr = _IOC_NR(code);

	if (_IOC_DIR(code) == _IOC_WRITE) {
		if (nr >= 0xc0 && nr <= 0xc0 + 0x3f) {
			tprints("EVIOCSABS(");
			printxval(evdev_abs, nr - 0xc0, "ABS_???");
			tprints(")");
			return 1;
		}
	}

	if (_IOC_DIR(code) != _IOC_READ)
		return 0;

	if (nr >= 0x20 && nr <= 0x20 + 0x1f) {
		tprints("EVIOCGBIT(");
		printxval(evdev_ev, nr - 0x20, "EV_???");
		tprintf(", %u)", _IOC_SIZE(code));
		return 1;
	} else if (nr >= 0x40 && nr <= 0x40 + 0x3f) {
		tprints("EVIOCGABS(");
		printxval(evdev_abs, nr - 0x40, "ABS_???");
		tprints(")");
		return 1;
	}

	switch (_IOC_NR(nr)) {
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

static int
ioctl_decode(struct tcb *tcp)
{
	const unsigned int code = tcp->u_arg[1];
	const kernel_ulong_t arg = tcp->u_arg[2];

	switch (_IOC_TYPE(code)) {
#if defined(ALPHA) || defined(POWERPC)
	case 'f': {
		int ret = file_ioctl(tcp, code, arg);
		if (ret != RVAL_DECODED)
			return ret;
	}
	case 't':
	case 'T':
		return term_ioctl(tcp, code, arg);
#else /* !ALPHA */
	case 'f':
		return file_ioctl(tcp, code, arg);
	case 0x54:
#endif /* !ALPHA */
		return term_ioctl(tcp, code, arg);
	case 0x89:
		return sock_ioctl(tcp, code, arg);
	case 'p':
		return rtc_ioctl(tcp, code, arg);
	case 0x03:
		return hdio_ioctl(tcp, code, arg);
	case 0x12:
		return block_ioctl(tcp, code, arg);
	case 'X':
		return fs_x_ioctl(tcp, code, arg);
	case 0x22:
		return scsi_ioctl(tcp, code, arg);
	case 'L':
		return loop_ioctl(tcp, code, arg);
	case 'M':
		return mtd_ioctl(tcp, code, arg);
	case 'o':
	case 'O':
		return ubi_ioctl(tcp, code, arg);
	case 'V':
		return v4l2_ioctl(tcp, code, arg);
	case '=':
		return ptp_ioctl(tcp, code, arg);
#ifdef HAVE_LINUX_INPUT_H
	case 'E':
		return evdev_ioctl(tcp, code, arg);
#endif
#ifdef HAVE_LINUX_USERFAULTFD_H
	case 0xaa:
		return uffdio_ioctl(tcp, code, arg);
#endif
#ifdef HAVE_LINUX_BTRFS_H
	case 0x94:
		return btrfs_ioctl(tcp, code, arg);
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
		ret = ioctl_decode(tcp);
	} else {
		ret = ioctl_decode(tcp) | RVAL_DECODED;
	}

	if (ret & RVAL_DECODED) {
		ret &= ~RVAL_DECODED;
		if (ret)
			--ret;
		else
			tprintf(", %#" PRI_klx, tcp->u_arg[2]);
		ret |= RVAL_DECODED;
	} else {
		if (ret)
			--ret;
	}

	return ret;
}
