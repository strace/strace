/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
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
/*
 * The C library's definition of struct termios might differ from
 * the kernel one, and we need to use the kernel layout.
 */
#include <linux/termios.h>
#ifdef HAVE_SYS_FILIO_H
# include <sys/filio.h>
#endif

#include "xlat/tcxonc_options.h"

#ifdef TCLFLSH
#include "xlat/tcflsh_options.h"
#endif

#include "xlat/baud_options.h"
#include "xlat/modem_flags.h"

int term_ioctl(struct tcb *tcp, long code, long arg)
{
	struct termios tios;
	struct termio tio;
	struct winsize ws;
#ifdef TIOCGSIZE
	struct  ttysize ts;
#endif
	int i;

	if (entering(tcp))
		return 0;

	switch (code) {

	/* ioctls with termios or termio args */

#ifdef TCGETS
	case TCGETS:
		if (syserror(tcp))
			return 0;
	case TCSETS:
	case TCSETSW:
	case TCSETSF:
		if (!verbose(tcp) || umove(tcp, arg, &tios) < 0)
			return 0;
		if (abbrev(tcp)) {
			tprints(", {");
			printxval(baud_options, tios.c_cflag & CBAUD, "B???");
			tprintf(" %sopost %sisig %sicanon %secho ...}",
				(tios.c_oflag & OPOST) ? "" : "-",
				(tios.c_lflag & ISIG) ? "" : "-",
				(tios.c_lflag & ICANON) ? "" : "-",
				(tios.c_lflag & ECHO) ? "" : "-");
			return 1;
		}
		tprintf(", {c_iflags=%#lx, c_oflags=%#lx, ",
			(long) tios.c_iflag, (long) tios.c_oflag);
		tprintf("c_cflags=%#lx, c_lflags=%#lx, ",
			(long) tios.c_cflag, (long) tios.c_lflag);
		tprintf("c_line=%u, ", tios.c_line);
		if (!(tios.c_lflag & ICANON))
			tprintf("c_cc[VMIN]=%d, c_cc[VTIME]=%d, ",
				tios.c_cc[VMIN], tios.c_cc[VTIME]);
		tprintf("c_cc=\"");
		for (i = 0; i < NCCS; i++)
			tprintf("\\x%02x", tios.c_cc[i]);
		tprintf("\"}");
		return 1;
#endif /* TCGETS */

#ifdef TCGETA
	case TCGETA:
		if (syserror(tcp))
			return 0;
	case TCSETA:
	case TCSETAW:
	case TCSETAF:
		if (!verbose(tcp) || umove(tcp, arg, &tio) < 0)
			return 0;
		if (abbrev(tcp)) {
			tprints(", {");
			printxval(baud_options, tio.c_cflag & CBAUD, "B???");
			tprintf(" %sopost %sisig %sicanon %secho ...}",
				(tio.c_oflag & OPOST) ? "" : "-",
				(tio.c_lflag & ISIG) ? "" : "-",
				(tio.c_lflag & ICANON) ? "" : "-",
				(tio.c_lflag & ECHO) ? "" : "-");
			return 1;
		}
		tprintf(", {c_iflags=%#lx, c_oflags=%#lx, ",
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
		tprintf("c_cc=\"");
		for (i = 0; i < NCC; i++)
			tprintf("\\x%02x", tio.c_cc[i]);
		tprintf("\"}");
		return 1;
#endif /* TCGETA */

	/* ioctls with winsize or ttysize args */

#ifdef TIOCGWINSZ
	case TIOCGWINSZ:
		if (syserror(tcp))
			return 0;
	case TIOCSWINSZ:
		if (!verbose(tcp) || umove(tcp, arg, &ws) < 0)
			return 0;
		tprintf(", {ws_row=%d, ws_col=%d, ws_xpixel=%d, ws_ypixel=%d}",
			ws.ws_row, ws.ws_col, ws.ws_xpixel, ws.ws_ypixel);
		return 1;
#endif /* TIOCGWINSZ */

#ifdef TIOCGSIZE
	case TIOCGSIZE:
		if (syserror(tcp))
			return 0;
	case TIOCSSIZE:
		if (!verbose(tcp) || umove(tcp, arg, &ts) < 0)
			return 0;
		tprintf(", {ts_lines=%d, ts_cols=%d}",
			ts.ts_lines, ts.ts_cols);
		return 1;
#endif

	/* ioctls with a direct decodable arg */
#ifdef TCXONC
	case TCXONC:
		tprints(", ");
		printxval(tcxonc_options, arg, "TC???");
		return 1;
#endif
#ifdef TCLFLSH
	case TCFLSH:
		tprints(", ");
		printxval(tcflsh_options, arg, "TC???");
		return 1;
#endif
#ifdef TIOCSCTTY
	case TIOCSCTTY:
		tprintf(", %ld", arg);
		return 1;
#endif

	/* ioctls with an indirect parameter displayed as modem flags */

#ifdef TIOCMGET
	case TIOCMGET:
	case TIOCMBIS:
	case TIOCMBIC:
	case TIOCMSET:
		if (umove(tcp, arg, &i) < 0)
			return 0;
		tprints(", [");
		printflags(modem_flags, i, "TIOCM_???");
		tprints("]");
		return 1;
#endif /* TIOCMGET */

	/* ioctls with an indirect parameter displayed in decimal */

	case TIOCSPGRP:
	case TIOCGPGRP:
#ifdef TIOCGETPGRP
	case TIOCGETPGRP:
#endif
#ifdef TIOCSETPGRP
	case TIOCSETPGRP:
#endif
#ifdef FIONREAD
	case FIONREAD:
#endif
	case TIOCOUTQ:
#ifdef FIONBIO
	case FIONBIO:
#endif
#ifdef FIOASYNC
	case FIOASYNC:
#endif
#ifdef FIOGETOWN
	case FIOGETOWN:
#endif
#ifdef FIOSETOWN
	case FIOSETOWN:
#endif
#ifdef TIOCGETD
	case TIOCGETD:
#endif
#ifdef TIOCSETD
	case TIOCSETD:
#endif
#ifdef TIOCPKT
	case TIOCPKT:
#endif
#ifdef TIOCREMOTE
	case TIOCREMOTE:
#endif
#ifdef TIOCUCNTL
	case TIOCUCNTL:
#endif
#ifdef TIOCTCNTL
	case TIOCTCNTL:
#endif
#ifdef TIOCSIGNAL
	case TIOCSIGNAL:
#endif
#ifdef TIOCSSOFTCAR
	case TIOCSSOFTCAR:
#endif
#ifdef TIOCGSOFTCAR
	case TIOCGSOFTCAR:
#endif
#ifdef TIOCISPACE
	case TIOCISPACE:
#endif
#ifdef TIOCISIZE
	case TIOCISIZE:
#endif
#ifdef TIOCSINTR
	case TIOCSINTR:
#endif
#ifdef TIOCSPTLCK
	case TIOCSPTLCK:
#endif
#ifdef TIOCGPTN
	case TIOCGPTN:
#endif
		tprints(", ");
		printnum_int(tcp, arg, "%d");
		return 1;

	/* ioctls with an indirect parameter displayed as a char */

#ifdef TIOCSTI
	case TIOCSTI:
#endif
		tprints(", ");
		printstr(tcp, arg, 1);
		return 1;

	/* ioctls with no parameters */

#ifdef TIOCNOTTY
	case TIOCNOTTY:
#endif
#ifdef FIOCLEX
	case FIOCLEX:
#endif
#ifdef FIONCLEX
	case FIONCLEX:
#endif
#ifdef TIOCCONS
	case TIOCCONS:
#endif
		return 1;

	/* ioctls which are unknown */

	default:
		return 0;
	}
}
