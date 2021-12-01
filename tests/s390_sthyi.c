/*
 * Check decoding of s390_sthyi syscall.
 *
 * Copyright (c) 2018-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#if defined HAVE_ICONV_H && defined HAVE_ICONV_OPEN && defined __NR_s390_sthyi

# include <errno.h>
# include <iconv.h>
# include <inttypes.h>
# include <stdint.h>
# include <stdio.h>
# include <unistd.h>

# include <sys/user.h>

# define EBCDIC_MAX_LEN 16

# ifndef VERBOSE
#  define VERBOSE 0
# endif

static bool
print_0x8(const char *prefix, unsigned char *buf, unsigned int offs, bool zero)
{
	if (!zero && !buf[offs])
		return false;

	printf("%s=%#02hhx", prefix, buf[offs]);

	return true;
}

# if VERBOSE
static bool
print_u8(const char *prefix, unsigned char *buf, unsigned int offs, bool zero)
{
	if (!zero && !buf[offs])
		return false;

	printf("%s=%hhu", prefix, buf[offs]);

	return true;
}
# endif

static bool
print_u16(const char *prefix, unsigned char *buf, unsigned int offs, bool zero)
{
	uint16_t val = *(uint16_t *) (buf + offs);

	if (!zero && !val)
		return false;

	printf("%s=%" PRIu16, prefix, val);

	return true;
}

static bool
print_x32(const char *prefix, unsigned char *buf, unsigned int offs, bool zero)
{
	uint32_t val = *(uint32_t *) (buf + offs);

	if (!zero && !val)
		return false;

	printf("%s=%#" PRIx32, prefix, val);

	return true;
}

static bool
print_weight(const char *prefix, unsigned char *buf, unsigned int offs,
	     bool zero)
{
	uint32_t val = *(uint32_t *) (buf + offs);

	if (print_x32(prefix, buf, offs, zero)) {
		if (val)
			printf(" /* %u %u/65536 cores */",
			       val >> 16, val & 0xFFFF);
		else
			printf(" /* unlimited */");

		return true;
	}

	return false;
}

static char *
ebcdic2ascii(unsigned char *ebcdic, size_t size)
{
	static char ascii_buf[EBCDIC_MAX_LEN];

	char *ebcdic_pos = (char *) ebcdic;
	char *ascii_pos = ascii_buf;
	size_t ebcdic_left = size;
	size_t ascii_left = size;
	size_t ret;

	iconv_t cd = iconv_open("ASCII", "EBCDICUS");

	if (size > sizeof(ascii_buf))
		error_msg_and_fail("ebcdic2ascii: EBCDIC string is too big: "
				   "%zu (maximum is %zu)",
				   size, sizeof(ascii_buf));
	if (cd == (iconv_t) -1)
		perror_msg_and_fail("ebcdic2ascii: unable to allocate a "
				    "conversion descriptor for converting "
				    "EBCDIC to ASCII");

	while ((ret = iconv(cd, &ebcdic_pos, &ebcdic_left,
	    &ascii_pos, &ascii_left)) == (size_t) -1) {
		switch (errno) {
		case EILSEQ:
		case EINVAL: /* That one is quite unexpected, actually */
			if (!ebcdic_left || !ascii_left)
				goto ebcdic2ascii_end;

			*ascii_pos++ = ' ';
			ebcdic_pos++;
			ebcdic_left--;

			break;

		case E2BIG:
			perror_msg_and_fail("ebcdic2ascii: ran out of "
					    "ASCII buffer unexpectedly");
		default:
			perror_msg_and_fail("ebcdic2ascii: unexpected error");
		}
	}

ebcdic2ascii_end:
	iconv_close(cd);

	if (ebcdic_left != ascii_left)
		error_msg_and_fail("ebcdic2ascii: ASCII string differs in size "
				   "from EBCDIC");

	return ascii_buf;
}

