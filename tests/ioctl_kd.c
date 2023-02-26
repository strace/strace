/*
 * This file is part of ioctl_kd strace test.
 *
 * Copyright (c) 2019-2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * Copyright (c) 2019-2023 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <signal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "scno.h"

#include <linux/ioctl.h>
#include <linux/kd.h>
#include <linux/keyboard.h>

#ifndef RETVAL_INJECTED
# define RETVAL_INJECTED 0
#endif

#if RETVAL_INJECTED
# define RETVAL "42 (INJECTED)\n"
#else
# define RETVAL "-1 EBADF (%m)\n"
#endif

#ifndef HAVE_STRUCT_KBDIACRUC
struct kbdiacruc {
	unsigned int diacr, base, result;
};
#endif

#ifndef HAVE_STRUCT_KBDIACRSUC
struct kbdiacrsuc {
	struct kbdiacruc kbdiacruc[256];
}
#endif

struct arg_val {
	kernel_ulong_t val;
	const char *str;
};

static long
sys_ioctl(kernel_long_t fd, kernel_ulong_t cmd, kernel_ulong_t arg)
{
	return syscall(__NR_ioctl, fd, cmd, arg);
}

static void
check_null_invalid(unsigned int c, const char *s)
{
	static char *p;

	if (!p)
		p = tail_alloc(1);

	sys_ioctl(-1, c, 0);
	printf("ioctl(-1, " XLAT_FMT ", NULL) = " RETVAL, XLAT_SEL(c, s));

	if (F8ILL_KULONG_SUPPORTED) {
		sys_ioctl(-1, c, F8ILL_KULONG_MASK);
		printf("ioctl(-1, " XLAT_FMT ", NULL) = " RETVAL,
		       XLAT_SEL(c, s));
	}

	sys_ioctl(-1, c, (uintptr_t) p + 1);
	printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL, XLAT_SEL(c, s), p + 1);
}

/* GIO_SCRNMAP, PIO_SCRNMAP */
static void
check_scrnmap(unsigned int c, const char *s)
{
	char *scrnmap = tail_alloc(E_TABSZ);

	int saved_errno;
	long rc;

	fill_memory_ex(scrnmap, E_TABSZ, 0, 0xff);

	check_null_invalid(c, s);

	sys_ioctl(-1, c, (uintptr_t) scrnmap + E_TABSZ - 31);
	printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL,
	       XLAT_SEL(c, s), scrnmap + E_TABSZ - 31);

	sys_ioctl(-1, c, (uintptr_t) scrnmap + E_TABSZ - 32);
	printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL,
	       XLAT_SEL(c, s), scrnmap + E_TABSZ - 32);

	rc = sys_ioctl(-1, c, (uintptr_t) scrnmap + E_TABSZ - 33);
	saved_errno = errno;
	printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c, s));
	if ((rc >= 0 || c == PIO_SCRNMAP) && (DEFAULT_STRLEN <= 32)) {
		print_quoted_hex(scrnmap + E_TABSZ - 33, 32);
		printf("...");
	} else {
		printf("%p", scrnmap + E_TABSZ - 33);
	}
	errno = saved_errno;
	printf(") = " RETVAL);

	rc = sys_ioctl(-1, c, (uintptr_t) scrnmap);
	saved_errno = errno;
	printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c, s));
	if (rc >= 0 || c == PIO_SCRNMAP) {
		print_quoted_hex(scrnmap, MIN(E_TABSZ, DEFAULT_STRLEN));
		if (DEFAULT_STRLEN < E_TABSZ)
			printf("...");
	} else {
		printf("%p", scrnmap);
	}
	errno = saved_errno;
	printf(") = " RETVAL);
}

/* KDGKBENT, KDSKBENT */
static void
check_kbdent(unsigned int c, const char *s)
{
	static const struct arg_val kbtbl_vecs[] = {
		{ ARG_XLAT_KNOWN(0, "K_NORMTAB") },
		{ ARG_XLAT_KNOWN(0x1, "K_SHIFTTAB") },
		{ ARG_XLAT_KNOWN(0x3, "K_ALTSHIFTTAB") },
		{ ARG_XLAT_KNOWN(0x4, "1<<KG_CTRL") },
		{ ARG_XLAT_KNOWN(0xff,
				 "1<<KG_SHIFT|1<<KG_ALTGR|1<<KG_CTRL"
				 "|1<<KG_ALT|1<<KG_SHIFTL|1<<KG_SHIFTR"
				 "|1<<KG_CTRLL|1<<KG_CTRLR") },
	};

	static const struct arg_val kbval_vecs[] = {
		{ ARG_STR(0) NRAW(" /* K(KT_LATIN, '\\x00') */") },
		{ ARG_STR(0x10) NRAW(" /* K(KT_LATIN, '\\x10') */") },
		{ ARG_STR(0x20) NRAW(" /* K(KT_LATIN, ' ') */") },
		{ ARG_STR(0x7e) NRAW(" /* K(KT_LATIN, '~') */") },
		{ ARG_STR(0x7f) NRAW(" /* K(KT_LATIN, '\\x7f') */") },

		{ ARG_STR(0x100) NRAW(" /* K_F1 */") },
		{ ARG_STR(0x11d) NRAW(" /* K_PAUSE */") },
		{ ARG_STR(0x1ff) NRAW(" /* K_UNDO */") },

		{ ARG_STR(0x200) NRAW(" /* K_HOLE */") },
		{ ARG_STR(0x213) NRAW(" /* K_BARENUMLOCK */") },
		{ ARG_STR(0x214) NRAW(" /* K(KT_SPEC, 0x14) */") },
		{ ARG_STR(0x27d) NRAW(" /* K(KT_SPEC, 0x7d) */") },
		{ ARG_STR(0x27e) NRAW(" /* K_ALLOCATED */") },
		{ ARG_STR(0x27f) NRAW(" /* K_NOSUCHMAP */") },
		{ ARG_STR(0x280) NRAW(" /* K(KT_SPEC, 0x80) */") },
		{ ARG_STR(0x2ff) NRAW(" /* K(KT_SPEC, 0xff) */") },

		{ ARG_STR(0x300) NRAW(" /* K_P0 */") },
		{ ARG_STR(0x313) NRAW(" /* K_PPARENR */") },
		{ ARG_STR(0x314) NRAW(" /* K(KT_PAD, 0x14) */") },
		{ ARG_STR(0x37f) NRAW(" /* K(KT_PAD, 0x7f) */") },
		{ ARG_STR(0x3ff) NRAW(" /* K(KT_PAD, 0xff) */") },

		{ ARG_STR(0x400) NRAW(" /* K_DGRAVE */") },
		{ ARG_STR(0x41a) NRAW(" /* K_DGREEK */") },
		{ ARG_STR(0x41b) NRAW(" /* K(KT_DEAD, 0x1b) */") },
		{ ARG_STR(0x47f) NRAW(" /* K(KT_DEAD, 0x7f) */") },
		{ ARG_STR(0x4ff) NRAW(" /* K(KT_DEAD, 0xff) */") },

		{ ARG_STR(0x500) NRAW(" /* K(KT_CONS, 0) */") },
		{ ARG_STR(0x540) NRAW(" /* K(KT_CONS, 0x40) */") },
		{ ARG_STR(0x5ff) NRAW(" /* K(KT_CONS, 0xff) */") },

		{ ARG_STR(0x600) NRAW(" /* K_DOWN */") },
		{ ARG_STR(0x603) NRAW(" /* K_UP */") },
		{ ARG_STR(0x604) NRAW(" /* K(KT_CUR, 0x4) */") },
		{ ARG_STR(0x680) NRAW(" /* K(KT_CUR, 0x80) */") },
		{ ARG_STR(0x6ff) NRAW(" /* K(KT_CUR, 0xff) */") },

		{ ARG_STR(0x700) NRAW(" /* K_SHIFT */") },
		{ ARG_STR(0x708) NRAW(" /* K_CAPSSHIFT */") },
		{ ARG_STR(0x709) NRAW(" /* K(KT_SHIFT, 0x9) */") },
		{ ARG_STR(0x7ff) NRAW(" /* K(KT_SHIFT, 0xff) */") },

		{ ARG_STR(0x800) NRAW(" /* K(KT_META, '\\x00') */") },
		{ ARG_STR(0x841) NRAW(" /* K(KT_META, 'A') */") },
		{ ARG_STR(0x8ff) NRAW(" /* K(KT_META, '\\xff') */") },

		{ ARG_STR(0x900) NRAW(" /* K_ASC0 */") },
		{ ARG_STR(0x909) NRAW(" /* K_ASC9 */") },
		{ ARG_STR(0x90a) NRAW(" /* K_HEX0 */") },
		{ ARG_STR(0x919) NRAW(" /* K_HEXf */") },
		{ ARG_STR(0x91a) NRAW(" /* K(KT_ASCII, 0x1a) */") },
		{ ARG_STR(0x9ff) NRAW(" /* K(KT_ASCII, 0xff) */") },

		{ ARG_STR(0xa00) NRAW(" /* K_SHIFTLOCK */") },
		{ ARG_STR(0xa08) NRAW(" /* K_CAPSSHIFTLOCK */") },
		{ ARG_STR(0xa09) NRAW(" /* K(KT_LOCK, 0x9) */") },
		{ ARG_STR(0xaff) NRAW(" /* K(KT_LOCK, 0xff) */") },

		{ ARG_STR(0xb00) NRAW(" /* K(KT_LETTER, '\\x00') */") },
		{ ARG_STR(0xb40) NRAW(" /* K(KT_LETTER, '@') */") },
		{ ARG_STR(0xb7f) NRAW(" /* K(KT_LETTER, '\\x7f') */") },
		{ ARG_STR(0xbff) NRAW(" /* K(KT_LETTER, '\\xff') */") },

		{ ARG_STR(0xc00) NRAW(" /* K_SHIFT_SLOCK */") },
		{ ARG_STR(0xc08) NRAW(" /* K_CAPSSHIFT_SLOCK */") },
		{ ARG_STR(0xc09) NRAW(" /* K(KT_SLOCK, 0x9) */") },
		{ ARG_STR(0xcff) NRAW(" /* K(KT_SLOCK, 0xff) */") },

		{ ARG_STR(0xd00) NRAW(" /* K(KT_DEAD2, '\\x00') */") },
		{ ARG_STR(0xd13) NRAW(" /* K(KT_DEAD2, '\\x13') */") },
		{ ARG_STR(0xd5c) NRAW(" /* K(KT_DEAD2, '\\\\') */") },
		{ ARG_STR(0xdff) NRAW(" /* K(KT_DEAD2, '\\xff') */") },

		{ ARG_STR(0xe00) NRAW(" /* K_BRL_BLANK */") },
		{ ARG_STR(0xe0a) NRAW(" /* K_BRL_DOT10 */") },
		{ ARG_STR(0xe0b) NRAW(" /* K(KT_BRL, 0xb) */") },
		{ ARG_STR(0xeff) NRAW(" /* K(KT_BRL, 0xff) */") },

		{ ARG_STR(0xf00) NRAW(" /* K(0xf, 0) */") },
		{ ARG_STR(0xfed) NRAW(" /* K(0xf, 0xed) */") },
		{ ARG_STR(0xf00d) NRAW(" /* K(0xf0, 0xd) */") },
	};

	struct kbentry *kbe = tail_alloc(sizeof(*kbe));

	int saved_errno;
	long rc;

	check_null_invalid(c, s);

	kbe->kb_value = 0xa8a8;
	sys_ioctl(-1, c, (uintptr_t) kbe + 2);
	printf("ioctl(-1, " XLAT_FMT ", {kb_table=%s, kb_index=168%s}"
	       ") = " RETVAL,
	       XLAT_SEL(c, s), XLAT_STR(1<<KG_ALT|1<<KG_SHIFTR|1<<KG_CTRLR),
	       RETVAL_INJECTED || c == KDSKBENT ? ", kb_value=???" : ""
	       );

	for (size_t i = 0;
	     i < MAX(ARRAY_SIZE(kbtbl_vecs), ARRAY_SIZE(kbval_vecs)); i++) {
		kbe->kb_table = kbtbl_vecs[i % ARRAY_SIZE(kbtbl_vecs)].val;
		kbe->kb_index = i * 3141;
		kbe->kb_value = kbval_vecs[i % ARRAY_SIZE(kbval_vecs)].val;

		rc = sys_ioctl(-1, c, (uintptr_t) kbe);
		saved_errno = errno;
		printf("ioctl(-1, " XLAT_FMT ", {kb_table=%s, kb_index=%u",
		       XLAT_SEL(c, s),
		       kbtbl_vecs[i % ARRAY_SIZE(kbtbl_vecs)].str,
		       kbe->kb_index);
		if (rc >= 0 || c == KDSKBENT) {
			printf(", kb_value=%s",
			       kbval_vecs[i % ARRAY_SIZE(kbval_vecs)].str);
		}
		errno = saved_errno;
		printf("}) = " RETVAL);
	}
}

