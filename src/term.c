/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-2022 The strace developers.
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

#include "xlat/term_cflags.h"
#include "xlat/term_cflags_csize.h"
#include "xlat/term_iflags.h"
#include "xlat/term_lflags.h"
#include "xlat/term_oflags.h"
#include "xlat/term_oflags_bsdly.h"
#include "xlat/term_oflags_crdly.h"
#include "xlat/term_oflags_ffdly.h"
#include "xlat/term_oflags_nldly.h"
#include "xlat/term_oflags_tabdly.h"
#include "xlat/term_oflags_vtdly.h"

#include "xlat/term_line_discs.h"

#include "xlat/termios_cc.h"

#ifdef _VMIN /* thanks, alpha and powerpc */
# include "xlat/termio_cc.h"
#else
# define termio_cc termios_cc
#endif

#include "xlat/term_cmds_overlapping.h"

static void
decode_oflag(uint64_t val)
{
	static const struct {
		const struct xlat *xl;
		uint64_t mask;
		const char *dfl;
	} xlats[] = {
		{ term_oflags_nldly,  NLDLY,  "NL?"  },
		{ term_oflags_crdly,  CRDLY,  "CR?"  },
		{ term_oflags_tabdly, TABDLY, "TAB?" },
		{ term_oflags_bsdly,  BSDLY,  "BS?"  },
		{ term_oflags_vtdly,  VTDLY,  "VT?"  },
		{ term_oflags_ffdly,  FFDLY,  "FF?"  },
	};

	tprint_flags_begin();
	for (unsigned int i = 0; i < ARRAY_SIZE(xlats); i++) {
		printxval64(xlats[i].xl, val & xlats[i].mask, xlats[i].dfl);
		tprint_flags_or();

		val &= ~xlats[i].mask;
	}

	printflags64_in(term_oflags, val, NULL);
	tprint_flags_end();
}

static void
decode_cflag(uint64_t val)
{
	tprint_flags_begin();
	printxval64(baud_options, val & CBAUD, "B???");
	tprint_flags_or();

	if (val & CIBAUD) {
		tprint_shift_begin();
		printxval64(baud_options, (val & CIBAUD) >> IBSHIFT, "B???");
		tprint_shift();
		print_xlat(IBSHIFT);
		tprint_shift_end();
		tprint_flags_or();
	}

	printxval64(term_cflags_csize, val & CSIZE, "CS?");
	tprint_flags_or();

	val &= ~(CBAUD | CIBAUD | CSIZE);
	printflags64_in(term_cflags, val, NULL);
	tprint_flags_end();
}

static void
decode_flags(uint64_t iflag, uint64_t oflag, uint64_t cflag, uint64_t lflag)
{
	tprints_field_name("c_iflag");
	printflags64(term_iflags, iflag, NULL);

	tprint_struct_next();
	tprints_field_name("c_oflag");
	decode_oflag(oflag);

	tprint_struct_next();
	tprints_field_name("c_cflag");
	decode_cflag(cflag);

	tprint_struct_next();
	tprints_field_name("c_lflag");
	printflags64(term_lflags, lflag, NULL);
}

static void
print_cc_char(bool *first, const unsigned char *data, const char *s,
	      unsigned int idx)
{
	if (*first)
		*first = false;
	else
		tprint_array_next();

	tprint_array_index_begin();
	if (s)
		tprints_string(s);
	else
		PRINT_VAL_U(idx);
	tprint_array_index_equal();

	PRINT_VAL_X(data[idx]);
	tprint_array_index_end();
}

static void
decode_term_cc(const struct xlat *xl, const unsigned char *data, unsigned size)
{
	tprints_field_name("c_cc");
	tprint_array_begin();
	bool first = true;
	for (unsigned int i = 0; i < size; i++)
		print_cc_char(&first, data, xlookup(xl, i), i);
	tprint_array_end();
}