# if VERBOSE
static bool
is_empty(unsigned char *ptr, size_t size)
{
	size_t i;

	for (i = 0; !*ptr && i < size; ptr++, i++)
		;

	return i == size;
}
# endif

static bool
print_ebcdic(const char *prefix, unsigned char *addr, unsigned int offs,
	     size_t size, bool zero, bool blank)
{
	const char *ascii = ebcdic2ascii(addr + offs, size);

	if (!zero) {
		size_t i;

		for (i = 0; (addr[offs + i] == (blank ? 64 : 0)) && (i < size);
		    i++)
			;

		if (i == size)
			return false;
	}

	printf("%s=", prefix);
	print_quoted_hex((char *) (addr + offs), size);
	printf(" /* ");
	print_quoted_memory(ascii, size);
	printf(" */");

	return true;
}

# if VERBOSE
static void
print_funcs(unsigned char *addr, unsigned int offs)
{
	bool cont;
	const uint8_t *funcs = addr + offs;

	if (!funcs[0])
		return;

	printf(" /* ");

	if (funcs[0] & 0x80) {
		printf("0: Obtain CPU Capacity Info");
		cont = true;
	}

	if (funcs[0] & 0x40)
		printf("%s1: Hypervisor Environment Info",
		       cont ? ", " : (cont = true, ""));
	if (funcs[0] & 0x20)
		printf("%s2: Guest List",
		       cont ? ", " : (cont = true, ""));
	if (funcs[0] & 0x10)
		printf("%s3: Designated Guest Info",
		       cont ? ", " : (cont = true, ""));
	if (funcs[0] & 0x08)
		printf("%s4: Resource Pool List",
		       cont ? ", " : (cont = true, ""));
	if (funcs[0] & 0x04)
		printf("%s5: Designated Resource Pool Information",
		       cont ? ", " : (cont = true, ""));
	if (funcs[0] & 0x02)
		printf("%s6: Resource Pool Member List",
		       cont ? ", " : (cont = true, ""));

	printf(" */");
}
# endif

static void
print_hypervisor_header(unsigned char *buf, int level, unsigned int offs_pos,
			unsigned int len_pos, bool mt)
{
	uint16_t offs = *(uint16_t *) (buf + offs_pos);
	uint16_t hdr_size = *(uint16_t *) (buf + len_pos);
	unsigned char *cur;

	if (!offs)
		return;
	if (hdr_size < 32)
		error_msg_and_fail("sthyi: hypervisor %d section is too small "
			           "(got %hu, 32 expected)", level, hdr_size);

	cur = buf + offs;

	printf(", /* hypervisor %d */ {infyflg1", level);
	print_0x8("", cur, 0, true);
# if VERBOSE
	if (cur[0]) {
		bool printed = false;

		printf(" /* ");
		if (cur[0] & 0x80) {
			printf("0x80 - guest CPU usage had limiting is using "
			       "the consumption method");
			printed = true;
		}
		if (cur[0] & 0x40) {
			if (printed)
				printf(", ");
			printf("0x40 - LIMITHARD caps use prorated core time "
			       "for capping");
			printed = true;
		}
		if (cur[0] & 0x20) {
			if (printed)
				printf(", ");
			printf("0x20 - hypervisor is MT-enabled");
			printed = true;
		}
		if (cur[0] & 0x1F) {
			if (printed)
				printf(", ");
			printf("%#hhx - ???", cur[0] & 0x1F);
		}
		printf(" */");
	}

	print_0x8(", infyflg2", cur, 1, false);
	print_0x8(", infyval1", cur, 2, false);
	print_0x8(", infyval2", cur, 3, false);

	print_u8(", infytype", cur, 4, true);
	if (cur[4] == 1)
		printf(" /* z/VM is the hypervisor */");
	else
		printf(" /* unknown hypervisor type */");

	if (cur[5])
		printf(", reserved_1__=\"\\x%#02hhx\"", cur[5]);

	print_u8(", infycpt",  cur, 6, mt);
	print_u8(", infyiflt", cur, 7, mt);
# endif /* !VERBOSE */

	print_ebcdic(", infysyid", cur, 8,  8, VERBOSE, true);
	print_ebcdic(", infyclnm", cur, 16, 8, VERBOSE, true);

	print_u16(", infyscps", cur, 24, VERBOSE);
	print_u16(", infydcps", cur, 26, VERBOSE);
	print_u16(", infysifl", cur, 28, VERBOSE);
	print_u16(", infydifl", cur, 30, VERBOSE);

# if VERBOSE
	if (hdr_size >= 48) {
		printf(", infyinsf=");
		print_quoted_hex((char *) (cur + 32), 8);
		print_funcs(cur, 32);

		printf(", infyautf=");
		print_quoted_hex((char *) (cur + 40), 8);
		print_funcs(cur, 40);

		if (hdr_size > 48 && !is_empty(cur + 48, hdr_size - 48)) {
			printf(", ");
			print_quoted_hex((char *) (cur + 48), hdr_size - 48);
		}
	} else if (hdr_size > 32 && !is_empty(cur + 32, hdr_size - 32)) {
		printf(", ");
		print_quoted_hex((char *) (cur + 32), hdr_size - 32);
	}
# else /* !VERBOSE */
	printf(", ...");
# endif /* !VERBOSE */

	printf("}");
}

