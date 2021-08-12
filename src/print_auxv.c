/*
 * Decode auxv.
 *
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"

#include <elf.h>

#include "print_fields.h"

#include "xlat.h"
#include "xlat/auxvec_types.h"
#include "xlat/hwcaps.h"
#include "xlat/hwcaps2.h"

static inline void
print_hwcap(uint64_t a_val)
{
	static const char *dflt =
#if defined ARM
		"HWCAP_???"
#elif defined MIPS
		"HWCAP_???"
#elif defined NDS32
		"HWCAP_???"
#elif defined POWERPC || defined POWERPC64 || defined POWERPC64LE
		"PPC_FEATURE_???"
#elif defined RISCV64
		"COMPAT_HWCAP_???"
#elif defined S390 || defined S390X
		"HWCAP_S390_???"
#elif defined SH || defined SH64
		NULL
#elif defined SPARC || defined SPARC64
		"HWCAP_SPARC_???"
#elif defined I386 || defined X86_64 || defined X32
		NULL
#else
		NULL
#endif
		;
	static const bool uapi =
#if defined ARM
		true
#elif defined MIPS
		true
#elif defined NDS32
		false
#elif defined POWERPC || defined POWERPC64 || defined POWERPC64LE
		true
#elif defined RISCV64
		true
#elif defined S390 || defined S390X
		false
#elif defined SH || defined SH64
		false
#elif defined SPARC || defined SPARC64
		false
#elif defined I386 || defined X86_64 || defined X32
		false
#else
		false
#endif
		;

	if (dflt) {
		printflags_ex(a_val, dflt,
			      uapi || (xlat_verbose(xlat_verbosity) ==
				       XLAT_STYLE_RAW) ? XLAT_STYLE_DEFAULT
						       : XLAT_STYLE_VERBOSE,
			      hwcaps, NULL);
	} else {
		PRINT_VAL_X(a_val);
	}
}

static inline void
print_hwcap2(uint64_t a_val)
{
	static const char *dflt =
#if defined ARM
		"HWCAP2_???"
#elif defined POWERPC || defined POWERPC64 || defined POWERPC64LE
		"PPC_FEATURE"
#elif defined I386 || defined X86_64 || defined X32
		"HWCAP2_???"
#else
		NULL
#endif
		;

	if (dflt)
		printflags64(hwcaps2, a_val, dflt);
	else
		PRINT_VAL_X(a_val);
}

static inline void
print_cshape(uint64_t a_val)
{
	tprintf_comment("CSHAPE(totalsize=%" PRIu64
			", linesize=ilog2(%u)"
			", assoc=%" PRIu64")",
			a_val & ~0xff,
			1 << ((a_val & 0xf0) >> 4),
			a_val & 0xf);
}

static inline void
print_cache_geometry(uint64_t a_val)
{
	tprintf_comment("line size %" PRIu64 " B, associativity %" PRIu64,
			a_val & 0xffff, a_val >> 16);
}

static inline void
print_auxv_val(struct tcb *tcp, uint32_t a_type, uint64_t a_val)
{
	switch (a_type) {
	case AT_NULL:
	case AT_IGNORE:
		PRINT_VAL_X(a_val);
		break;
	case AT_EXECFD:
		printfd(tcp, a_val);
		break;
	case AT_PHDR:
		printaddr(a_val);
		break;
	case AT_PHENT:
	case AT_PHNUM:
	case AT_PAGESZ:
		PRINT_VAL_U(a_val);
		break;
	case AT_BASE:
		printaddr(a_val);
		break;
	case AT_FLAGS:
		PRINT_VAL_X(a_val);
		break;
	case AT_ENTRY:
		printaddr(a_val);
		break;
	case AT_NOTELF:
		PRINT_VAL_X(a_val);
		break;
	case AT_UID:
	case AT_EUID:
	case AT_GID:
	case AT_EGID:
		printuid(a_val);
		break;
	case AT_PLATFORM:
		printstr(tcp, a_val);
		break;
	case AT_HWCAP:
		print_hwcap(a_val);
		break;
	case AT_CLKTCK:
		PRINT_VAL_U(a_val);
		break;
	case AT_FPUCW:
		PRINT_VAL_X(a_val);
		break;
	case AT_DCACHEBSIZE:
	case AT_ICACHEBSIZE:
	case AT_UCACHEBSIZE:
	case AT_IGNOREPPC:
		PRINT_VAL_U(a_val);
		break;
	case AT_SECURE:
		PRINT_VAL_U(a_val);
		break;
	case AT_BASE_PLATFORM:
		printstr(tcp, a_val);
		break;
	case AT_RANDOM: /* 16 random bytes */
		printstr_ex(tcp, a_val, 16, QUOTE_FORCE_HEX);
		break;
	case AT_HWCAP2:
		print_hwcap2(a_val);
		break;
	case AT_EXECFN:
		printstr(tcp, a_val);
		break;
	case AT_SYSINFO:
	case AT_SYSINFO_EHDR:
		printaddr(a_val);
		break;
	case AT_L1I_CACHESHAPE:
	case AT_L1D_CACHESHAPE:
	case AT_L2_CACHESHAPE:
	case AT_L3_CACHESHAPE:
		PRINT_VAL_X(a_val);
		print_cshape(a_val);
		break;
	case AT_L1I_CACHESIZE:
	case AT_L1D_CACHESIZE:
	case AT_L2_CACHESIZE:
	case AT_L3_CACHESIZE:
		PRINT_VAL_U(a_val);
		break;
	case AT_L1I_CACHEGEOMETRY:
	case AT_L1D_CACHEGEOMETRY:
	case AT_L2_CACHEGEOMETRY:
	case AT_L3_CACHEGEOMETRY:
		PRINT_VAL_X(a_val);
		print_cache_geometry(a_val);
		break;
	case AT_ADI_BLKSZ:
	case AT_ADI_NBITS:
	case AT_ADI_UEONADI:
		PRINT_VAL_U(a_val);
		break;
	case AT_MINSIGSTKSZ:
		PRINT_VAL_X(a_val);
		break;
	default:
		PRINT_VAL_X(a_val);
	}
}