#ifdef HAVE_STRUCT_TERMIOS2
static void
decode_termios2(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct termios2 tios;
	if (umove_or_printaddr(tcp, addr, &tios))
		return;

	tprint_struct_begin();
	decode_flags(tios.c_iflag, tios.c_oflag, tios.c_cflag, tios.c_lflag);
	tprint_struct_next();

	if (abbrev(tcp)) {
		tprint_more_data_follows();
	} else {
		PRINT_FIELD_XVAL(tios, c_line, term_line_discs, "N_???");

		tprint_struct_next();
		/* SPARC has two additional bytes in c_cc. */
		decode_term_cc(termios_cc, tios.c_cc, sizeof(tios.c_cc));

		tprint_struct_next();
		PRINT_FIELD_U(tios, c_ispeed);

		tprint_struct_next();
		PRINT_FIELD_U(tios, c_ospeed);
	}
	tprint_struct_end();
}
#endif /* HAVE_STRUCT_TERMIOS2 */

static void
decode_termios(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct termios tios;
	if (umove_or_printaddr(tcp, addr, &tios))
		return;

	tprint_struct_begin();
	decode_flags(tios.c_iflag, tios.c_oflag, tios.c_cflag, tios.c_lflag);
	tprint_struct_next();

	if (abbrev(tcp)) {
		tprint_more_data_follows();
	} else {
		PRINT_FIELD_XVAL(tios, c_line, term_line_discs, "N_???");

		tprint_struct_next();
		/*
		 * Fun fact: MIPS has NCCS defined to 23, SPARC to 17 and
		 * everything else to 19.
		 */
		decode_term_cc(termios_cc, tios.c_cc,
			       MIN(NCCS, sizeof(tios.c_cc)));

		/*
		 * alpha and powerpc have those in struct termios instead of
		 * having a separate struct termios2.
		 */
#ifdef HAVE_STRUCT_TERMIOS_C_ISPEED
		tprint_struct_next();
		PRINT_FIELD_U(tios, c_ispeed);
#endif
#ifdef HAVE_STRUCT_TERMIOS_C_OSPEED
		tprint_struct_next();
		PRINT_FIELD_U(tios, c_ospeed);
#endif
	}
	tprint_struct_end();
}

static void
decode_termio(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct termio tio;
	if (umove_or_printaddr(tcp, addr, &tio))
		return;

	tprint_struct_begin();
	decode_flags(tio.c_iflag, tio.c_oflag, tio.c_cflag, tio.c_lflag);
	tprint_struct_next();

	if (abbrev(tcp)) {
		tprint_more_data_follows();
	} else {
		PRINT_FIELD_XVAL(tio, c_line, term_line_discs, "N_???");

		tprint_struct_next();
		decode_term_cc(termio_cc, tio.c_cc,
			       MIN(NCC, sizeof(tio.c_cc)));
	}
	tprint_struct_end();
}

static void
decode_winsize(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct winsize ws;
	if (umove_or_printaddr(tcp, addr, &ws))
		return;

	tprint_struct_begin();
	PRINT_FIELD_U(ws, ws_row);

	tprint_struct_next();
	PRINT_FIELD_U(ws, ws_col);

	tprint_struct_next();
	PRINT_FIELD_U(ws, ws_xpixel);

	tprint_struct_next();
	PRINT_FIELD_U(ws, ws_ypixel);

	tprint_struct_end();
}

#ifdef TIOCGSIZE
static void
decode_ttysize(struct tcb *const tcp, const kernel_ulong_t addr)
{
	struct ttysize ts;
	if (umove_or_printaddr(tcp, addr, &ts))
		return;

	tprint_struct_begin();
	PRINT_FIELD_U(ts, ts_lines);
	tprint_struct_next();
	PRINT_FIELD_U(ts, ts_cols);
	tprint_struct_end();
}
#endif