static void
print_guest_header(unsigned char *buf, int level, unsigned int offs_pos,
		   unsigned int len_pos)
{
	uint16_t offs = *(uint16_t *) (buf + offs_pos);
	uint16_t hdr_size = *(uint16_t *) (buf + len_pos);
	unsigned char *cur;

	if (!offs)
		return;
	if (hdr_size < 56)
		error_msg_and_fail("sthyi: guest %d section is too small "
			           "(got %hu, 56 expected)", level, hdr_size);

	cur = buf + offs;

	printf(", /* guest %d */ {infgflg1", level);
	print_0x8("", cur, 0, true);
# if VERBOSE
	if (cur[0]) {
		bool printed = false;

		printf(" /* ");
		if (cur[0] & 0x80) {
			printf("0x80 - guest is mobility enabled");
			printed = true;
		}
		if (cur[0] & 0x40) {
			if (printed)
				printf(", ");
			printf("0x40 - guest has multiple virtual CPU types");
			printed = true;
		}
		if (cur[0] & 0x20) {
			if (printed)
				printf(", ");
			printf("0x20 - guest CP dispatch type has LIMITHARD "
			       "cap");
			printed = true;
		}
		if (cur[0] & 0x10) {
			if (printed)
				printf(", ");
			printf("0x10 - guest IFL dispatch type has LIMITHARD "
			       "cap");
			printed = true;
		}
		if (cur[0] & 0x08) {
			if (printed)
				printf(", ");
			printf("0x08 - virtual CPs are thread dispatched");
			printed = true;
		}
		if (cur[0] & 0x04) {
			if (printed)
				printf(", ");
			printf("0x04 - virtual IFLs are thread dispatched");
			printed = true;
		}
		if (cur[0] & 0x3) {
			if (printed)
				printf(", ");
			printf("%#hhx - ???", cur[0] & 0x3);
		}
		printf(" */");
	}

	print_0x8(", infgflg2", cur, 1, false);
	print_0x8(", infgval1", cur, 2, false);
	print_0x8(", infgval2", cur, 3, false);
# endif /* !VERBOSE */

	print_ebcdic(", infgusid", cur, 4, 8, true, false);

	print_u16(", infgscps", cur, 12, VERBOSE);
	print_u16(", infgdcps", cur, 14, VERBOSE);

# if VERBOSE
	print_u8(", infgcpdt", cur, 16, true);
	if (cur[16] == 0)
		printf(" /* General Purpose (CP) */");
	else
		printf(" /* unknown */");

	if (cur[17] || cur[18] || cur[19])
		printf(", reserved_1__=\"\\x%#02hhx\\x%#02hhx\\x%#02hhx\"",
		       cur[17], cur[18], cur[19]);
# endif /* !VERBOSE */

	print_weight(", infgcpcc", cur, 20, VERBOSE);

	print_u16(", infgsifl", cur, 24, VERBOSE);
	print_u16(", infgdifl", cur, 26, VERBOSE);

# if VERBOSE
	print_u8(", infgifdt", cur, 28, true);
	if (cur[28] == 0)
		printf(" /* General Purpose (CP) */");
	else if (cur[28] == 3)
		printf(" /* Integrated Facility for Linux (IFL) */");
	else
		printf(" /* unknown */");

	if (cur[29] || cur[30] || cur[31])
		printf(", reserved_2__=\"\\x%#02hhx\\x%#02hhx\\x%#02hhx\"",
		       cur[29], cur[30], cur[31]);
# endif /* !VERBOSE */

	print_weight(", infgifcc", cur, 32, VERBOSE);

	print_0x8(", infgpflg", cur, 36, true);
# if VERBOSE
	if (cur[36]) {
		bool printed = false;

		printf(" /* ");
		if (cur[36] & 0x80) {
			printf("0x80 - CPU pool's CP virtual type has "
			       "LIMITHARD cap");
			printed = true;
		}
		if (cur[36] & 0x40) {
			if (printed)
				printf(", ");
			printf("0x40 - CPU pool's CP virtual type has "
			       "CAPACITY cap");
			printed = true;
		}
		if (cur[36] & 0x20) {
			if (printed)
				printf(", ");
			printf("0x20 - CPU pool's IFL virtual type has "
			       "LIMITHARD cap");
			printed = true;
		}
		if (cur[36] & 0x10) {
			if (printed)
				printf(", ");
			printf("0x10 - CPU pool's IFL virtual type has "
			       "CAPACITY cap");
			printed = true;
		}
		if (cur[36] & 0x08) {
			if (printed)
				printf(", ");
			printf("0x08 - CPU pool uses prorated core time");
			printed = true;
		}
		if (cur[36] & 0x7) {
			if (printed)
				printf(", ");
			printf("%#hhx - ???", cur[36] & 0x7);
		}
		printf(" */");
	}

	if (cur[37] || cur[38] || cur[39])
		printf(", reserved_3__=\"\\x%#02hhx\\x%#02hhx\\x%#02hhx\"",
		       cur[37], cur[38], cur[39]);

	print_ebcdic(", infgpnam", cur, 40, 8, false, true);

	print_weight(", infgpccc", cur, 48, true);
	print_weight(", infgpicc", cur, 52, true);

	if (hdr_size > 56 && !is_empty(cur + 56, hdr_size - 56)) {
		printf(", ");
		print_quoted_hex((char *) (cur + 56), hdr_size - 56);
	}
# else /* !VERBOSE */
	printf(", ...");
# endif /* !VERBOSE */

	printf("}");
}