/* KDGKBSENT, KDSKBSENT */
static void
check_kbdsent(unsigned int c, const char *s)
{
	static struct arg_val kbse_offsets[] = {
		{ sizeof(struct kbsentry) - 1, "KVAL(K_F2)" },
		{ sizeof(struct kbsentry) - 2, "KVAL(K_F1)" },
		{ sizeof(struct kbsentry) - 34, "KVAL(K_F214)" },
		{ sizeof(struct kbsentry) - 35, "KVAL(K_F213)" },
		{ 1, "KVAL(K_F1)" },
		{ 0, "KVAL(K_F245)" },
	};

	static const struct arg_val kbfn_vecs[] = {
		{ ARG_XLAT_KNOWN(0, "KVAL(K_F1)") },
		{ ARG_XLAT_KNOWN(0x10, "KVAL(K_F17)") },
		{ ARG_XLAT_KNOWN(0x7f, "KVAL(K_F118)") },
		{ ARG_XLAT_KNOWN(0xff, "KVAL(K_UNDO)") },
	};

	struct kbsentry *kbse = tail_alloc(sizeof(*kbse));

	int saved_errno;

	fill_memory_ex(kbse->kb_string, sizeof(kbse->kb_string), 0, 0xff);
	kbse->kb_func = 0xfe;

	check_null_invalid(c, s);

	for (size_t i = 0; i < ARRAY_SIZE(kbse_offsets); i++) {
		sys_ioctl(-1, c,
			  (uintptr_t) kbse + kbse_offsets[i].val);
		saved_errno = errno;
		printf("ioctl(-1, " XLAT_FMT ", {kb_func=%s",
	               XLAT_SEL(c, s),
		       sprintxlat(kbse_offsets[i].str,
				  (kbse_offsets[i].val + 254) % 0xff, NULL));

		if (RETVAL_INJECTED || c == KDSKBSENT) {
			printf(", kb_string=");
			if (kbse_offsets[i].val < 255 * 2) {
				print_quoted_stringn(
					(char *) kbse->kb_string +
					kbse_offsets[i].val,
					MIN(DEFAULT_STRLEN,
					    sizeof(kbse->kb_string)));
			} else {
				printf("???");
			}
		}

		errno = saved_errno;
		printf("}) = " RETVAL);
	}

	fill_memory_ex(kbse->kb_string, sizeof(kbse->kb_string), 0x80, 0x7f);
	kbse->kb_func = KVAL(K_PGDN);

	sys_ioctl(-1, c, (uintptr_t) kbse);
	saved_errno = errno;
	printf("ioctl(-1, " XLAT_FMT ", {kb_func="
	       XLAT_KNOWN(0x19, "KVAL(K_PGDN)"),
               XLAT_SEL(c, s));

	if (RETVAL_INJECTED || c == KDSKBSENT) {
		printf(", kb_string=");
		print_quoted_stringn((char *) kbse->kb_string,
				     MIN(DEFAULT_STRLEN,
					 sizeof(kbse->kb_string)));
	}

	errno = saved_errno;
	printf("}) = " RETVAL);

	for (size_t i = 0; i < ARRAY_SIZE(kbfn_vecs); i++) {
		kbse->kb_func = kbfn_vecs[i].val;
		fill_memory_ex(kbse->kb_string, sizeof(kbse->kb_string),
			       i * 357 + 42, i * 257 + 13);

		sys_ioctl(-1, c, (uintptr_t) kbse);
		saved_errno = errno;
		printf("ioctl(-1, " XLAT_FMT ", {kb_func=%s",
		       XLAT_SEL(c, s),
		       kbfn_vecs[i].str);

		if (RETVAL_INJECTED || c == KDSKBSENT) {
			printf(", kb_string=");
			print_quoted_stringn((char *) kbse->kb_string,
					     MIN(DEFAULT_STRLEN,
						 sizeof(kbse->kb_string)));
		}

		errno = saved_errno;
		printf("}) = " RETVAL);
	}

	if (DEFAULT_STRLEN < sizeof(kbse->kb_string) &&
	    (RETVAL_INJECTED || c == KDSKBSENT)) {
		/*
		 * Check how struct kbsentry.kb_string is printed when it
		 * starts DEFAULT_STRLEN / 2 bytes before the page boundary.
		 */
		struct kbsentry *const k =
			tail_alloc(get_page_size() +
				   DEFAULT_STRLEN / 2 + sizeof(k->kb_func));
		fill_memory_ex(k->kb_string, DEFAULT_STRLEN - 1, '0', 10);
		k->kb_string[DEFAULT_STRLEN - 1] = '\0';

		sys_ioctl(-1, c, (uintptr_t) k);
		printf("ioctl(-1, " XLAT_FMT ", {kb_func="
		       XLAT_KNOWN(0xff, "KVAL(K_UNDO)")
		       ", kb_string=\"%s\"}) = " RETVAL,
		       XLAT_SEL(c, s), k->kb_string);
	}
}