static void
decode_modem_flags(struct tcb *const tcp, const kernel_ulong_t addr)
{
	unsigned int flags;
	if (umove_or_printaddr(tcp, addr, &flags))
		return;

	tprint_indirect_begin();
	printflags(modem_flags, flags, "TIOCM_???");
	tprint_indirect_end();
}

int
term_ioctl(struct tcb *const tcp, const unsigned int code,
	   const kernel_ulong_t arg)
{
	switch (code) {
#ifdef HAVE_STRUCT_TERMIOS2
	/* struct termios2 */
# ifdef TCGETS2
	case TCGETS2:
# endif
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
# ifdef TCSETS2
	case TCSETS2:
# endif
# ifdef TCSETSW2
	case TCSETSW2:
# endif
# ifdef TCSETSF2
	case TCSETSF2:
# endif
		tprint_arg_next();
		decode_termios2(tcp, arg);
		break;
#endif /* HAVE_STRUCT_TERMIOS2 */

	/* struct termios */
	case TCGETS:
	case TIOCGLCKTRMIOS:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TCSETS:
	case TCSETSW:
	case TCSETSF:
	case TIOCSLCKTRMIOS:
		tprint_arg_next();
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
		tprint_arg_next();
		decode_termio(tcp, arg);
		break;

	/* struct winsize */
	case TIOCGWINSZ:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TIOCSWINSZ:
		tprint_arg_next();
		decode_winsize(tcp, arg);
		break;

	/* struct ttysize */
#ifdef TIOCGSIZE
	case TIOCGSIZE:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TIOCSSIZE:
		tprint_arg_next();
		decode_ttysize(tcp, arg);
		break;
#endif

	/* ioctls with a direct decodable arg */
	case TCXONC:
		tprint_arg_next();
		printxval64(tcxonc_options, arg, "TC???");
		break;
	case TCFLSH:
		tprint_arg_next();
		printxval64(tcflsh_options, arg, "TC???");
		break;
	case TCSBRK:
	case TCSBRKP:
	case TIOCSCTTY:
		tprint_arg_next();
		PRINT_VAL_D((int) arg);
		break;

	/* ioctls with an indirect parameter displayed as modem flags */
	case TIOCMGET:
		if (entering(tcp))
			return 0;
		ATTRIBUTE_FALLTHROUGH;
	case TIOCMBIS:
	case TIOCMBIC:
	case TIOCMSET:
		tprint_arg_next();
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
		tprint_arg_next();
		printnum_int(tcp, arg, "%d");
		break;

	/* ioctls with an indirect parameter displayed as a char */
	case TIOCSTI:
		tprint_arg_next();
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

/*
 * TTY and SND ioctl commands may clash, for example:
 *
 *    0x00005404
 *    { "SNDCTL_TMR_CONTINUE", 0x00005404 },
 *    { "TCSETSF", 0x00005404 },
 *    0x00005403
 *    { "SNDCTL_TMR_STOP", 0x00005403 },
 *    { "TCSETSW", 0x00005403 },
 *    0x00005402
 *    { "SNDCTL_TMR_START", 0x00005402 },
 *    { "TCSETS", 0x00005402 },
 *
 * This function tries to resolve the collision using the device information
 * associated with the specified file descriptor.
 */
int
term_ioctl_decode_command_number(struct tcb *tcp,
				 const struct finfo *finfo,
				 unsigned int code)
{
   /*
    * See Linux kernel Documentation/admin-guide/devices.txt
    */
   if (finfo
       && finfo->type == FINFO_DEV_CHR
       && ((3 <= finfo->dev.major && finfo->dev.major <= 5) ||
	   (136 <= finfo->dev.major && finfo->dev.major <= 143))) {
	   const char *str = xlookup(term_cmds_overlapping, code);
	   if (str) {
		   tprints_string(str);
		   return IOCTL_NUMBER_STOP_LOOKUP;
	   }
   }
   return 0;
}
