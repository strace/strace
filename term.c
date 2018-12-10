/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
/*
 * The C library's definition of struct termios might differ from
 * the kernel one, and we need to use the kernel layout.
 */
#include <linux/termios.h>

#include "xlat/tcxonc_options.h"
#include "xlat/tcflsh_options.h"
#include "xlat/baud_options.h"
#include "xlat/modem_flags.h"

static void
decode_termios(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct termios tios;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &tios))
		return;
	if (abbrev(tcp)) {
		tprints("{");
		printxval(baud_options, tios.c_cflag & CBAUD, "B???");
		tprintf(" %sopost %sisig %sicanon %secho ...}",
			(tios.c_oflag & OPOST) ? "" : "-",
			(tios.c_lflag & ISIG) ? "" : "-",
			(tios.c_lflag & ICANON) ? "" : "-",
			(tios.c_lflag & ECHO) ? "" : "-");
		return;
	}
	tprintf("{c_iflags=%#lx, c_oflags=%#lx, ",
		(long) tios.c_iflag, (long) tios.c_oflag);
	tprintf("c_cflags=%#lx, c_lflags=%#lx, ",
		(long) tios.c_cflag, (long) tios.c_lflag);
	tprintf("c_line=%u, ", tios.c_line);
	if (!(tios.c_lflag & ICANON))
		tprintf("c_cc[VMIN]=%d, c_cc[VTIME]=%d, ",
			tios.c_cc[VMIN], tios.c_cc[VTIME]);
	tprints("c_cc=");
	print_quoted_string((char *) tios.c_cc, NCCS, QUOTE_FORCE_HEX);
	tprints("}");
}

static void
decode_termio(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct termio tio;
	int i;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &tio))
		return;
	if (abbrev(tcp)) {
		tprints("{");
		printxval(baud_options, tio.c_cflag & CBAUD, "B???");
		tprintf(" %sopost %sisig %sicanon %secho ...}",
			(tio.c_oflag & OPOST) ? "" : "-",
			(tio.c_lflag & ISIG) ? "" : "-",
			(tio.c_lflag & ICANON) ? "" : "-",
			(tio.c_lflag & ECHO) ? "" : "-");
		return;
	}
	tprintf("{c_iflags=%#lx, c_oflags=%#lx, ",
		(long) tio.c_iflag, (long) tio.c_oflag);
	tprintf("c_cflags=%#lx, c_lflags=%#lx, ",
		(long) tio.c_cflag, (long) tio.c_lflag);
	tprintf("c_line=%u, ", tio.c_line);
#ifdef _VMIN
	if (!(tio.c_lflag & ICANON))
		tprintf("c_cc[_VMIN]=%d, c_cc[_VTIME]=%d, ",
			tio.c_cc[_VMIN], tio.c_cc[_VTIME]);
#else /* !_VMIN */
	if (!(tio.c_lflag & ICANON))
		tprintf("c_cc[VMIN]=%d, c_cc[VTIME]=%d, ",
			tio.c_cc[VMIN], tio.c_cc[VTIME]);
#endif /* !_VMIN */
	tprints("c_cc=\"");
	for (i = 0; i < NCC; i++)
		tprintf("\\x%02x", tio.c_cc[i]);
	tprints("\"}");
}

static void
decode_winsize(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct winsize ws;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &ws))
		return;
	tprintf("{ws_row=%d, ws_col=%d, ws_xpixel=%d, ws_ypixel=%d}",
		ws.ws_row, ws.ws_col, ws.ws_xpixel, ws.ws_ypixel);
}

#ifdef TIOCGSIZE
static void
decode_ttysize(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct ttysize ts;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &ts))
		return;
	tprintf("{ts_lines=%d, ts_cols=%d}",
		ts.ts_lines, ts.ts_cols);
}
#endif

static void
decode_modem_flags(struct tcb *const tcp, const kernel_ulong_t addr)
{
	int i;

	tprints(", ");
	if (umove_or_printaddr(tcp, addr, &i))
		return;
	tprints("[");
	printflags(modem_flags, i, "TIOCM_???");
	tprints("]");
}

int
term_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	switch (code) {
	/* struct termios */
	case TCGETS:
#ifdef TCGETS2
	case TCGETS2:
#endif
	case TIOCGLCKTRMIOS:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TCSETS:
#ifdef TCSETS2
	case TCSETS2:
#endif
	case TCSETSW:
#ifdef TCSETSW2
	case TCSETSW2:
#endif
	case TCSETSF:
#ifdef TCSETSF2
	case TCSETSF2:
#endif
	case TIOCSLCKTRMIOS:
		decode_termios(tcp, arg);
		break;

	/* struct termio */
	case TCGETA:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TCSETA:
	case TCSETAW:
	case TCSETAF:
		decode_termio(tcp, arg);
		break;

	/* struct winsize */
	case TIOCGWINSZ:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TIOCSWINSZ:
		decode_winsize(tcp, arg);
		break;

	/* struct ttysize */
#ifdef TIOCGSIZE
	case TIOCGSIZE:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TIOCSSIZE:
		decode_ttysize(tcp, arg);
		break;
#endif

	/* ioctls with a direct decodable arg */
	case TCXONC:
		tprints(", ");
		printxval64(tcxonc_options, arg, "TC???");
		break;
	case TCFLSH:
		tprints(", ");
		printxval64(tcflsh_options, arg, "TC???");
		break;
	case TCSBRK:
	case TCSBRKP:
	case TIOCSCTTY:
		tprintf(", %d", (int) arg);
		break;

	/* ioctls with an indirect parameter displayed as modem flags */
	case TIOCMGET:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TIOCMBIS:
	case TIOCMBIC:
	case TIOCMSET:
		decode_modem_flags(tcp, arg);
		break;

	/* ioctls with an indirect parameter displayed in decimal */
	case TIOCGPGRP:
	case TIOCGSID:
	case TIOCGETD:
	case TIOCGSOFTCAR:
	case TIOCGPTN:
	case FIONREAD:
	case TIOCOUTQ:
#ifdef TIOCGEXCL
	case TIOCGEXCL:
#endif
#ifdef TIOCGDEV
	case TIOCGDEV:
#endif
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TIOCSPGRP:
	case TIOCSETD:
	case FIONBIO:
	case FIOASYNC:
	case TIOCPKT:
	case TIOCSSOFTCAR:
	case TIOCSPTLCK:
		tprints(", ");
		printnum_int(tcp, arg, "%d");
		break;

	/* ioctls with an indirect parameter displayed as a char */
	case TIOCSTI:
		tprints(", ");
		printstrn(tcp, arg, 1);
		break;

	/* ioctls with no parameters */

	case TIOCSBRK:
	case TIOCCBRK:
	case TIOCCONS:
	case TIOCNOTTY:
	case TIOCEXCL:
	case TIOCNXCL:
	case FIOCLEX:
	case FIONCLEX:
#ifdef TIOCVHANGUP
	case TIOCVHANGUP:
#endif
#ifdef TIOCSSERIAL
	case TIOCSSERIAL:
#endif
		break;

	/* ioctls which are unknown */

	default:
		return RVAL_DECODED;
	}

	return RVAL_IOCTL_DECODED;
}