/* KDGKBDIACR, KDSKBDIACR */
static void
check_diacr(unsigned int c, const char *s)
{
	static struct arg_val diac_vecs[] = {
		{ 0,    "\\x00" },
		{ '\n', "\\n" },
		{ ' ',  " " },
		{ 'a',  "a" },
		{ '~',  "~" },
		{ '\'',  "\\'" },
		{ '\\',  "\\\\" },
		{ '"',  "\"" },
		{ '`',  "`" },
		{ 0x7f, "\\x7f" },
		{ 0xff, "\\xff" },
	};

	struct kbdiacrs *diacrs0 = tail_alloc(sizeof(diacrs0->kb_cnt));
	struct kbdiacrs *diacrs1 = tail_alloc(sizeof(diacrs1->kb_cnt) +
					      4 * sizeof(struct kbdiacr));
	struct kbdiacrs *diacrs2 = tail_alloc(sizeof(*diacrs2));

	int saved_errno;

	check_null_invalid(c, s);

	for (size_t i = 0; i < 2; i++) {
		diacrs0->kb_cnt = i;
		sys_ioctl(-1, c, (uintptr_t) diacrs0);
		saved_errno = errno;
		printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c, s));
		if (RETVAL_INJECTED || c == KDSKBDIACR) {
			printf("{kb_cnt=%zu, kbdiacr=", i);
			if (i)
				printf("%p}", diacrs0->kbdiacr);
			else
				printf("[]}");
		} else {
			printf("%p", diacrs0);
		}
		errno = saved_errno;
		printf(") = " RETVAL);
	}

	fill_memory_ex(diacrs1->kbdiacr, 4 * sizeof(struct kbdiacr), 40, 44);
	for (size_t i = 0; i < 7; i++) {
		diacrs1->kb_cnt = i;
		sys_ioctl(-1, c, (uintptr_t) diacrs1);
		saved_errno = errno;
		printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c, s));
		if (RETVAL_INJECTED || c == KDSKBDIACR) {
			printf("{kb_cnt=%zu, kbdiacr=[", i);
			for (size_t j = 0; j < MIN(i, 4); j++)
				printf("%s{diacr='%c', base='%c', result='%c'}",
				       j ? ", " : "", (int) (40 + j * 3),
				       (int) (41 + j * 3), (int) (42 + j * 3));

			if (i > 4)
				printf(", ... /* %p */", diacrs1->kbdiacr + 4);
			printf("]}");
		} else {
			printf("%p", diacrs1);
		}
		errno = saved_errno;
		printf(") = " RETVAL);
	}

	fill_memory_ex(diacrs2->kbdiacr, sizeof(diacrs2->kbdiacr), 40, 52);

	for (size_t i = ARRAY_SIZE(diacrs2->kbdiacr) - 1;
	     i < ARRAY_SIZE(diacrs2->kbdiacr) + 3; i++) {
		diacrs2->kb_cnt = i;
		sys_ioctl(-1, c, (uintptr_t) diacrs2);
		printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c, s));
		saved_errno = errno;
		if (RETVAL_INJECTED || c == KDSKBDIACR) {
			printf("{kb_cnt=%zu, kbdiacr=[", i);
			for (size_t j = 0;
			     j < MIN(i, MIN(DEFAULT_STRLEN,
					    ARRAY_SIZE(diacrs2->kbdiacr))); j++)
				printf("%s{diacr='%c', base='%c', result='%c'}",
				       j ? ", " : "",
				       (int) (40 + (j * 3 + 0) % 52),
				       (int) (40 + (j * 3 + 1) % 52),
				       (int) (40 + (j * 3 + 2) % 52));

			if (i > MIN(DEFAULT_STRLEN,
				    ARRAY_SIZE(diacrs2->kbdiacr)))
				printf(", ...");
			printf("]}");
		} else {
			printf("%p", diacrs2);
		}
		errno = saved_errno;
		printf(") = " RETVAL);
	}

	for (size_t i = 0; i< ARRAY_SIZE(diac_vecs); i++) {
		diacrs2->kbdiacr[i].diacr  = diac_vecs[i].val;
		diacrs2->kbdiacr[i].base   = diac_vecs[i].val;
		diacrs2->kbdiacr[i].result = diac_vecs[i].val;
	}
	diacrs2->kb_cnt = ARRAY_SIZE(diac_vecs);

	sys_ioctl(-1, c, (uintptr_t) diacrs2);
	printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c, s));
	saved_errno = errno;
	if (RETVAL_INJECTED || c == KDSKBDIACR) {
		printf("{kb_cnt=%zu, kbdiacr=[", ARRAY_SIZE(diac_vecs));
		for (size_t i = 0; i < ARRAY_SIZE(diac_vecs); i++)
			printf("%1$s{diacr='%2$s', base='%2$s', result='%2$s'}",
			       i ? ", " : "", diac_vecs[i].str);
		printf("]}");
	} else {
		printf("%p", diacrs2);
	}
	errno = saved_errno;
	printf(") = " RETVAL);
}

/* KDGETKEYCODE, KDSETKEYCODE */
static void
check_xetkeycode(unsigned int c, const char *s)
{
	static const struct kbkeycode args[] = {
		{ 0, 0 },
		{ 0, 0xdeadface },
		{ 0xfacefeed, 0 },
		{ 0xdecaffed, 0xdadfaced },
	};
	struct kbkeycode *tail_arg = tail_alloc(sizeof(args[0]));

	check_null_invalid(c, s);

	sys_ioctl(-1, c, (uintptr_t) tail_arg + 4);
	printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL,
	       XLAT_SEL(c, s), (char *) tail_arg + 4);

	for (size_t i = 0; i < ARRAY_SIZE(args); i++) {
		memcpy(tail_arg, args + i, sizeof(args[i]));

		sys_ioctl(-1, c, (uintptr_t) tail_arg);
		printf("ioctl(-1, " XLAT_FMT ", {scancode=%#x, keycode=%#x",
		       XLAT_SEL(c, s), args[i].scancode, args[i].keycode);
		if ((c == KDGETKEYCODE) && RETVAL_INJECTED)
			printf(" => %#x", args[i].keycode);
		printf("}) = " RETVAL);
	}
}

/* KDKBDREP */
static void
check_kbdrep(unsigned int c, const char *s)
{
	static const struct kbd_repeat args[] = {
		{ -1, -1 },
		{ -1234567890,  0 },
		{  0, -2134567890 },
		{ 314159265, 271828182 },
	};
	struct kbd_repeat *tail_arg = tail_alloc(sizeof(args[0]));

	check_null_invalid(c, s);

	sys_ioctl(-1, c, (uintptr_t) tail_arg + 4);
	printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL,
	       XLAT_SEL(c, s), (char *) tail_arg + 4);

	for (size_t i = 0; i < ARRAY_SIZE(args); i++) {
		memcpy(tail_arg, args + i, sizeof(args[i]));

		sys_ioctl(-1, c, (uintptr_t) tail_arg);
		printf("ioctl(-1, " XLAT_FMT, XLAT_SEL(c, s));
		for (size_t j = 0; j < 1 + RETVAL_INJECTED; j++) {
			printf("%s {delay=%d, period=%d}",
			       j ? " =>" : ",", args[i].delay, args[i].period);
		}
		printf(") = " RETVAL);
	}
}

/* GIO_FONT, PIO_FONT */
static void
check_font(unsigned int c, const char *s)
{
	char *data = tail_alloc(8192);
	char *data_end = data + 8192;

	fill_memory(data, 8192);

	check_null_invalid(c, s);

	sys_ioctl(-1, c, (uintptr_t) data_end - 31);
	printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL,
	       XLAT_SEL(c, s), data_end - 31);

	sys_ioctl(-1, c, (uintptr_t) data_end - 32);
	printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL,
	       XLAT_SEL(c, s), data_end - 32);

	bool ok = (DEFAULT_STRLEN == 32)
		   && ((c != GIO_FONT) || RETVAL_INJECTED);
	sys_ioctl(-1, c, (uintptr_t) data_end - 33);
	printf("ioctl(-1, " XLAT_FMT ", %s", XLAT_SEL(c, s), ok ? "\"" : "");
	if (ok) {
		for (size_t i = 8192 - 33; i < 8192 - 1; i++)
			printf("\\x%hhx", (unsigned char) ( 0x80 + i % 0x80));
	} else {
		printf("%p", data_end - 33);
	}
	printf("%s) = " RETVAL, ok ? "\"..." : "");

	ok = (c != GIO_FONT) || RETVAL_INJECTED;
	sys_ioctl(-1, c, (uintptr_t) data_end - 1025);
	printf("ioctl(-1, " XLAT_FMT ", %s", XLAT_SEL(c, s), ok ? "\"" : "");
	if (ok) {
		for (size_t i = 8192 - 1025; i < 8192 - 1025 + DEFAULT_STRLEN;
		     i++)
			printf("\\x%hhx", (unsigned char) (0x80 + i % 0x80));
	} else {
		printf("%p", data_end - 1025);
	}
	printf("%s) = " RETVAL, ok ? "\"..." : "");

	sys_ioctl(-1, c, (uintptr_t) data);
	printf("ioctl(-1, " XLAT_FMT ", %s", XLAT_SEL(c, s), ok ? "\"" : "");
	if (ok) {
		for (size_t i = 0; i < DEFAULT_STRLEN; i++)
			printf("\\x%hhx", (unsigned char) (0x80 + i % 0x80));
	} else {
		printf("%p", data);
	}
	printf("%s) = " RETVAL, ok ? "\"..." : "");
}

