/*
 * Provides an AUDIT_ARCH_* constant for the current process in CUR_AUDIT_ARCH
 * macro for some arcitectures (where such a constant defined).
 *
 * Copyright (c) 2021 Eugene Syromyatnikov <evgsyr@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef STRACE_TESTS_CUR_AUDIT_ARCH_H
# define STRACE_TESTS_CUR_AUDIT_ARCH_H

# include <linux/audit.h>

/*
 * If only one could easily get AUDIT_ARCH_* of the running process...
 * CUR_AUDIT_ARCH - AUDIT_ARCH_* of the current personality
 * PERS0_AUDIT_ARCH - AUDIT_ARCH_* of the strace's default personality
 *                    if the current one is mpers
 * PERS0__NR_gettid - gettid() syscall number in strace's default personality
 * M32_AUDIT_ARCH, MX32_AUDIT_ARCH - AUDIT_ARCH_* values for mpers
 * M32__NR_gettid, MX32_NR_gettid - gettid() syscall numbers in mpers
 */
# if defined __alpha__
#  define CUR_AUDIT_ARCH AUDIT_ARCH_ALPHA

# elif defined __arc__
#  if WORDS_BIGENDIAN
#   define CUR_AUDIT_ARCH AUDIT_ARCH_ARCOMPACTBE
#  else
#   define CUR_AUDIT_ARCH AUDIT_ARCH_ARCOMPACT
#  endif

# elif defined __arm64__ || defined __aarch64__
#  define CUR_AUDIT_ARCH AUDIT_ARCH_AARCH64
#  define M32_AUDIT_ARCH AUDIT_ARCH_ARM
#  define M32__NR_gettid 224
# elif defined __arm__
#  ifdef WORDS_BIGENDIAN
#   define CUR_AUDIT_ARCH AUDIT_ARCH_ARMEB
#  else
#   define PERS0_AUDIT_ARCH AUDIT_ARCH_AARCH64
#   define PERS0__NR_gettid 178
#   define CUR_AUDIT_ARCH AUDIT_ARCH_ARM
#  endif

# elif defined __x86_64__
#  define PERS0_AUDIT_ARCH AUDIT_ARCH_X86_64
#  define PERS0__NR_gettid 186
#  define CUR_AUDIT_ARCH AUDIT_ARCH_X86_64
#  define M32_AUDIT_ARCH AUDIT_ARCH_I386
#  define M32__NR_gettid 224
#  define MX32_AUDIT_ARCH AUDIT_ARCH_X86_64
#  define MX32__NR_gettid 1073742010
# elif defined __i386__
#  define PERS0_AUDIT_ARCH AUDIT_ARCH_X86_64
#  ifdef X32
#   define PERS0__NR_gettid 1073742010
#  else
#   define PERS0__NR_gettid 186
#  endif
#  define CUR_AUDIT_ARCH AUDIT_ARCH_I386

# elif defined __ia64__
#  define CUR_AUDIT_ARCH AUDIT_ARCH_IA64

# elif defined __hppa__
#  define CUR_AUDIT_ARCH AUDIT_ARCH_PARISC

# elif defined __m68k__
#  define CUR_AUDIT_ARCH AUDIT_ARCH_M68K

# elif defined __mips__
#  if _MIPS_SIM == _MIPS_SIM_ABI64
#   ifdef WORDS_BIGENDIAN
#    define CUR_AUDIT_ARCH AUDIT_ARCH_MIPS64
#   else
#    define CUR_AUDIT_ARCH AUDIT_ARCH_MIPSEL64
#   endif
#  elif _MIPS_SIM == _MIPS_SIM_ABIN32
#   ifdef WORDS_BIGENDIAN
#    define CUR_AUDIT_ARCH AUDIT_ARCH_MIPS64N32
#   else
#    define CUR_AUDIT_ARCH AUDIT_ARCH_MIPSEL64N32
#   endif
#  elif _MIPS_SIM == _MIPS_SIM_ABI32
#   ifdef WORDS_BIGENDIAN
#    define CUR_AUDIT_ARCH AUDIT_ARCH_MIPS
#   else
#    define CUR_AUDIT_ARCH AUDIT_ARCH_MIPSEL
#   endif
#  endif

# elif defined __powerpc64__
#  ifdef WORDS_BIGENDIAN
#   define CUR_AUDIT_ARCH AUDIT_ARCH_PPC64
#   define M32_AUDIT_ARCH AUDIT_ARCH_PPC
#   define M32__NR_gettid 207
#  else
#   define CUR_AUDIT_ARCH AUDIT_ARCH_PPC64LE
#  endif
# elif defined __powerpc__
#  define PERS0_AUDIT_ARCH AUDIT_ARCH_PPC64
#  define PERS0__NR_gettid 207
#  define CUR_AUDIT_ARCH AUDIT_ARCH_PPC

# elif defined __riscv__
#  define CUR_AUDIT_ARCH AUDIT_ARCH_RISCV64

# elif defined __s390x__
#  define CUR_AUDIT_ARCH AUDIT_ARCH_S390X
#  define M32_AUDIT_ARCH AUDIT_ARCH_S390
#  define M32__NR_gettid 236
# elif defined __s390__
#  define PERS0_AUDIT_ARCH AUDIT_ARCH_S390X
#  define PERS0__NR_gettid 236
#  define CUR_AUDIT_ARCH AUDIT_ARCH_S390

# elif defined __sh64__
#  ifdef WORDS_BIGENDIAN
#   define CUR_AUDIT_ARCH AUDIT_ARCH_SH64
#  else
#   define CUR_AUDIT_ARCH AUDIT_ARCH_SH64EL
#  endif
# elif defined __sh__
#  ifdef WORDS_BIGENDIAN
#   define CUR_AUDIT_ARCH AUDIT_ARCH_SH
#  else
#   define CUR_AUDIT_ARCH AUDIT_ARCH_SHEL
#  endif

# elif defined __sparc__ && defined __arch64__
#  define CUR_AUDIT_ARCH AUDIT_ARCH_SPARC64
#  define M32_AUDIT_ARCH AUDIT_ARCH_SPARC
#  define M32__NR_gettid 143
# elif defined __sparc__
#  define PERS0_AUDIT_ARCH AUDIT_ARCH_SPARC64
#  define PERS0__NR_gettid 143
#  define CUR_AUDIT_ARCH AUDIT_ARCH_SPARC

# elif defined __xtensa__
#  define CUR_AUDIT_ARCH AUDIT_ARCH_XTENSA

# endif

/* Undefine meaningless definitions */
# if defined(PERS0_AUDIT_ARCH) \
     && !defined(MPERS_IS_m32) && !defined(MPERS_IS_mx32)
#  undef PERS0_AUDIT_ARCH
# endif

# if defined(M32_AUDIT_ARCH) \
     && (!defined(HAVE_M32_MPERS) || defined(MPERS_IS_m32))
#  undef M32_AUDIT_ARCH
# endif

# if defined(MX32_AUDIT_ARCH) \
     && (!defined(HAVE_MX32_MPERS) || defined(MPERS_IS_mx32))
#  undef MX32_AUDIT_ARCH
# endif

#endif /* STRACE_TESTS_CUR_AUDIT_ARCH_H */