static void
print_sthyi(unsigned char *buf)
{
	unsigned char *cur;
	uint16_t hdr_size;
	uint16_t offs;
	bool mt = false;

	hdr_size = *(uint16_t *) (buf + 10);
	if (hdr_size < 44)
		error_msg_and_fail("sthyi: header section is too small "
				   "(got %hu, >=44 expected)", hdr_size);

	/* INFHFLG1 */
	print_0x8("{/* header */ {infhflg1", buf, 0, true);
# if VERBOSE
	if (buf[0]) {
		bool printed = false;

		printf(" /* ");
		if (buf[0] & 0x80) {
			printf("0x80 - Global Performance Data unavailable");
			printed = true;
		}
		if (buf[0] & 0x40) {
			if (printed)
				printf(", ");
			printf("0x40 - One or more hypervisor levels below "
			       "this level does not support the STHYI "
			       "instruction");
			printed = true;
		}
		if (buf[0] & 0x20) {
			if (printed)
				printf(", ");
			printf("0x20 - Virtualization stack is incomplete");
			printed = true;
		}
		if (buf[0] & 0x10) {
			if (printed)
				printf(", ");
			printf("0x10 - Execution environment is not within a "
			       "logical partition");
			printed = true;
		}
		if (buf[0] & 0xF) {
			if (printed)
				printf(", ");
			printf("%#hhx - ???", buf[0] & 0xF);
		}
		printf(" */");
	}

	print_0x8(", infhflg2", buf, 1, false);
	print_0x8(", infhval1", buf, 2, false);
	print_0x8(", infhval2", buf, 3, false);

	/* Reserved */
	if (buf[4] || buf[5] || buf[6])
		printf(", reserved_1__=\"\\x%#02hhx\\x%#02hhx\\x%#02hhx\"",
		       buf[4], buf[5], buf[6]);

	print_u8(", infhygct", buf, 7, true);
	print_u16(", infhtotl", buf, 8, true);
	print_u16(", infhdln", buf, 10, true);
	print_u16(", infmoff", buf, 12, true);
	print_u16(", infmlen", buf, 14, true);
	print_u16(", infpoff", buf, 16, true);
	print_u16(", infplen", buf, 18, true);
	print_u16(", infhoff1", buf, 20, true);
	print_u16(", infhlen1", buf, 22, true);
	print_u16(", infgoff1", buf, 24, true);
	print_u16(", infglen1", buf, 26, true);
	print_u16(", infhoff2", buf, 28, true);
	print_u16(", infhlen2", buf, 30, true);
	print_u16(", infgoff2", buf, 32, true);
	print_u16(", infglen2", buf, 34, true);
	print_u16(", infhoff3", buf, 36, true);
	print_u16(", infhlen3", buf, 38, true);
	print_u16(", infgoff3", buf, 40, true);
	print_u16(", infglen3", buf, 42, true);

	if (hdr_size > 44 && !is_empty(buf + 44, hdr_size - 44)) {
		printf(", ");
		print_quoted_hex((char *) (buf + 44), hdr_size - 44);
	}
# else /* !VERBOSE */
	printf(", ...");
# endif /* !VERBOSE */

	printf("}");

	/* Machine header */
	offs = *(uint16_t *) (buf + 12);
	if (!offs)
		goto partition_hdr;

	hdr_size = *(uint16_t *) (buf + 14);
	if (hdr_size < 60)
		error_msg_and_fail("sthyi: machine section is too small "
				   "(got %hu, >=60 expected)", hdr_size);

	cur = buf + offs;

	printf(", /* machine */ {");

# if VERBOSE
	print_0x8("infmflg1", cur, 0, false);
	if (cur[0])
		printf(", ");
	print_0x8("infmflg2", cur, 1, false);
	if (cur[1])
		printf(", ");
# endif /* !VERBOSE */
	print_0x8("infmval1", cur, 2, true);

	bool cnt_valid = cur[2] & 0x80;
# if VERBOSE
	bool id_valid = cur[2] & 0x40;
	bool name_valid = cur[2] & 0x20;

	printf(" /* processor count validity: %d, machine ID validity: %d, "
	       "machine name validity: %d",
	       !!cnt_valid, !!id_valid, !!name_valid);
	if (cur[2] & 0x1F)
		printf(", %#hhx - ???", cur[2] & 0x1F);
	printf(" */");
	print_0x8(", infmval2", cur, 3, false);
# endif /* !VERBOSE */

	print_u16(", infmscps", cur, 4,  cnt_valid);
	print_u16(", infmdcps", cur, 6,  cnt_valid);
	print_u16(", infmsifl", cur, 8,  cnt_valid);
	print_u16(", infmdifl", cur, 10, cnt_valid);

# if VERBOSE
	print_ebcdic(", infmname", cur, 12, 8, name_valid, false);

	print_ebcdic(", infmtype", cur, 20, 4,  id_valid, false);
	print_ebcdic(", infmmanu", cur, 24, 16, id_valid, false);
	print_ebcdic(", infmseq",  cur, 40, 16, id_valid, false);
	print_ebcdic(", infmpman", cur, 56, 4,  id_valid, false);

	if (hdr_size >= 72) {
		if (cur[60] || cur[61] || cur[62] || cur[63])
			printf(", reserved_1__="
			       "\"\\x%#02hhx\\x%#02hhx\\x%#02hhx\\x%#02hhx\"",
			       cur[60], cur[61], cur[62], cur[63]);

		print_ebcdic(", infmplnm", cur, 64, 8, false, false);

		if (hdr_size > 72 && !is_empty(cur + 72, hdr_size - 72)) {
			printf(", ");
			print_quoted_hex((char *) (cur + 72), hdr_size - 72);
		}
	} else if (hdr_size > 60 && !is_empty(cur + 60, hdr_size - 60)) {
		printf(", ");
		print_quoted_hex((char *) (cur + 60), hdr_size - 60);
	}
# else /* !VERBOSE */
	printf(", ...");
# endif /* !VERBOSE */

	printf("}");

partition_hdr:
	/* Partition header */
	offs = *(uint16_t *) (buf + 16);
	if (!offs)
		goto hv_hdr;

	hdr_size = *(uint16_t *) (buf + 18);
	if (hdr_size < 40)
		error_msg_and_fail("sthyi: partition section is too small "
				   "(got %hu, >=40 expected)", hdr_size);

	cur = buf + offs;

	print_0x8(", /* partition */ {infpflg1", cur, 0, true);
	mt = !!(cur[0] & 0x80);
# if VERBOSE
	if (cur[0]) {
		bool printed = false;

		printf(" /* ");
		if (cur[0] & 0x80) {
			printf("0x80 - multithreading is enabled");
			printed = true;
		}
		if (cur[0] & 0x7F) {
			if (printed)
				printf(", ");
			printf("%#hhx - ???", cur[0] & 0x7F);
		}
		printf(" */");
	}
	print_0x8(", infpflg2", cur, 1, false);
# endif /* !VERBOSE */
	print_0x8(", infpval1", cur, 2, true);

	bool pcnt_valid  = cur[2] & 0x80;
	bool pid_valid   = cur[2] & 0x10;
# if VERBOSE
	bool pwcap_valid = cur[2] & 0x40;
	bool pacap_valid = cur[2] & 0x20;
	bool lpar_valid  = cur[2] & 0x08;
# endif /* !VERBOSE */

# if VERBOSE
	printf(" /* processor count validity: %d, partition weight-based "
	       "capacity validity: %d, partition absolute capacity validity: "
	       "%d, partition ID validity: %d, LPAR group absolute capacity "
	       "capping information validity: %d",
	       !!pcnt_valid, !!pwcap_valid, !!pacap_valid, !!pid_valid,
	       !!lpar_valid);
	if (cur[2] & 0x7)
		printf(", %#hhx - ???", cur[2] & 0x7);
	printf(" */");

	print_0x8(", infpval2", cur, 3, false);
# endif /* !VERBOSE */

	print_u16(", infppnum", cur, 4, pid_valid);

	print_u16(", infpscps", cur, 6,  pcnt_valid);
	print_u16(", infpdcps", cur, 8,  pcnt_valid);
	print_u16(", infpsifl", cur, 10, pcnt_valid);
	print_u16(", infpdifl", cur, 12, pcnt_valid);

# if VERBOSE
	if (cur[14] || cur[15])
		printf(", reserved_1__=\"\\x%#02hhx\\x%#02hhx\"",
		       cur[14], cur[15]);
# endif /* !VERBOSE */

	print_ebcdic(", infppnam", cur, 16, 8, pid_valid, false);

# if VERBOSE
	print_weight(", infpwbcp", cur, 24, pwcap_valid);
	print_weight(", infpabcp", cur, 28, pacap_valid);
	print_weight(", infpwbif", cur, 32, pwcap_valid);
	print_weight(", infpabif", cur, 36, pacap_valid);

	if (hdr_size >= 56) {
		if (print_ebcdic(", infplgnm", cur, 40, 8, false, false)) {
			print_weight(", infplgcp", cur, 48, false);
			print_weight(", infplgif", cur, 52, false);
		} else {
			if (lpar_valid) {
				printf(", infplgnm=");
				print_quoted_hex((char *) (cur + 40), 8);
			}
			print_x32(", infplgcp", cur, 48, false);
			print_x32(", infplgif", cur, 52, false);
		}
	}

	if (hdr_size >= 64) {
		print_ebcdic(", infpplnm", cur, 56, 8, false, false);

		if (hdr_size > 64 && !is_empty(cur + 64, hdr_size - 64)) {
			printf(", ");
			print_quoted_hex((char *) (cur + 64), hdr_size - 64);
		}
	} else if (hdr_size > 56 && !is_empty(cur + 56, hdr_size - 56)) {
		printf(", ");
		print_quoted_hex((char *) (cur + 56), hdr_size - 56);
	}
# else /* !VERBOSE */
	printf(", ...");
# endif /* !VERBOSE */

	printf("}");

hv_hdr:
	/* Hypervisor/guest headers */
	print_hypervisor_header(buf, 1, 20, 22, mt);
	print_guest_header(buf, 1, 24, 26);
	print_hypervisor_header(buf, 2, 28, 30, mt);
	print_guest_header(buf, 2, 32, 34);
	print_hypervisor_header(buf, 3, 36, 38, mt);
	print_guest_header(buf, 3, 40, 42);

	printf("}");
}