/* GIO_UNIMAP, PIO_UNIMAP */
static void
check_unimap(unsigned int c, const char *s)
{
	struct unimapdesc *umd = tail_alloc(sizeof(*umd));
	struct unipair *ups = tail_alloc(33 * sizeof(*ups));

	fill_memory16(ups, 33 * sizeof(*ups));
	ups[0].unicode = 0;
	ups[0].fontpos = 0;

	check_null_invalid(c, s);

	umd->entry_ct = 0xdead;
	umd->entries = NULL;
	sys_ioctl(-1, c, (uintptr_t) umd);
	printf("ioctl(-1, " XLAT_FMT ", {entry_ct=57005%s, entries=NULL}) = "
	       RETVAL, XLAT_SEL(c, s), c == GIO_UNIMAP ? " => 57005" : "");

	umd->entry_ct = 0;
	umd->entries = ups + 33;
	sys_ioctl(-1, c, (uintptr_t) umd);
	printf("ioctl(-1, " XLAT_FMT ", {entry_ct=0%s, entries=",
	       XLAT_SEL(c, s), c == GIO_UNIMAP ? " => 0" : "");
	if (c == GIO_UNIMAP && !RETVAL_INJECTED)
		printf("%p", ups + 33);
	else
		printf("[]");
	printf("}) = " RETVAL);

	umd->entry_ct = 1;
	sys_ioctl(-1, c, (uintptr_t) umd);
	printf("ioctl(-1, " XLAT_FMT ", {entry_ct=1%s, entries=%p}) = " RETVAL,
	       XLAT_SEL(c, s), c == GIO_UNIMAP ? " => 1" : "", ups + 33);

	for (unsigned int i = 0; i < 6; i++) {
		umd->entry_ct = 31 + (i + 1) / 2;
		umd->entries = ups + 2 - i / 2;
		sys_ioctl(-1, c, (uintptr_t) umd);
		printf("ioctl(-1, " XLAT_FMT ", {entry_ct=%u",
		       XLAT_SEL(c, s), 31 + (i + 1) / 2);

		if (c == GIO_UNIMAP) {
			printf(" => %u", 31 + (i + 1) / 2);
#if !RETVAL_INJECTED
			printf(", entries=%p}) = " RETVAL, ups + 2 - i / 2);
			continue;
#endif
		}

		printf(", entries=[%s", i > 3 ? "{unicode=0, fontpos=0}" : "");

		for (unsigned int j = 0; j < 31
#if DEFAULT_STRLEN > 32
						+ MIN(i / 2, 1)
#else
						+ ((i / 2) == 1)
#endif
		     ; j++) {
			printf("%s{unicode=%#x, fontpos=%#x}",
			       j == 0 && i < 4 ? "" : ", ",
			       0x80c4 + 2 * (j - MIN(i / 2, 1)),
			       0x80c5 + 2 * (j - MIN(i / 2, 1)));
		}
		if (i == 1 || i == 3 || ((DEFAULT_STRLEN > 32) && (i == 5)))
			printf(", ... /* %p */", ups + 33);
#if DEFAULT_STRLEN == 32
		if (i > 3)
			printf(", ...");
#endif
		printf("]}) = " RETVAL);
	}
}

/* GIO_UNISCRNMAP, PIO_UNISCRNMAP */
static void
check_uniscrnmap(unsigned int c, const char *s)
{
	uint16_t *map = tail_alloc(256 * sizeof(*map));
	for (unsigned int i = 0; i < 256; i++)
		map[i] = 0xeff1 + 32 * (i % 112) - i / 8;

	check_null_invalid(c, s);

	for (unsigned int i = 0; i < 3; i++) {
		sys_ioctl(-1, c, (uintptr_t) (map + 224 - 112 * i));
		printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c, s));

		if (c == GIO_UNISCRNMAP && !RETVAL_INJECTED) {
			printf("%p) = " RETVAL, map + 224 - 112 * i);
			continue;
		}

		for (size_t j = 0;
		     j < MIN(32 + 112 * i, DEFAULT_STRLEN); j++) {
			uint16_t v = 0xefd5 + 32 * (j % 112) - j / 8 + i * 14;
			if ((j % 112) < (2 - (i + 1 - j / 112) / 2)
			    || (j % 112) > (17 - (i + 1 - j / 112) / 2)) {
				printf("%s%#hx", j ? ", " : "[", v);
			} else {
				printf(", "
				       XLAT_KNOWN_FMT("%#hx",
						      "UNI_DIRECT_BASE+%#hx"),
				       XLAT_SEL(v, (uint16_t) (v - 0xf000)));
			}
		}

		if (DEFAULT_STRLEN == 32 || i < 2) {
			printf(", ...");
			if (DEFAULT_STRLEN >= 32 + 112 * i)
				printf(" /* %p */", map + 256);
		}

		printf("]) = " RETVAL);
	}
}

/* GIO_FONTX, PIO_FONTX */
static void
check_fontx(unsigned int c, const char *s)
{
	static const short cnts[] = { 1, 32, 256 };
	struct consolefontdesc *cfd = tail_alloc(sizeof(*cfd));
	char *data = tail_alloc(2048);
	char *data_end = data + 2048;

	fill_memory_ex(data, 2048, 0xf0, 255);

	check_null_invalid(c, s);

	cfd->charcount = 0;
	cfd->charheight = 0xdead;
	cfd->chardata = NULL;
	sys_ioctl(-1, c, (uintptr_t) cfd);
	printf("ioctl(-1, " XLAT_FMT
	       ", {charcount=0, charheight=57005, chardata=NULL}%s) = " RETVAL,
	       XLAT_SEL(c, s), RETVAL_INJECTED && (c == GIO_FONTX)
		? " => {charcount=0, charheight=57005, chardata=NULL}" : "");

	cfd->chardata = data_end;
	sys_ioctl(-1, c, (uintptr_t) cfd);
	printf("ioctl(-1, " XLAT_FMT
	       ", {charcount=0, charheight=57005, chardata=%p}",
	       XLAT_SEL(c, s), data_end);
#if RETVAL_INJECTED
	if (c == GIO_FONTX)
		printf(" => {charcount=0, charheight=57005, chardata=\"\"}");
#endif
	printf(") = " RETVAL);

	cfd->chardata = data;
	sys_ioctl(-1, c, (uintptr_t) cfd);
	printf("ioctl(-1, " XLAT_FMT
	       ", {charcount=0, charheight=57005, chardata=%p}",
	       XLAT_SEL(c, s), data);
#if RETVAL_INJECTED
	if (c == GIO_FONTX)
		printf(" => {charcount=0, charheight=57005, chardata=\"\"}");
#endif
	printf(") = " RETVAL);

	for (size_t i = 0; i < ARRAY_SIZE(cnts); i++) {
		char *p = data_end - MIN(2048, cnts[i] * 32);
		cfd->charcount = cnts[i];

		cfd->chardata = p + 1;
		sys_ioctl(-1, c, (uintptr_t) cfd);
		printf("ioctl(-1, " XLAT_FMT
		       ", {charcount=%u, charheight=57005, chardata=",
		       XLAT_SEL(c, s), cnts[i]);
		if (c == PIO_FONTX && cnts[i] * 32 > DEFAULT_STRLEN) {
			print_quoted_hex(p + 1, DEFAULT_STRLEN);
			printf("...}");
		} else {
			printf("%p}", p + 1);
		}
#if RETVAL_INJECTED
		if (c == GIO_FONTX) {
			printf(" => {charcount=%u, charheight=57005, chardata=",
			       cnts[i]);
			if (cnts[i] * 32 > DEFAULT_STRLEN) {
				print_quoted_hex(p + 1, DEFAULT_STRLEN);
				printf("...}");
			} else {
				printf("%p}", p + 1);
			}
		}
#endif
		printf(") = " RETVAL);

		cfd->chardata = p;
		sys_ioctl(-1, c, (uintptr_t) cfd);
		printf("ioctl(-1, " XLAT_FMT
		       ", {charcount=%u, charheight=57005, chardata=",
		       XLAT_SEL(c, s), cnts[i]);
		if (c == PIO_FONTX) {
			print_quoted_hex(p, MIN(DEFAULT_STRLEN, cnts[i] * 32));
			if (cnts[i] * 32 > DEFAULT_STRLEN)
				printf("...");
		} else {
			printf("%p", p);
		}
		printf("}");
#if RETVAL_INJECTED
		if (c == GIO_FONTX) {
			printf(" => {charcount=%u, charheight=57005, chardata=",
			       cnts[i]);
			print_quoted_hex(p, MIN(DEFAULT_STRLEN, cnts[i] * 32));
			if (cnts[i] * 32 > DEFAULT_STRLEN)
				printf("...");
			printf("}");
		}
#endif
		printf(") = " RETVAL);
	}
}

/* GIO_CMAP, PIO_CMAP */
static void
check_cmap(unsigned int c, const char *s)
{
	char *cmap = tail_alloc(48);

	fill_memory(cmap, 48);

	check_null_invalid(c, s);

	sys_ioctl(-1, c, (uintptr_t) (cmap + 1));
	printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c, s));
	if ((c == PIO_CMAP || RETVAL_INJECTED) && (DEFAULT_STRLEN == 32)) {
		printf("\"");
		for (unsigned int i = 0; i < MIN(DEFAULT_STRLEN, 48); i++)
			printf("\\x%x", 0x81 + i);
		printf("\"...");
	} else {
		printf("%p", cmap + 1);
	}
	printf(") = " RETVAL);

	sys_ioctl(-1, c, (uintptr_t) cmap);
	printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c, s));
	if (c == PIO_CMAP || RETVAL_INJECTED) {
		printf("\"");
		for (unsigned int i = 0; i < MIN(DEFAULT_STRLEN, 48); i++)
			printf("\\x%x", 0x80 + i);
#if DEFAULT_STRLEN == 32
		printf("\"...");
#else
		printf("\"");
#endif
	} else {
		printf("%p", cmap);
	}
	printf(") = " RETVAL);
}