static inline bool
print_auxv32_elem(struct tcb *tcp, void *elem_buf, size_t elem_size,
		  void *opaque_data)
{
	Elf32_auxv_t *memb = (Elf32_auxv_t *) elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(*memb, a_type, auxvec_types, "AT_???");
	tprint_struct_next();
	tprints_field_name("a_un.a_val");
	print_auxv_val(tcp, memb->a_type, memb->a_un.a_val);
	tprint_struct_end();

	return true;
}

void
print_auxv32(struct tcb *tcp, kernel_ulong_t addr, kernel_ulong_t size)
{
	Elf32_auxv_t memb;

	print_array_ex(tcp, addr, size / sizeof(memb), &memb, sizeof(memb),
		       tfetch_mem, print_auxv32_elem, NULL,
		       size % sizeof(memb) ? PAF_ARRAY_TRUNCATED
					   : XLAT_STYLE_DEFAULT,
		       NULL, NULL);
}

static inline bool
print_auxv64_elem(struct tcb *tcp, void *elem_buf, size_t elem_size,
		  void *opaque_data)
{
	Elf64_auxv_t *memb = (Elf64_auxv_t *) elem_buf;

	tprint_struct_begin();
	PRINT_FIELD_XVAL(*memb, a_type, auxvec_types, "AT_???");
	tprint_struct_next();
	tprints_field_name("a_un.a_val");
	print_auxv_val(tcp, memb->a_type, memb->a_un.a_val);
	tprint_struct_end();

	return true;
}

void
print_auxv64(struct tcb *tcp, kernel_ulong_t addr, kernel_ulong_t size)
{
	Elf64_auxv_t memb;

	print_array_ex(tcp, addr, size / sizeof(memb), &memb, sizeof(memb),
		       tfetch_mem, print_auxv64_elem, NULL,
		       size % sizeof(memb) ? PAF_ARRAY_TRUNCATED
					   : XLAT_STYLE_DEFAULT,
		       NULL, NULL);
}

void
print_auxv(struct tcb *tcp, kernel_ulong_t addr, kernel_ulong_t size)
{
	switch (current_wordsize) {
	case 4: print_auxv32(tcp, addr, size); break;
	case 8: print_auxv64(tcp, addr, size); break;
	}
}