int
main(void)
{
	static const kernel_ulong_t bogus_func =
		(kernel_ulong_t) 0xdeafbeefdeadc0deULL;
	static const kernel_ulong_t bogus_resp_buf =
		(kernel_ulong_t) 0xfacefeedac0ffeedULL;
	static const kernel_ulong_t bogus_ret_code =
		(kernel_ulong_t) 0xf00dfa57decaffedULL;
	static const kernel_ulong_t bogus_flags =
		(kernel_ulong_t) 0xfee1deadfa57beefULL;

	unsigned char *buf = tail_alloc(PAGE_SIZE);
	TAIL_ALLOC_OBJECT_CONST_PTR(uint64_t, ret);

	long rc;

	rc = syscall(__NR_s390_sthyi, 0, 0, 0, 0);
	printf("s390_sthyi(STHYI_FC_CP_IFL_CAP, NULL, NULL, 0) = %s\n",
	       sprintrc(rc));

	rc = syscall(__NR_s390_sthyi, bogus_func, bogus_resp_buf,
		     bogus_ret_code, bogus_flags);
	printf("s390_sthyi(%#llx /* STHYI_FC_??? */, %#llx, %#llx, %#llx) = "
	       "%s\n",
	       (unsigned long long) bogus_func,
	       (unsigned long long) bogus_resp_buf,
	       (unsigned long long) bogus_ret_code,
	       (unsigned long long) bogus_flags,
	       sprintrc(rc));

	rc = syscall(__NR_s390_sthyi, bogus_func, buf, ret, 0);
	printf("s390_sthyi(%#llx /* STHYI_FC_??? */, %p, %p, 0) = %s\n",
	       (unsigned long long) bogus_func, buf, ret, sprintrc(rc));

	rc = syscall(__NR_s390_sthyi, 0, buf, ret, 0);
	if (rc)
		error_msg_and_skip("syscall(__NR_s390_sthyi, 0, buf, ret, 0) "
				   "returned unexpected value of %ld", rc);

	printf("s390_sthyi(STHYI_FC_CP_IFL_CAP, ");
	print_sthyi(buf);
	printf(", [0], 0) = 0\n");

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_ICONV_H && HAVE_ICONV_OPEN && __NR_s390_sthyi")

#endif