/* KDGKBDIACRUC, KDSKBDIACRUC */
static void
check_diacruc(unsigned int c, const char *s)
{
	struct kbdiacrsuc *diacrs0 = tail_alloc(sizeof(diacrs0->kb_cnt));
	struct kbdiacrsuc *diacrs1 = tail_alloc(sizeof(diacrs1->kb_cnt) +
						4 * sizeof(struct kbdiacruc));
	struct kbdiacrsuc *diacrs2 = tail_alloc(sizeof(*diacrs2));

	int saved_errno;

	check_null_invalid(c, s);

	for (size_t i = 0; i < 2; i++) {
		diacrs0->kb_cnt = i;
		sys_ioctl(-1, c, (uintptr_t) diacrs0);
		saved_errno = errno;
		printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c, s));
		if (RETVAL_INJECTED || c == KDSKBDIACRUC) {
			printf("{kb_cnt=%zu, kbdiacruc=", i);
			if (i)
				printf("%p}", diacrs0->kbdiacruc);
			else
				printf("[]}");
		} else {
			printf("%p", diacrs0);
		}
		errno = saved_errno;
		printf(") = " RETVAL);
	}

	fill_memory32(diacrs1->kbdiacruc, 4 * sizeof(struct kbdiacruc));
	for (size_t i = 0; i < 7; i++) {
		diacrs1->kb_cnt = i;
		sys_ioctl(-1, c, (uintptr_t) diacrs1);
		saved_errno = errno;
		printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c, s));
		if (RETVAL_INJECTED || c == KDSKBDIACRUC) {
			printf("{kb_cnt=%zu, kbdiacruc=[", i);
			for (size_t j = 0; j < MIN(i, 4); j++)
				printf("%s{diacr=%#x, base=%#x, result=%#x}",
				       j ? ", " : "",
				       (unsigned int) (0x80a0c0e0 + j * 3),
				       (unsigned int) (0x80a0c0e1 + j * 3),
				       (unsigned int) (0x80a0c0e2 + j * 3));

			if (i > 4) {
				printf(", ... /* %p */",
				       diacrs1->kbdiacruc + 4);
			}
			printf("]}");
		} else {
			printf("%p", diacrs1);
		}
		errno = saved_errno;
		printf(") = " RETVAL);
	}

	fill_memory32(diacrs2->kbdiacruc, sizeof(diacrs2->kbdiacruc));

	for (size_t i = ARRAY_SIZE(diacrs2->kbdiacruc) - 1;
	     i < ARRAY_SIZE(diacrs2->kbdiacruc) + 3; i++) {
		diacrs2->kb_cnt = i;
		sys_ioctl(-1, c, (uintptr_t) diacrs2);
		printf("ioctl(-1, " XLAT_FMT ", ", XLAT_SEL(c, s));
		saved_errno = errno;
		if (RETVAL_INJECTED || c == KDSKBDIACRUC) {
			printf("{kb_cnt=%zu, kbdiacruc=[", i);
			for (size_t j = 0;
			     j < MIN(i, MIN(ARRAY_SIZE(diacrs2->kbdiacruc),
					    DEFAULT_STRLEN)); j++)
				printf("%s{diacr=%#x, base=%#x, result=%#x}",
				       j ? ", " : "",
				       (unsigned int) (0x80a0c0e0 + j * 3),
				       (unsigned int) (0x80a0c0e1 + j * 3),
				       (unsigned int) (0x80a0c0e2 + j * 3));

			if (i > MIN(DEFAULT_STRLEN,
				    ARRAY_SIZE(diacrs2->kbdiacruc)))
				printf(", ...");
			printf("]}");
		} else {
			printf("%p", diacrs2);
		}
		errno = saved_errno;
		printf(") = " RETVAL);
	}
}

int
main(int argc, char *argv[])
{
	static const kernel_ulong_t magic =
		(kernel_ulong_t) 0xdeadbeefbadc0dedULL;

	static const uint32_t unknown_ioctls[] = {
		0xfffffff1, 0xc0007fff, 0xfffb800c, 0xfff8c000,
		0xffffffff, 0xffffffff, 0xffffffff, 0xf3ffffff,
	};

	enum { MAP_ELEM_BIT = sizeof(unknown_ioctls[0]) * CHAR_BIT };

	long rc;

#if RETVAL_INJECTED
	unsigned long num_skip;
	bool locked = false;

	if (argc < 2)
		error_msg_and_fail("Usage: %s NUM_SKIP", argv[0]);

	num_skip = strtoul(argv[1], NULL, 0);

	for (unsigned int i = 0; i < num_skip; i++) {
		long rc = sys_ioctl(-1, KDGETLED, 0);
		printf("ioctl(-1, " XLAT_FMT ", NULL) = %s%s\n",
		       XLAT_ARGS(KDGETLED), sprintrc(rc),
		       rc == 42 ? " (INJECTED)" : "");

		if (rc != 42)
			continue;

		locked = true;
		break;
	}

	if (!locked)
		error_msg_and_fail("Have not locked on ioctl(-1"
				   ", KDGETLED, NULL) returning 42");
#endif

	for (size_t i = 0; i < ARRAY_SIZE(unknown_ioctls); i++) {
		for (size_t j = 0; j < MAP_ELEM_BIT; j++) {
			if (!((unknown_ioctls[i] >> j) & 1))
				continue;

			const unsigned int id = i * MAP_ELEM_BIT + j;

			sys_ioctl(-1, 'K' << 8 | id, magic);
			printf("ioctl(-1, "
			       NABBR("%#x") VERB(" /* ")
			       NRAW("_IOC(%s, 0x4b, %#x, 0)") VERB(" */")
			       ", %#lx) = " RETVAL,
#if XLAT_RAW || XLAT_VERBOSE
			       'K' << 8 | id,
#endif
#if !XLAT_RAW
			       _IOC_NONE ? "0" : "_IOC_NONE", id,
#endif
			       (unsigned long) magic);
		}
	}


	/* KIOCSOUND */
	sys_ioctl(-1, KIOCSOUND, 0);
	printf("ioctl(-1, " XLAT_FMT ", 0" NRAW(" /* off */") ") = " RETVAL,
	       XLAT_ARGS(KIOCSOUND));

	sys_ioctl(-1, KIOCSOUND, 1);
	printf("ioctl(-1, " XLAT_FMT ", 1" NRAW(" /* 1193182 Hz */")
	       ") = " RETVAL, XLAT_ARGS(KIOCSOUND));

	sys_ioctl(-1, KIOCSOUND, 440);
	printf("ioctl(-1, " XLAT_FMT ", 440" NRAW(" /* 2711 Hz */")
	       ") = " RETVAL, XLAT_ARGS(KIOCSOUND));

	sys_ioctl(-1, KIOCSOUND, 1193182);
	printf("ioctl(-1, " XLAT_FMT ", 1193182" NRAW(" /* 1 Hz */")
	       ") = " RETVAL, XLAT_ARGS(KIOCSOUND));

	sys_ioctl(-1, KIOCSOUND, 1193183);
	printf("ioctl(-1, " XLAT_FMT ", 1193183" NRAW(" /* off */")
	       ") = " RETVAL, XLAT_ARGS(KIOCSOUND));

	sys_ioctl(-1, KIOCSOUND,
		  (kernel_ulong_t) (0xbadc0ded00000000ULL | 2710));
	printf("ioctl(-1, " XLAT_FMT
#if SIZEOF_LONG == 4
	       ", 2710" NRAW(" /* 440 Hz */")
#else
	       ", 13464652297489353366" NRAW(" /* off */")
#endif
	       ") = " RETVAL, XLAT_ARGS(KIOCSOUND));

	sys_ioctl(-1, KIOCSOUND, (kernel_ulong_t) 0xbadc0deddeadfaceULL);
	printf("ioctl(-1, " XLAT_FMT
#if SIZEOF_LONG == 8
	       ", 13464652301225294542"
#else
	       ", 3735943886"
#endif
	       NRAW(" /* off */") ") = " RETVAL, XLAT_ARGS(KIOCSOUND));

	/* KDMKTONE */
	sys_ioctl(-1, KDMKTONE, 0);
	printf("ioctl(-1, " XLAT_FMT ", 0" NRAW(" /* off */") ") = " RETVAL,
	       XLAT_ARGS(KDMKTONE));

	sys_ioctl(-1, KDMKTONE, 440);
	printf("ioctl(-1, " XLAT_FMT ", 440" NRAW(" /* off */") ") = " RETVAL,
	       XLAT_ARGS(KDMKTONE));

	sys_ioctl(-1, KDMKTONE, 0xffff);
	printf("ioctl(-1, " XLAT_FMT ", 65535" NRAW(" /* off */") ") = " RETVAL,
	       XLAT_ARGS(KDMKTONE));

	sys_ioctl(-1, KDMKTONE, 0x10000);
	printf("ioctl(-1, " XLAT_FMT ", 1<<16|0" NRAW(" /* off */")
	       ") = " RETVAL, XLAT_ARGS(KDMKTONE));

	sys_ioctl(-1, KDMKTONE,
		  (kernel_ulong_t) (0xbadc0ded00000000ULL | 0x10001));
	printf("ioctl(-1, " XLAT_FMT ", 1<<16|1" NRAW(" /* 1193182 Hz, 1 ms */")
	       ") = " RETVAL, XLAT_ARGS(KDMKTONE));

	sys_ioctl(-1, KDMKTONE, 0x1ffff);
	printf("ioctl(-1, " XLAT_FMT ", 1<<16|65535" NRAW(" /* 18 Hz, 1 ms */")
	       ") = " RETVAL, XLAT_ARGS(KDMKTONE));

	sys_ioctl(-1, KDMKTONE, (kernel_ulong_t) 0xbadc0deddeadfaceULL);
	printf("ioctl(-1, " XLAT_FMT ", 57005<<16|64206"
	       NRAW(" /* 18 Hz, 57005 ms */") ") = " RETVAL,
	       XLAT_ARGS(KDMKTONE));


	/* KDGETLED */
	static const struct arg_val led_vecs[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1, "LED_SCR") },
		{ ARG_XLAT_KNOWN(0x7, "LED_SCR|LED_NUM|LED_CAP") },
		{ ARG_XLAT_KNOWN(0xfe, "LED_NUM|LED_CAP|0xf8") },
		{ (kernel_ulong_t) 0xbadc0dedfeedfaf0ULL,
		  XLAT_UNKNOWN(0xf0, "LED_???") },
	};

	unsigned char *leds = tail_alloc(sizeof(*leds));

	check_null_invalid(ARG_STR(KDGETLED));

	for (size_t i = 0; i < ARRAY_SIZE(led_vecs); i++) {
		*leds = led_vecs[i].val;
		rc = sys_ioctl(-1, KDGETLED, (uintptr_t) leds);
		if (rc >= 0) {
			printf("ioctl(-1, " XLAT_FMT ", [%s]) = " RETVAL,
			       XLAT_ARGS(KDGETLED), led_vecs[i].str);
		} else {
			printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL,
			       XLAT_ARGS(KDGETLED), leds);
		}
	}


	/* KDSETLED */
	for (size_t i = 0; i < ARRAY_SIZE(led_vecs); i++) {
		sys_ioctl(-1, KDSETLED, led_vecs[i].val);
		printf("ioctl(-1, " XLAT_FMT ", %s) = " RETVAL,
		       XLAT_ARGS(KDSETLED), led_vecs[i].str);
	}

	sys_ioctl(-1, KDSETLED, (kernel_ulong_t) 0xdeadc0defeedfaceULL);
	printf("ioctl(-1, " XLAT_FMT ", %s) = " RETVAL,
	       XLAT_ARGS(KDSETLED), XLAT_STR(LED_NUM|LED_CAP|0xc8));


	/* KDGKBTYPE */
	static const struct arg_val kbt_vecs[] = {
		{ ARG_XLAT_UNKNOWN(0, "KB_???") },
		{ ARG_XLAT_KNOWN(0x1, "KB_84") },
		{ ARG_XLAT_KNOWN(0x2, "KB_101") },
		{ ARG_XLAT_KNOWN(0x3, "KB_OTHER") },
		{ ARG_XLAT_UNKNOWN(0x4, "KB_???") },
		{ (kernel_ulong_t) 0xbadc0dedcacafefeULL,
		  XLAT_UNKNOWN(0xfe, "KB_???") },
	};

	unsigned char *kbt = tail_alloc(sizeof(*kbt));

	check_null_invalid(ARG_STR(KDGKBTYPE));

	for (size_t i = 0; i < ARRAY_SIZE(kbt_vecs); i++) {
		*kbt = kbt_vecs[i].val;
		rc = sys_ioctl(-1, KDGKBTYPE, (uintptr_t) kbt);
		if (rc >= 0) {
			printf("ioctl(-1, " XLAT_FMT ", [%s]) = " RETVAL,
			       XLAT_ARGS(KDGKBTYPE), kbt_vecs[i].str);
		} else {
			printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL,
			       XLAT_ARGS(KDGKBTYPE), kbt);
		}
	}


	/* KDADDIO */
	static const struct arg_val iop_vecs[] = {
		{ ARG_STR(0) },
		{ ARG_STR(0x3b3) },
		{ ARG_STR(0x3b4) NRAW(" /* GPFIRST + 0 */") },
		{ ARG_STR(0x3c0) NRAW(" /* GPFIRST + 12 */") },
		{ ARG_STR(0x3df) NRAW(" /* GPFIRST + 43 */") },
		{ ARG_STR(0x3e0) },
		{ ARG_STR(0xdeadc0de) },
		{ (kernel_ulong_t) 0xbadc0dedfacefeedULL,
#if SIZEOF_LONG > 4
		  "0xbadc0dedfacefeed"
#else
		  "0xfacefeed"
#endif
		},
	};

	for (size_t i = 0; i < ARRAY_SIZE(iop_vecs); i++) {
		sys_ioctl(-1, KDADDIO, iop_vecs[i].val);
		printf("ioctl(-1, " XLAT_FMT ", %s) = " RETVAL,
		       XLAT_ARGS(KDADDIO), iop_vecs[i].str);
	}


	/* KDDELIO */
	for (size_t i = 0; i < ARRAY_SIZE(iop_vecs); i++) {
		sys_ioctl(-1, KDDELIO, iop_vecs[i].val);
		printf("ioctl(-1, " XLAT_FMT ", %s) = " RETVAL,
		       XLAT_ARGS(KDDELIO), iop_vecs[i].str);
	}


	/* KDENABIO */
	sys_ioctl(-1, KDENABIO, (kernel_ulong_t) 0xbadc0deddeadface);
	printf("ioctl(-1, " XLAT_FMT ") = " RETVAL, XLAT_ARGS(KDENABIO));


	/* KDDISABIO */
	sys_ioctl(-1, KDDISABIO, (kernel_ulong_t) 0xbadc0deddeadface);
	printf("ioctl(-1, " XLAT_FMT ") = " RETVAL, XLAT_ARGS(KDDISABIO));


	/* KDSETMODE */
	static const struct arg_val mode_vecs[] = {
		{ ARG_XLAT_KNOWN(0, "KD_TEXT") },
		{ ARG_XLAT_KNOWN(0x1, "KD_GRAPHICS") },
		{ ARG_XLAT_KNOWN(0x3, "KD_TEXT1") },
		{ ARG_XLAT_UNKNOWN(0x4, "KD_???") },
		{ (kernel_ulong_t) 0xbadc0dedcacafefeULL,
		  "0xcacafefe" NRAW(" /* KD_??? */") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(mode_vecs); i++) {
		sys_ioctl(-1, KDSETMODE, mode_vecs[i].val);
		printf("ioctl(-1, " XLAT_FMT ", %s) = " RETVAL,
		       XLAT_ARGS(KDSETMODE), mode_vecs[i].str);
	}


	/* KDGETMODE */
	unsigned int *mode = tail_alloc(sizeof(*mode));

	check_null_invalid(ARG_STR(KDGETMODE));

	for (size_t i = 0; i < ARRAY_SIZE(mode_vecs); i++) {
		*mode = mode_vecs[i].val;
		rc = sys_ioctl(-1, KDGETMODE, (uintptr_t) mode);
		if (rc >= 0) {
			printf("ioctl(-1, " XLAT_FMT ", [%s]) = " RETVAL,
			       XLAT_ARGS(KDGETMODE), mode_vecs[i].str);
		} else {
			printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL,
			       XLAT_ARGS(KDGETMODE), mode);
		}
	}


	/* KDMAPDISP */
	sys_ioctl(-1, KDMAPDISP, (kernel_ulong_t) 0xbadc0deddeadface);
	printf("ioctl(-1, " XLAT_FMT ") = " RETVAL, XLAT_ARGS(KDMAPDISP));


	/* KDUNMAPDISP */
	sys_ioctl(-1, KDUNMAPDISP, (kernel_ulong_t) 0xbadc0deddeadface);
	printf("ioctl(-1, " XLAT_FMT ") = " RETVAL, XLAT_ARGS(KDUNMAPDISP));


	/* GIO_SCRNMAP */
	check_scrnmap(ARG_STR(GIO_SCRNMAP));


	/* PIO_SCRNMAP */
	check_scrnmap(ARG_STR(PIO_SCRNMAP));


	/* KDGKBMODE */
	static const struct arg_val kbmode_vecs[] = {
		{ ARG_XLAT_UNKNOWN(-1, "K_???") },
		{ ARG_XLAT_KNOWN(0, "K_RAW") },
		{ ARG_XLAT_KNOWN(1, "K_XLATE") },
		{ ARG_XLAT_KNOWN(4, "K_OFF") },
		{ ARG_XLAT_UNKNOWN(5, "K_???") },
		{ (kernel_ulong_t) 0xbadc0dedfeeddeadULL,
		  XLAT_UNKNOWN(-17965395, "K_???") },
	};

	check_null_invalid(ARG_STR(KDGKBMODE));

	for (size_t i = 0; i < ARRAY_SIZE(kbmode_vecs); i++) {
		*mode = kbmode_vecs[i].val;
		rc = sys_ioctl(-1, KDGKBMODE, (uintptr_t) mode);
		if (rc >= 0) {
			printf("ioctl(-1, " XLAT_FMT ", [%s]) = " RETVAL,
			       XLAT_ARGS(KDGKBMODE), kbmode_vecs[i].str);
		} else {
			printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL,
			       XLAT_ARGS(KDGKBMODE), mode);
		}
	}


	/* KDSKBMODE */
	for (size_t i = 0; i < ARRAY_SIZE(kbmode_vecs); i++) {
		sys_ioctl(-1, KDSKBMODE, kbmode_vecs[i].val);
		printf("ioctl(-1, " XLAT_FMT ", %s) = " RETVAL,
		       XLAT_ARGS(KDSKBMODE), kbmode_vecs[i].str);
	}


	/* KDGKBENT */
	check_kbdent(ARG_STR(KDGKBENT));


	/* KDSKBENT */
	check_kbdent(ARG_STR(KDSKBENT));


	/* KDGKBSENT */
	check_kbdsent(ARG_STR(KDGKBSENT));


	/* KDSKBSENT */
	check_kbdsent(ARG_STR(KDSKBSENT));


	/* KDGKBDIACR */
	check_diacr(ARG_STR(KDGKBDIACR));


	/* KDSKBDIACR */
	check_diacr(ARG_STR(KDSKBDIACR));


	/* KDGETKEYCODE */
	check_xetkeycode(ARG_STR(KDGETKEYCODE));


	/* KDSETKEYCODE */
	check_xetkeycode(ARG_STR(KDSETKEYCODE));


	/* KDSIGACCEPT */
	static const struct {
		kernel_ulong_t val;
		const char *str;
	} sigaccept_vecs[] = {
		{ (kernel_ulong_t) -1ULL,
		  SIZEOF_LONG == 8 ? "18446744073709551615" : "4294967295" },
		{ 0, "0" },
		{ ARG_XLAT_KNOWN(SIGHUP, "SIGHUP") },
		{ ARG_XLAT_KNOWN(SIGUSR1, "SIGUSR1") },
		{ ARG_XLAT_KNOWN(32, "SIGRTMIN") },
		{ ARG_XLAT_KNOWN(33, "SIGRT_1") },
		{ ARG_STR(128) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(sigaccept_vecs); i++) {
		sys_ioctl(-1, KDSIGACCEPT, sigaccept_vecs[i].val);
		printf("ioctl(-1, " XLAT_FMT ", %s) = " RETVAL,
		       XLAT_ARGS(KDSIGACCEPT), sigaccept_vecs[i].str);
	}


	/* KDKBDREP */
	check_kbdrep(ARG_STR(KDKBDREP));


	/* GIO_FONT */
	check_font(ARG_STR(GIO_FONT));


	/* PIO_FONT */
	check_font(ARG_STR(PIO_FONT));


	/* KDGKBMETA */
	static const struct {
		kernel_ulong_t arg;
		const char *str;
	} meta_vecs[] = {
		{ ARG_XLAT_UNKNOWN(0, "K_???") },
		{ ARG_XLAT_UNKNOWN(0x1, "K_???") },
		{ ARG_XLAT_UNKNOWN(0x2, "K_???") },
		{ ARG_XLAT_KNOWN(0x3, "K_METABIT") },
		{ ARG_XLAT_KNOWN(0x4, "K_ESCPREFIX") },
		{ ARG_XLAT_UNKNOWN(0x5, "K_???") },
		{ (kernel_ulong_t) 0xdeadfacebeeffeedULL,
		  "0xbeeffeed" NRAW(" /* K_??? */") },
	};
	int *meta = tail_alloc(sizeof(*meta));

	check_null_invalid(ARG_STR(KDGKBMETA));

	for (size_t i = 0; i < ARRAY_SIZE(meta_vecs); i++) {
		*meta = meta_vecs[i].arg;
		sys_ioctl(-1, KDGKBMETA, (uintptr_t) meta);
		printf("ioctl(-1, " XLAT_FMT ", ", XLAT_ARGS(KDGKBMETA));
#if RETVAL_INJECTED
		printf("[%s]", meta_vecs[i].str);
#else
		printf("%p", meta);
#endif
		printf(") = " RETVAL);
	}


	/* KDSKBMETA */
	for (size_t i = 0; i < ARRAY_SIZE(meta_vecs); i++) {
		sys_ioctl(-1, KDSKBMETA, meta_vecs[i].arg);
		printf("ioctl(-1, " XLAT_FMT ", %s) = " RETVAL,
		       XLAT_ARGS(KDSKBMETA), meta_vecs[i].str);
	}


	/* KDGKBLED */
	static const struct arg_val kbled_vecs[] = {
		{ ARG_STR(0) },
		{ ARG_XLAT_KNOWN(0x1, "LED_SCR") },
		{ ARG_XLAT_KNOWN(0x7, "LED_SCR|LED_NUM|LED_CAP") },
		{ ARG_XLAT_KNOWN(0x10, "LED_SCR<<4") },
		{ ARG_XLAT_KNOWN(0x70, "LED_SCR<<4|LED_NUM<<4|LED_CAP<<4") },
		{ ARG_XLAT_KNOWN(0xfe, "LED_NUM|LED_CAP|LED_SCR<<4|LED_NUM<<4"
				       "|LED_CAP<<4|0x88") },
		{ (kernel_ulong_t) 0xbadc0dedfeedfa88ULL,
		  XLAT_UNKNOWN(0x88, "LED_???") },
	};

	unsigned char *kbleds = tail_alloc(sizeof(*kbleds));

	check_null_invalid(ARG_STR(KDGKBLED));

	for (size_t i = 0; i < ARRAY_SIZE(kbled_vecs); i++) {
		*kbleds = kbled_vecs[i].val;
		rc = sys_ioctl(-1, KDGKBLED, (uintptr_t) kbleds);
		if (rc >= 0) {
			printf("ioctl(-1, " XLAT_FMT ", [%s]) = " RETVAL,
			       XLAT_ARGS(KDGKBLED), kbled_vecs[i].str);
		} else {
			printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL,
			       XLAT_ARGS(KDGKBLED), kbleds);
		}
	}


	/* KDSKBLED */
	for (size_t i = 0; i < ARRAY_SIZE(kbled_vecs); i++) {
		sys_ioctl(-1, KDSKBLED, kbled_vecs[i].val);
		printf("ioctl(-1, " XLAT_FMT ", %s) = " RETVAL,
		       XLAT_ARGS(KDSKBLED), kbled_vecs[i].str);
	}


	/* GIO_UNIMAP */
	check_unimap(ARG_STR(GIO_UNIMAP));


	/* PIO_UNIMAP */
	check_unimap(ARG_STR(PIO_UNIMAP));


	/* PIO_UNIMAPCLR */
	struct unimapinit *umi = tail_alloc(sizeof(*umi));

	check_null_invalid(ARG_STR(PIO_UNIMAPCLR));

	sys_ioctl(-1, PIO_UNIMAPCLR, (uintptr_t) umi + 2);
	printf("ioctl(-1, " XLAT_FMT ", %p) = " RETVAL,
	       XLAT_ARGS(PIO_UNIMAPCLR), (char *) umi + 2);

	memset(umi, 0, sizeof(*umi));
	sys_ioctl(-1, PIO_UNIMAPCLR, (uintptr_t) umi);
	printf("ioctl(-1, " XLAT_FMT ", {advised_hashsize=0"
	       ", advised_hashstep=0, advised_hashlevel=0}) = " RETVAL,
	       XLAT_ARGS(PIO_UNIMAPCLR));

	fill_memory16(umi, sizeof(*umi));
	sys_ioctl(-1, PIO_UNIMAPCLR, (uintptr_t) umi);
	printf("ioctl(-1, " XLAT_FMT ", {advised_hashsize=32960"
	       ", advised_hashstep=32961, advised_hashlevel=32962}) = " RETVAL,
	       XLAT_ARGS(PIO_UNIMAPCLR));


	/* GIO_UNISCRNMAP */
	check_uniscrnmap(ARG_STR(GIO_UNISCRNMAP));


	/* PIO_UNISCRNMAP */
	check_uniscrnmap(ARG_STR(PIO_UNISCRNMAP));


	/* GIO_FONTX */
	check_fontx(ARG_STR(GIO_FONTX));


	/* PIO_FONTX */
	check_fontx(ARG_STR(GIO_FONTX));


	/* PIO_FONTRESET */
	sys_ioctl(-1, PIO_FONTRESET, (kernel_ulong_t) 0xbadc0deddeadface);
	printf("ioctl(-1, " XLAT_FMT ") = " RETVAL, XLAT_ARGS(PIO_FONTRESET));


	/* GIO_CMAP */
	check_cmap(ARG_STR(GIO_CMAP));


	/* PIO_CMAP */
	check_cmap(ARG_STR(PIO_CMAP));


	/* KDFONTOP */
	struct console_font_op *cfo = tail_alloc(sizeof(*cfo));
	unsigned char *cfo_data = tail_alloc(2048);
	unsigned char *cfo_data_end = cfo_data + 2048;

	fill_memory(cfo_data, 2048);

	check_null_invalid(ARG_STR(KDFONTOP));

	cfo->op = 4;
	cfo->flags = 0xdeadbeef;
	cfo->width = 0xbadc0ded;
	cfo->height = 0xfacecafe;
	cfo->charcount = 0xdadfaded;
	cfo->data = NULL;
	sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
	printf("ioctl(-1, " XLAT_FMT ", {op=0x4" NRAW(" /* KD_FONT_OP_??? */")
	       ", flags=" XLAT_KNOWN(0xdeadbeef, "KD_FONT_FLAG_DONT_RECALC"
						 "|KD_FONT_FLAG_OLD|0x5eadbeee")
	       ", width=3134983661, height=4207856382, charcount=3672092141"
	       ", data=NULL}"
#if RETVAL_INJECTED
	       " => {width=3134983661, height=4207856382, charcount=3672092141"
	       ", data=NULL}"
#endif
	       ") = " RETVAL, XLAT_ARGS(KDFONTOP));

	cfo->op = 0xbeefface;
	cfo->flags = 0x5a1ecafe;;
	cfo->data = (unsigned char *) cfo;
	sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
	printf("ioctl(-1, " XLAT_FMT ", {op=0xbeefface"
	       NRAW(" /* KD_FONT_OP_??? */")
	       ", flags=0x5a1ecafe" NRAW(" /* KD_FONT_FLAG_??? */")
	       ", width=3134983661, height=4207856382, charcount=3672092141"
	       ", data=%p}"
#if RETVAL_INJECTED
	       " => {width=3134983661, height=4207856382, charcount=3672092141"
	       ", data=%p}"
#endif
	       ") = " RETVAL, XLAT_ARGS(KDFONTOP), cfo
#if RETVAL_INJECTED
	       , cfo
#endif
	       );

	static const struct strval32 kdfont_ops[] = {
		{ ARG_XLAT_KNOWN(0, "KD_FONT_OP_SET") },
		{ ARG_XLAT_KNOWN(0x1, "KD_FONT_OP_GET") },
	};

	for (size_t i = 0; i < ARRAY_SIZE(kdfont_ops); i++) {
		cfo->op = kdfont_ops[i].val;
		cfo->flags = 1;
		cfo->width = 0;
		cfo->height = 0;
		cfo->charcount = 0;
		cfo->data = NULL;
		sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
		printf("ioctl(-1, " XLAT_FMT ", {op=%s, flags="
		       XLAT_FMT ", width=0, height=0, charcount=0, data=NULL}",
		       XLAT_ARGS(KDFONTOP), kdfont_ops[i].str,
		       XLAT_ARGS(KD_FONT_FLAG_DONT_RECALC));
		if (kdfont_ops[i].val == KD_FONT_OP_GET && RETVAL_INJECTED) {
			printf(" => {width=0, height=0, charcount=0"
			       ", data=NULL}");
		}
		printf(") = " RETVAL);

		cfo->data = cfo_data_end;
		for (size_t j = 0; j < 2; j++) {
			cfo->charcount = j;
			sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
			printf("ioctl(-1, " XLAT_FMT ", {op=%s, flags="
			       XLAT_FMT ", width=0, height=0, charcount=%zu"
			       ", data=",
			       XLAT_ARGS(KDFONTOP), kdfont_ops[i].str,
			       XLAT_ARGS(KD_FONT_FLAG_DONT_RECALC), j);
			if (kdfont_ops[i].val == KD_FONT_OP_SET)
				printf("\"\"");
			else
				printf("%p", cfo_data_end);
#if RETVAL_INJECTED
			if (kdfont_ops[i].val == KD_FONT_OP_GET) {
				printf("} => {width=0, height=0, charcount=%zu"
				       ", data=\"\"", j);
			}
#endif
			printf("}) = " RETVAL);

		}

		cfo->flags = 0;
		cfo->width = 1;
		cfo->data = cfo_data_end - 31;
		sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
		printf("ioctl(-1, " XLAT_FMT ", {op=%s, flags=0, width=1"
		       ", height=0, charcount=1, data=%p}",
		       XLAT_ARGS(KDFONTOP), kdfont_ops[i].str,
		       cfo_data_end - 31);
		if (kdfont_ops[i].val == KD_FONT_OP_GET && RETVAL_INJECTED) {
			printf(" => {width=1, height=0, charcount=1, data=%p}",
			       cfo_data_end - 31);
		}
		printf(") = " RETVAL);

		cfo->data = cfo_data_end - 32;
		sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
		printf("ioctl(-1, " XLAT_FMT ", {op=%s, flags=0, width=1"
		       ", height=0, charcount=1, data=",
		       XLAT_ARGS(KDFONTOP), kdfont_ops[i].str);
		if (kdfont_ops[i].val == KD_FONT_OP_SET)
			print_quoted_hex(cfo_data_end - 32, 32);
		else
			printf("%p", cfo_data_end - 32);
		if (kdfont_ops[i].val == KD_FONT_OP_GET && RETVAL_INJECTED) {
			printf("} => {width=1, height=0, charcount=1, data=");
			print_quoted_hex(cfo_data_end - 32, 32);

		}
		printf("}) = " RETVAL);

		cfo->charcount = 32;
		cfo->data = cfo_data_end - 1023;
		sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
		printf("ioctl(-1, " XLAT_FMT ", {op=%s, flags=0, width=1"
		       ", height=0, charcount=32, data=",
		       XLAT_ARGS(KDFONTOP), kdfont_ops[i].str);
		if (kdfont_ops[i].val == KD_FONT_OP_SET && DEFAULT_STRLEN == 32)
		{
			print_quoted_hex(cfo_data_end - 1023, 32);
			printf("...");
		} else {
			printf("%p", cfo_data_end - 1023);
		}
		if (kdfont_ops[i].val == KD_FONT_OP_GET && RETVAL_INJECTED) {
			printf("} => {width=1, height=0, charcount=32, data=");
#if DEFAULT_STRLEN == 32
			print_quoted_hex(cfo_data_end - 1023, 32);
			printf("...");
#else
			printf("%p", cfo_data_end - 1023);
#endif

		}
		printf("}) = " RETVAL);

		cfo->data = cfo_data_end - 1024;
		sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
		printf("ioctl(-1, " XLAT_FMT ", {op=%s, flags=0, width=1"
		       ", height=0, charcount=32, data=",
		       XLAT_ARGS(KDFONTOP), kdfont_ops[i].str);
		if (kdfont_ops[i].val == KD_FONT_OP_SET) {
			print_quoted_hex(cfo_data_end - 1024, DEFAULT_STRLEN);
#if DEFAULT_STRLEN == 32
			printf("...");
#endif
		} else {
			printf("%p", cfo_data_end - 1024);
		}
		if (kdfont_ops[i].val == KD_FONT_OP_GET && RETVAL_INJECTED) {
			printf("} => {width=1, height=0, charcount=32, data=");
			print_quoted_hex(cfo_data_end - 1024, DEFAULT_STRLEN);
#if DEFAULT_STRLEN == 32
			printf("...");
#endif
		}
		printf("}) = " RETVAL);

		cfo->charcount = 256;
		cfo->data = cfo_data_end - 1025;
		sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
		printf("ioctl(-1, " XLAT_FMT ", {op=%s, flags=0, width=1"
		       ", height=0, charcount=256, data=",
		       XLAT_ARGS(KDFONTOP), kdfont_ops[i].str);
		if (kdfont_ops[i].val == KD_FONT_OP_SET) {
			print_quoted_hex(cfo_data_end - 1025, DEFAULT_STRLEN);
			printf("...");
		} else {
			printf("%p", cfo_data_end - 1025);
		}
		if (kdfont_ops[i].val == KD_FONT_OP_GET && RETVAL_INJECTED) {
			printf("} => {width=1, height=0, charcount=256, data=");
			print_quoted_hex(cfo_data_end - 1025, DEFAULT_STRLEN);
			printf("...");

		}
		printf("}) = " RETVAL);
	}

	cfo->op = 2;
	cfo->data = NULL;
	sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
	printf("ioctl(-1, " XLAT_FMT ", {op=" XLAT_FMT ", width=1, height=0"
	       ", data=NULL}"
#if RETVAL_INJECTED
	       " => {width=1, height=0}"
#endif
	       ") = " RETVAL,
	       XLAT_ARGS(KDFONTOP), XLAT_ARGS(KD_FONT_OP_SET_DEFAULT));

	cfo->data = cfo_data_end - 1;
	cfo->data[0] = '\0';
	sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
	printf("ioctl(-1, " XLAT_FMT ", {op=" XLAT_FMT ", width=1, height=0"
	       ", data=\"\"}"
#if RETVAL_INJECTED
	       " => {width=1, height=0}"
#endif
	       ") = " RETVAL,
	       XLAT_ARGS(KDFONTOP), XLAT_ARGS(KD_FONT_OP_SET_DEFAULT));

	cfo->data[0] = 'x';
	sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
	printf("ioctl(-1, " XLAT_FMT ", {op=" XLAT_FMT ", width=1, height=0"
	       ", data=%p}"
#if RETVAL_INJECTED
	       " => {width=1, height=0}"
#endif
	       ") = " RETVAL,
	       XLAT_ARGS(KDFONTOP), XLAT_ARGS(KD_FONT_OP_SET_DEFAULT),
	       cfo_data_end - 1);

	cfo->width = 0xcafebeef;
	cfo->height = 0xbea7bee5;
	cfo->data = cfo_data_end - 32;
	strcpy((char *) cfo->data,
	       "\1\2\3\r\n\t\v\f\\\"OH\377HAI\7\10\02101234567890x");
	for (size_t j = 0; j < 2; j++) {
		sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
		printf("ioctl(-1, " XLAT_FMT ", {op=" XLAT_FMT
		       ", width=3405692655, height=3198664421, data=\"\\1\\2\\3"
		       "\\r\\n\\t\\v\\f\\\\\\\"OH\\377HAI\\7\\10\\0210123456789"
		       "0x\"%s}"
#if RETVAL_INJECTED
		       " => {width=3405692655, height=3198664421}"
#endif
		       ") = " RETVAL,
		       XLAT_ARGS(KDFONTOP), XLAT_ARGS(KD_FONT_OP_SET_DEFAULT),
		       j ? "..." : "");

		cfo->data[31] = 'y';
	}

	cfo->op = 3;
	cfo->height = 0;
	sys_ioctl(-1, KDFONTOP, (uintptr_t) cfo);
	printf("ioctl(-1, " XLAT_FMT ", {op=" XLAT_FMT ", height=0}) = " RETVAL,
	       XLAT_ARGS(KDFONTOP), XLAT_ARGS(KD_FONT_OP_COPY));


	/* KDGKBDIACRUC */
	check_diacruc(ARG_STR(KDGKBDIACRUC));


	/* KDSKBDIACRUC */
	check_diacruc(ARG_STR(KDSKBDIACRUC));


	puts("+++ exited with 0 +++");
	return 0;
}
