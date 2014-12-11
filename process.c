/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 2000 PocketPenguins Inc.  Linux for Hitachi SuperH
 *                    port by Greg Banks <gbanks@pocketpenguins.com>
 *
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
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/user.h>
#ifdef HAVE_ELF_H
# include <elf.h>
#endif

#ifdef HAVE_SYS_REG_H
# include <sys/reg.h>
#endif

#ifdef HAVE_LINUX_PTRACE_H
# undef PTRACE_SYSCALL
# ifdef HAVE_STRUCT_IA64_FPREG
#  define ia64_fpreg XXX_ia64_fpreg
# endif
# ifdef HAVE_STRUCT_PT_ALL_USER_REGS
#  define pt_all_user_regs XXX_pt_all_user_regs
# endif
# ifdef HAVE_STRUCT_PTRACE_PEEKSIGINFO_ARGS
#  define ptrace_peeksiginfo_args XXX_ptrace_peeksiginfo_args
# endif
# include <linux/ptrace.h>
# undef ptrace_peeksiginfo_args
# undef ia64_fpreg
# undef pt_all_user_regs
#endif

#if defined(SPARC64)
# define r_pc r_tpc
# undef PTRACE_GETREGS
# define PTRACE_GETREGS PTRACE_GETREGS64
# undef PTRACE_SETREGS
# define PTRACE_SETREGS PTRACE_SETREGS64
#endif

#if defined(IA64)
# include <asm/ptrace_offsets.h>
# include <asm/rse.h>
#endif

#include "xlat/ptrace_cmds.h"
#include "xlat/ptrace_setoptions_flags.h"
#include "xlat/nt_descriptor_types.h"

#define uoff(member)	offsetof(struct user, member)
#define XLAT_UOFF(member)	{ uoff(member), "offsetof(struct user, " #member ")" }

static const struct xlat struct_user_offsets[] = {
#if defined(S390) || defined(S390X)
	{ PT_PSWMASK,		"psw_mask"				},
	{ PT_PSWADDR,		"psw_addr"				},
	{ PT_GPR0,		"gpr0"					},
	{ PT_GPR1,		"gpr1"					},
	{ PT_GPR2,		"gpr2"					},
	{ PT_GPR3,		"gpr3"					},
	{ PT_GPR4,		"gpr4"					},
	{ PT_GPR5,		"gpr5"					},
	{ PT_GPR6,		"gpr6"					},
	{ PT_GPR7,		"gpr7"					},
	{ PT_GPR8,		"gpr8"					},
	{ PT_GPR9,		"gpr9"					},
	{ PT_GPR10,		"gpr10"					},
	{ PT_GPR11,		"gpr11"					},
	{ PT_GPR12,		"gpr12"					},
	{ PT_GPR13,		"gpr13"					},
	{ PT_GPR14,		"gpr14"					},
	{ PT_GPR15,		"gpr15"					},
	{ PT_ACR0,		"acr0"					},
	{ PT_ACR1,		"acr1"					},
	{ PT_ACR2,		"acr2"					},
	{ PT_ACR3,		"acr3"					},
	{ PT_ACR4,		"acr4"					},
	{ PT_ACR5,		"acr5"					},
	{ PT_ACR6,		"acr6"					},
	{ PT_ACR7,		"acr7"					},
	{ PT_ACR8,		"acr8"					},
	{ PT_ACR9,		"acr9"					},
	{ PT_ACR10,		"acr10"					},
	{ PT_ACR11,		"acr11"					},
	{ PT_ACR12,		"acr12"					},
	{ PT_ACR13,		"acr13"					},
	{ PT_ACR14,		"acr14"					},
	{ PT_ACR15,		"acr15"					},
	{ PT_ORIGGPR2,		"orig_gpr2"				},
	{ PT_FPC,		"fpc"					},
#if defined(S390)
	{ PT_FPR0_HI,		"fpr0.hi"				},
	{ PT_FPR0_LO,		"fpr0.lo"				},
	{ PT_FPR1_HI,		"fpr1.hi"				},
	{ PT_FPR1_LO,		"fpr1.lo"				},
	{ PT_FPR2_HI,		"fpr2.hi"				},
	{ PT_FPR2_LO,		"fpr2.lo"				},
	{ PT_FPR3_HI,		"fpr3.hi"				},
	{ PT_FPR3_LO,		"fpr3.lo"				},
	{ PT_FPR4_HI,		"fpr4.hi"				},
	{ PT_FPR4_LO,		"fpr4.lo"				},
	{ PT_FPR5_HI,		"fpr5.hi"				},
	{ PT_FPR5_LO,		"fpr5.lo"				},
	{ PT_FPR6_HI,		"fpr6.hi"				},
	{ PT_FPR6_LO,		"fpr6.lo"				},
	{ PT_FPR7_HI,		"fpr7.hi"				},
	{ PT_FPR7_LO,		"fpr7.lo"				},
	{ PT_FPR8_HI,		"fpr8.hi"				},
	{ PT_FPR8_LO,		"fpr8.lo"				},
	{ PT_FPR9_HI,		"fpr9.hi"				},
	{ PT_FPR9_LO,		"fpr9.lo"				},
	{ PT_FPR10_HI,		"fpr10.hi"				},
	{ PT_FPR10_LO,		"fpr10.lo"				},
	{ PT_FPR11_HI,		"fpr11.hi"				},
	{ PT_FPR11_LO,		"fpr11.lo"				},
	{ PT_FPR12_HI,		"fpr12.hi"				},
	{ PT_FPR12_LO,		"fpr12.lo"				},
	{ PT_FPR13_HI,		"fpr13.hi"				},
	{ PT_FPR13_LO,		"fpr13.lo"				},
	{ PT_FPR14_HI,		"fpr14.hi"				},
	{ PT_FPR14_LO,		"fpr14.lo"				},
	{ PT_FPR15_HI,		"fpr15.hi"				},
	{ PT_FPR15_LO,		"fpr15.lo"				},
#endif
#if defined(S390X)
	{ PT_FPR0,		"fpr0"					},
	{ PT_FPR1,		"fpr1"					},
	{ PT_FPR2,		"fpr2"					},
	{ PT_FPR3,		"fpr3"					},
	{ PT_FPR4,		"fpr4"					},
	{ PT_FPR5,		"fpr5"					},
	{ PT_FPR6,		"fpr6"					},
	{ PT_FPR7,		"fpr7"					},
	{ PT_FPR8,		"fpr8"					},
	{ PT_FPR9,		"fpr9"					},
	{ PT_FPR10,		"fpr10"					},
	{ PT_FPR11,		"fpr11"					},
	{ PT_FPR12,		"fpr12"					},
	{ PT_FPR13,		"fpr13"					},
	{ PT_FPR14,		"fpr14"					},
	{ PT_FPR15,		"fpr15"					},
#endif
	{ PT_CR_9,		"cr9"					},
	{ PT_CR_10,		"cr10"					},
	{ PT_CR_11,		"cr11"					},
	{ PT_IEEE_IP,		"ieee_exception_ip"			},
#elif defined(SPARC)
	/* XXX No support for these offsets yet. */
#elif defined(HPPA)
	/* XXX No support for these offsets yet. */
#elif defined(POWERPC)
# ifndef PT_ORIG_R3
#  define PT_ORIG_R3 34
# endif
# define REGSIZE (sizeof(unsigned long))
	{ REGSIZE*PT_R0,		"r0"				},
	{ REGSIZE*PT_R1,		"r1"				},
	{ REGSIZE*PT_R2,		"r2"				},
	{ REGSIZE*PT_R3,		"r3"				},
	{ REGSIZE*PT_R4,		"r4"				},
	{ REGSIZE*PT_R5,		"r5"				},
	{ REGSIZE*PT_R6,		"r6"				},
	{ REGSIZE*PT_R7,		"r7"				},
	{ REGSIZE*PT_R8,		"r8"				},
	{ REGSIZE*PT_R9,		"r9"				},
	{ REGSIZE*PT_R10,		"r10"				},
	{ REGSIZE*PT_R11,		"r11"				},
	{ REGSIZE*PT_R12,		"r12"				},
	{ REGSIZE*PT_R13,		"r13"				},
	{ REGSIZE*PT_R14,		"r14"				},
	{ REGSIZE*PT_R15,		"r15"				},
	{ REGSIZE*PT_R16,		"r16"				},
	{ REGSIZE*PT_R17,		"r17"				},
	{ REGSIZE*PT_R18,		"r18"				},
	{ REGSIZE*PT_R19,		"r19"				},
	{ REGSIZE*PT_R20,		"r20"				},
	{ REGSIZE*PT_R21,		"r21"				},
	{ REGSIZE*PT_R22,		"r22"				},
	{ REGSIZE*PT_R23,		"r23"				},
	{ REGSIZE*PT_R24,		"r24"				},
	{ REGSIZE*PT_R25,		"r25"				},
	{ REGSIZE*PT_R26,		"r26"				},
	{ REGSIZE*PT_R27,		"r27"				},
	{ REGSIZE*PT_R28,		"r28"				},
	{ REGSIZE*PT_R29,		"r29"				},
	{ REGSIZE*PT_R30,		"r30"				},
	{ REGSIZE*PT_R31,		"r31"				},
	{ REGSIZE*PT_NIP,		"NIP"				},
	{ REGSIZE*PT_MSR,		"MSR"				},
	{ REGSIZE*PT_ORIG_R3,		"ORIG_R3"			},
	{ REGSIZE*PT_CTR,		"CTR"				},
	{ REGSIZE*PT_LNK,		"LNK"				},
	{ REGSIZE*PT_XER,		"XER"				},
	{ REGSIZE*PT_CCR,		"CCR"				},
	{ REGSIZE*PT_FPR0,		"FPR0"				},
# undef REGSIZE
#elif defined(ALPHA)
	{ 0,			"r0"					},
	{ 1,			"r1"					},
	{ 2,			"r2"					},
	{ 3,			"r3"					},
	{ 4,			"r4"					},
	{ 5,			"r5"					},
	{ 6,			"r6"					},
	{ 7,			"r7"					},
	{ 8,			"r8"					},
	{ 9,			"r9"					},
	{ 10,			"r10"					},
	{ 11,			"r11"					},
	{ 12,			"r12"					},
	{ 13,			"r13"					},
	{ 14,			"r14"					},
	{ 15,			"r15"					},
	{ 16,			"r16"					},
	{ 17,			"r17"					},
	{ 18,			"r18"					},
	{ 19,			"r19"					},
	{ 20,			"r20"					},
	{ 21,			"r21"					},
	{ 22,			"r22"					},
	{ 23,			"r23"					},
	{ 24,			"r24"					},
	{ 25,			"r25"					},
	{ 26,			"r26"					},
	{ 27,			"r27"					},
	{ 28,			"r28"					},
	{ 29,			"gp"					},
	{ 30,			"fp"					},
	{ 31,			"zero"					},
	{ 32,			"fp0"					},
	{ 33,			"fp"					},
	{ 34,			"fp2"					},
	{ 35,			"fp3"					},
	{ 36,			"fp4"					},
	{ 37,			"fp5"					},
	{ 38,			"fp6"					},
	{ 39,			"fp7"					},
	{ 40,			"fp8"					},
	{ 41,			"fp9"					},
	{ 42,			"fp10"					},
	{ 43,			"fp11"					},
	{ 44,			"fp12"					},
	{ 45,			"fp13"					},
	{ 46,			"fp14"					},
	{ 47,			"fp15"					},
	{ 48,			"fp16"					},
	{ 49,			"fp17"					},
	{ 50,			"fp18"					},
	{ 51,			"fp19"					},
	{ 52,			"fp20"					},
	{ 53,			"fp21"					},
	{ 54,			"fp22"					},
	{ 55,			"fp23"					},
	{ 56,			"fp24"					},
	{ 57,			"fp25"					},
	{ 58,			"fp26"					},
	{ 59,			"fp27"					},
	{ 60,			"fp28"					},
	{ 61,			"fp29"					},
	{ 62,			"fp30"					},
	{ 63,			"fp31"					},
	{ 64,			"pc"					},
#elif defined(IA64)
	{ PT_F32, "f32" }, { PT_F33, "f33" }, { PT_F34, "f34" },
	{ PT_F35, "f35" }, { PT_F36, "f36" }, { PT_F37, "f37" },
	{ PT_F38, "f38" }, { PT_F39, "f39" }, { PT_F40, "f40" },
	{ PT_F41, "f41" }, { PT_F42, "f42" }, { PT_F43, "f43" },
	{ PT_F44, "f44" }, { PT_F45, "f45" }, { PT_F46, "f46" },
	{ PT_F47, "f47" }, { PT_F48, "f48" }, { PT_F49, "f49" },
	{ PT_F50, "f50" }, { PT_F51, "f51" }, { PT_F52, "f52" },
	{ PT_F53, "f53" }, { PT_F54, "f54" }, { PT_F55, "f55" },
	{ PT_F56, "f56" }, { PT_F57, "f57" }, { PT_F58, "f58" },
	{ PT_F59, "f59" }, { PT_F60, "f60" }, { PT_F61, "f61" },
	{ PT_F62, "f62" }, { PT_F63, "f63" }, { PT_F64, "f64" },
	{ PT_F65, "f65" }, { PT_F66, "f66" }, { PT_F67, "f67" },
	{ PT_F68, "f68" }, { PT_F69, "f69" }, { PT_F70, "f70" },
	{ PT_F71, "f71" }, { PT_F72, "f72" }, { PT_F73, "f73" },
	{ PT_F74, "f74" }, { PT_F75, "f75" }, { PT_F76, "f76" },
	{ PT_F77, "f77" }, { PT_F78, "f78" }, { PT_F79, "f79" },
	{ PT_F80, "f80" }, { PT_F81, "f81" }, { PT_F82, "f82" },
	{ PT_F83, "f83" }, { PT_F84, "f84" }, { PT_F85, "f85" },
	{ PT_F86, "f86" }, { PT_F87, "f87" }, { PT_F88, "f88" },
	{ PT_F89, "f89" }, { PT_F90, "f90" }, { PT_F91, "f91" },
	{ PT_F92, "f92" }, { PT_F93, "f93" }, { PT_F94, "f94" },
	{ PT_F95, "f95" }, { PT_F96, "f96" }, { PT_F97, "f97" },
	{ PT_F98, "f98" }, { PT_F99, "f99" }, { PT_F100, "f100" },
	{ PT_F101, "f101" }, { PT_F102, "f102" }, { PT_F103, "f103" },
	{ PT_F104, "f104" }, { PT_F105, "f105" }, { PT_F106, "f106" },
	{ PT_F107, "f107" }, { PT_F108, "f108" }, { PT_F109, "f109" },
	{ PT_F110, "f110" }, { PT_F111, "f111" }, { PT_F112, "f112" },
	{ PT_F113, "f113" }, { PT_F114, "f114" }, { PT_F115, "f115" },
	{ PT_F116, "f116" }, { PT_F117, "f117" }, { PT_F118, "f118" },
	{ PT_F119, "f119" }, { PT_F120, "f120" }, { PT_F121, "f121" },
	{ PT_F122, "f122" }, { PT_F123, "f123" }, { PT_F124, "f124" },
	{ PT_F125, "f125" }, { PT_F126, "f126" }, { PT_F127, "f127" },
	/* switch stack: */
	{ PT_F2, "f2" }, { PT_F3, "f3" }, { PT_F4, "f4" },
	{ PT_F5, "f5" }, { PT_F10, "f10" }, { PT_F11, "f11" },
	{ PT_F12, "f12" }, { PT_F13, "f13" }, { PT_F14, "f14" },
	{ PT_F15, "f15" }, { PT_F16, "f16" }, { PT_F17, "f17" },
	{ PT_F18, "f18" }, { PT_F19, "f19" }, { PT_F20, "f20" },
	{ PT_F21, "f21" }, { PT_F22, "f22" }, { PT_F23, "f23" },
	{ PT_F24, "f24" }, { PT_F25, "f25" }, { PT_F26, "f26" },
	{ PT_F27, "f27" }, { PT_F28, "f28" }, { PT_F29, "f29" },
	{ PT_F30, "f30" }, { PT_F31, "f31" }, { PT_R4, "r4" },
	{ PT_R5, "r5" }, { PT_R6, "r6" }, { PT_R7, "r7" },
	{ PT_B1, "b1" }, { PT_B2, "b2" }, { PT_B3, "b3" },
	{ PT_B4, "b4" }, { PT_B5, "b5" },
	{ PT_AR_EC, "ar.ec" }, { PT_AR_LC, "ar.lc" },
	/* pt_regs */
	{ PT_CR_IPSR, "psr" }, { PT_CR_IIP, "ip" },
	{ PT_CFM, "cfm" }, { PT_AR_UNAT, "ar.unat" },
	{ PT_AR_PFS, "ar.pfs" }, { PT_AR_RSC, "ar.rsc" },
	{ PT_AR_RNAT, "ar.rnat" }, { PT_AR_BSPSTORE, "ar.bspstore" },
	{ PT_PR, "pr" }, { PT_B6, "b6" }, { PT_AR_BSP, "ar.bsp" },
	{ PT_R1, "r1" }, { PT_R2, "r2" }, { PT_R3, "r3" },
	{ PT_R12, "r12" }, { PT_R13, "r13" }, { PT_R14, "r14" },
	{ PT_R15, "r15" }, { PT_R8, "r8" }, { PT_R9, "r9" },
	{ PT_R10, "r10" }, { PT_R11, "r11" }, { PT_R16, "r16" },
	{ PT_R17, "r17" }, { PT_R18, "r18" }, { PT_R19, "r19" },
	{ PT_R20, "r20" }, { PT_R21, "r21" }, { PT_R22, "r22" },
	{ PT_R23, "r23" }, { PT_R24, "r24" }, { PT_R25, "r25" },
	{ PT_R26, "r26" }, { PT_R27, "r27" }, { PT_R28, "r28" },
	{ PT_R29, "r29" }, { PT_R30, "r30" }, { PT_R31, "r31" },
	{ PT_AR_CCV, "ar.ccv" }, { PT_AR_FPSR, "ar.fpsr" },
	{ PT_B0, "b0" }, { PT_B7, "b7" }, { PT_F6, "f6" },
	{ PT_F7, "f7" }, { PT_F8, "f8" }, { PT_F9, "f9" },
# ifdef PT_AR_CSD
	{ PT_AR_CSD, "ar.csd" },
# endif
# ifdef PT_AR_SSD
	{ PT_AR_SSD, "ar.ssd" },
# endif
	{ PT_DBR, "dbr" }, { PT_IBR, "ibr" }, { PT_PMD, "pmd" },
#elif defined(I386)
	XLAT(4*EBX),
	XLAT(4*ECX),
	XLAT(4*EDX),
	XLAT(4*ESI),
	XLAT(4*EDI),
	XLAT(4*EBP),
	XLAT(4*EAX),
	XLAT(4*DS),
	XLAT(4*ES),
	XLAT(4*FS),
	XLAT(4*GS),
	XLAT(4*ORIG_EAX),
	XLAT(4*EIP),
	XLAT(4*CS),
	XLAT(4*EFL),
	XLAT(4*UESP),
	XLAT(4*SS),
#elif defined(X86_64) || defined(X32)
	XLAT(8*R15),
	XLAT(8*R14),
	XLAT(8*R13),
	XLAT(8*R12),
	XLAT(8*RBP),
	XLAT(8*RBX),
	XLAT(8*R11),
	XLAT(8*R10),
	XLAT(8*R9),
	XLAT(8*R8),
	XLAT(8*RAX),
	XLAT(8*RCX),
	XLAT(8*RDX),
	XLAT(8*RSI),
	XLAT(8*RDI),
	XLAT(8*ORIG_RAX),
	XLAT(8*RIP),
	XLAT(8*CS),
	{ 8*EFLAGS,		"8*EFL"					},
	XLAT(8*RSP),
	XLAT(8*SS),
#elif defined(M68K)
	XLAT(4*PT_D1),
	XLAT(4*PT_D2),
	XLAT(4*PT_D3),
	XLAT(4*PT_D4),
	XLAT(4*PT_D5),
	XLAT(4*PT_D6),
	XLAT(4*PT_D7),
	XLAT(4*PT_A0),
	XLAT(4*PT_A1),
	XLAT(4*PT_A2),
	XLAT(4*PT_A3),
	XLAT(4*PT_A4),
	XLAT(4*PT_A5),
	XLAT(4*PT_A6),
	XLAT(4*PT_D0),
	XLAT(4*PT_USP),
	XLAT(4*PT_ORIG_D0),
	XLAT(4*PT_SR),
	XLAT(4*PT_PC),
#elif defined(SH)
	XLAT(4*REG_REG0),
	{ 4*(REG_REG0+1),	"4*REG_REG1"				},
	{ 4*(REG_REG0+2),	"4*REG_REG2"				},
	{ 4*(REG_REG0+3),	"4*REG_REG3"				},
	{ 4*(REG_REG0+4),	"4*REG_REG4"				},
	{ 4*(REG_REG0+5),	"4*REG_REG5"				},
	{ 4*(REG_REG0+6),	"4*REG_REG6"				},
	{ 4*(REG_REG0+7),	"4*REG_REG7"				},
	{ 4*(REG_REG0+8),	"4*REG_REG8"				},
	{ 4*(REG_REG0+9),	"4*REG_REG9"				},
	{ 4*(REG_REG0+10),	"4*REG_REG10"				},
	{ 4*(REG_REG0+11),	"4*REG_REG11"				},
	{ 4*(REG_REG0+12),	"4*REG_REG12"				},
	{ 4*(REG_REG0+13),	"4*REG_REG13"				},
	{ 4*(REG_REG0+14),	"4*REG_REG14"				},
	XLAT(4*REG_REG15),
	XLAT(4*REG_PC),
	XLAT(4*REG_PR),
	XLAT(4*REG_SR),
	XLAT(4*REG_GBR),
	XLAT(4*REG_MACH),
	XLAT(4*REG_MACL),
	XLAT(4*REG_SYSCALL),
	XLAT(4*REG_FPUL),
	XLAT(4*REG_FPREG0),
	{ 4*(REG_FPREG0+1),	"4*REG_FPREG1"				},
	{ 4*(REG_FPREG0+2),	"4*REG_FPREG2"				},
	{ 4*(REG_FPREG0+3),	"4*REG_FPREG3"				},
	{ 4*(REG_FPREG0+4),	"4*REG_FPREG4"				},
	{ 4*(REG_FPREG0+5),	"4*REG_FPREG5"				},
	{ 4*(REG_FPREG0+6),	"4*REG_FPREG6"				},
	{ 4*(REG_FPREG0+7),	"4*REG_FPREG7"				},
	{ 4*(REG_FPREG0+8),	"4*REG_FPREG8"				},
	{ 4*(REG_FPREG0+9),	"4*REG_FPREG9"				},
	{ 4*(REG_FPREG0+10),	"4*REG_FPREG10"				},
	{ 4*(REG_FPREG0+11),	"4*REG_FPREG11"				},
	{ 4*(REG_FPREG0+12),	"4*REG_FPREG12"				},
	{ 4*(REG_FPREG0+13),	"4*REG_FPREG13"				},
	{ 4*(REG_FPREG0+14),	"4*REG_FPREG14"				},
	XLAT(4*REG_FPREG15),
# ifdef REG_XDREG0
	XLAT(4*REG_XDREG0),
	{ 4*(REG_XDREG0+2),	"4*REG_XDREG2"				},
	{ 4*(REG_XDREG0+4),	"4*REG_XDREG4"				},
	{ 4*(REG_XDREG0+6),	"4*REG_XDREG6"				},
	{ 4*(REG_XDREG0+8),	"4*REG_XDREG8"				},
	{ 4*(REG_XDREG0+10),	"4*REG_XDREG10"				},
	{ 4*(REG_XDREG0+12),	"4*REG_XDREG12"				},
	XLAT(4*REG_XDREG14),
# endif
	XLAT(4*REG_FPSCR),
#elif defined(SH64)
	{ 0,			"PC(L)"					},
	{ 4,			"PC(U)"					},
	{ 8,			"SR(L)"					},
	{ 12,			"SR(U)"					},
	{ 16,			"syscall no.(L)"			},
	{ 20,			"syscall_no.(U)"			},
	{ 24,			"R0(L)"					},
	{ 28,			"R0(U)"					},
	{ 32,			"R1(L)"					},
	{ 36,			"R1(U)"					},
	{ 40,			"R2(L)"					},
	{ 44,			"R2(U)"					},
	{ 48,			"R3(L)"					},
	{ 52,			"R3(U)"					},
	{ 56,			"R4(L)"					},
	{ 60,			"R4(U)"					},
	{ 64,			"R5(L)"					},
	{ 68,			"R5(U)"					},
	{ 72,			"R6(L)"					},
	{ 76,			"R6(U)"					},
	{ 80,			"R7(L)"					},
	{ 84,			"R7(U)"					},
	{ 88,			"R8(L)"					},
	{ 92,			"R8(U)"					},
	{ 96,			"R9(L)"					},
	{ 100,			"R9(U)"					},
	{ 104,			"R10(L)"				},
	{ 108,			"R10(U)"				},
	{ 112,			"R11(L)"				},
	{ 116,			"R11(U)"				},
	{ 120,			"R12(L)"				},
	{ 124,			"R12(U)"				},
	{ 128,			"R13(L)"				},
	{ 132,			"R13(U)"				},
	{ 136,			"R14(L)"				},
	{ 140,			"R14(U)"				},
	{ 144,			"R15(L)"				},
	{ 148,			"R15(U)"				},
	{ 152,			"R16(L)"				},
	{ 156,			"R16(U)"				},
	{ 160,			"R17(L)"				},
	{ 164,			"R17(U)"				},
	{ 168,			"R18(L)"				},
	{ 172,			"R18(U)"				},
	{ 176,			"R19(L)"				},
	{ 180,			"R19(U)"				},
	{ 184,			"R20(L)"				},
	{ 188,			"R20(U)"				},
	{ 192,			"R21(L)"				},
	{ 196,			"R21(U)"				},
	{ 200,			"R22(L)"				},
	{ 204,			"R22(U)"				},
	{ 208,			"R23(L)"				},
	{ 212,			"R23(U)"				},
	{ 216,			"R24(L)"				},
	{ 220,			"R24(U)"				},
	{ 224,			"R25(L)"				},
	{ 228,			"R25(U)"				},
	{ 232,			"R26(L)"				},
	{ 236,			"R26(U)"				},
	{ 240,			"R27(L)"				},
	{ 244,			"R27(U)"				},
	{ 248,			"R28(L)"				},
	{ 252,			"R28(U)"				},
	{ 256,			"R29(L)"				},
	{ 260,			"R29(U)"				},
	{ 264,			"R30(L)"				},
	{ 268,			"R30(U)"				},
	{ 272,			"R31(L)"				},
	{ 276,			"R31(U)"				},
	{ 280,			"R32(L)"				},
	{ 284,			"R32(U)"				},
	{ 288,			"R33(L)"				},
	{ 292,			"R33(U)"				},
	{ 296,			"R34(L)"				},
	{ 300,			"R34(U)"				},
	{ 304,			"R35(L)"				},
	{ 308,			"R35(U)"				},
	{ 312,			"R36(L)"				},
	{ 316,			"R36(U)"				},
	{ 320,			"R37(L)"				},
	{ 324,			"R37(U)"				},
	{ 328,			"R38(L)"				},
	{ 332,			"R38(U)"				},
	{ 336,			"R39(L)"				},
	{ 340,			"R39(U)"				},
	{ 344,			"R40(L)"				},
	{ 348,			"R40(U)"				},
	{ 352,			"R41(L)"				},
	{ 356,			"R41(U)"				},
	{ 360,			"R42(L)"				},
	{ 364,			"R42(U)"				},
	{ 368,			"R43(L)"				},
	{ 372,			"R43(U)"				},
	{ 376,			"R44(L)"				},
	{ 380,			"R44(U)"				},
	{ 384,			"R45(L)"				},
	{ 388,			"R45(U)"				},
	{ 392,			"R46(L)"				},
	{ 396,			"R46(U)"				},
	{ 400,			"R47(L)"				},
	{ 404,			"R47(U)"				},
	{ 408,			"R48(L)"				},
	{ 412,			"R48(U)"				},
	{ 416,			"R49(L)"				},
	{ 420,			"R49(U)"				},
	{ 424,			"R50(L)"				},
	{ 428,			"R50(U)"				},
	{ 432,			"R51(L)"				},
	{ 436,			"R51(U)"				},
	{ 440,			"R52(L)"				},
	{ 444,			"R52(U)"				},
	{ 448,			"R53(L)"				},
	{ 452,			"R53(U)"				},
	{ 456,			"R54(L)"				},
	{ 460,			"R54(U)"				},
	{ 464,			"R55(L)"				},
	{ 468,			"R55(U)"				},
	{ 472,			"R56(L)"				},
	{ 476,			"R56(U)"				},
	{ 480,			"R57(L)"				},
	{ 484,			"R57(U)"				},
	{ 488,			"R58(L)"				},
	{ 492,			"R58(U)"				},
	{ 496,			"R59(L)"				},
	{ 500,			"R59(U)"				},
	{ 504,			"R60(L)"				},
	{ 508,			"R60(U)"				},
	{ 512,			"R61(L)"				},
	{ 516,			"R61(U)"				},
	{ 520,			"R62(L)"				},
	{ 524,			"R62(U)"				},
	{ 528,			"TR0(L)"				},
	{ 532,			"TR0(U)"				},
	{ 536,			"TR1(L)"				},
	{ 540,			"TR1(U)"				},
	{ 544,			"TR2(L)"				},
	{ 548,			"TR2(U)"				},
	{ 552,			"TR3(L)"				},
	{ 556,			"TR3(U)"				},
	{ 560,			"TR4(L)"				},
	{ 564,			"TR4(U)"				},
	{ 568,			"TR5(L)"				},
	{ 572,			"TR5(U)"				},
	{ 576,			"TR6(L)"				},
	{ 580,			"TR6(U)"				},
	{ 584,			"TR7(L)"				},
	{ 588,			"TR7(U)"				},
	/* This entry is in case pt_regs contains dregs (depends on
	   the kernel build options). */
	XLAT_UOFF(regs),
	XLAT_UOFF(fpu),
#elif defined(ARM)
	{ uoff(regs.ARM_r0),	"r0"					},
	{ uoff(regs.ARM_r1),	"r1"					},
	{ uoff(regs.ARM_r2),	"r2"					},
	{ uoff(regs.ARM_r3),	"r3"					},
	{ uoff(regs.ARM_r4),	"r4"					},
	{ uoff(regs.ARM_r5),	"r5"					},
	{ uoff(regs.ARM_r6),	"r6"					},
	{ uoff(regs.ARM_r7),	"r7"					},
	{ uoff(regs.ARM_r8),	"r8"					},
	{ uoff(regs.ARM_r9),	"r9"					},
	{ uoff(regs.ARM_r10),	"r10"					},
	{ uoff(regs.ARM_fp),	"fp"					},
	{ uoff(regs.ARM_ip),	"ip"					},
	{ uoff(regs.ARM_sp),	"sp"					},
	{ uoff(regs.ARM_lr),	"lr"					},
	{ uoff(regs.ARM_pc),	"pc"					},
	{ uoff(regs.ARM_cpsr),	"cpsr"					},
#elif defined(AVR32)
	{ uoff(regs.sr),	"sr"					},
	{ uoff(regs.pc),	"pc"					},
	{ uoff(regs.lr),	"lr"					},
	{ uoff(regs.sp),	"sp"					},
	{ uoff(regs.r12),	"r12"					},
	{ uoff(regs.r11),	"r11"					},
	{ uoff(regs.r10),	"r10"					},
	{ uoff(regs.r9),	"r9"					},
	{ uoff(regs.r8),	"r8"					},
	{ uoff(regs.r7),	"r7"					},
	{ uoff(regs.r6),	"r6"					},
	{ uoff(regs.r5),	"r5"					},
	{ uoff(regs.r4),	"r4"					},
	{ uoff(regs.r3),	"r3"					},
	{ uoff(regs.r2),	"r2"					},
	{ uoff(regs.r1),	"r1"					},
	{ uoff(regs.r0),	"r0"					},
	{ uoff(regs.r12_orig),	"orig_r12"				},
#elif defined(MIPS)
	{ 0,			"r0"					},
	{ 1,			"r1"					},
	{ 2,			"r2"					},
	{ 3,			"r3"					},
	{ 4,			"r4"					},
	{ 5,			"r5"					},
	{ 6,			"r6"					},
	{ 7,			"r7"					},
	{ 8,			"r8"					},
	{ 9,			"r9"					},
	{ 10,			"r10"					},
	{ 11,			"r11"					},
	{ 12,			"r12"					},
	{ 13,			"r13"					},
	{ 14,			"r14"					},
	{ 15,			"r15"					},
	{ 16,			"r16"					},
	{ 17,			"r17"					},
	{ 18,			"r18"					},
	{ 19,			"r19"					},
	{ 20,			"r20"					},
	{ 21,			"r21"					},
	{ 22,			"r22"					},
	{ 23,			"r23"					},
	{ 24,			"r24"					},
	{ 25,			"r25"					},
	{ 26,			"r26"					},
	{ 27,			"r27"					},
	{ 28,			"r28"					},
	{ 29,			"r29"					},
	{ 30,			"r30"					},
	{ 31,			"r31"					},
	{ 32,			"f0"					},
	{ 33,			"f1"					},
	{ 34,			"f2"					},
	{ 35,			"f3"					},
	{ 36,			"f4"					},
	{ 37,			"f5"					},
	{ 38,			"f6"					},
	{ 39,			"f7"					},
	{ 40,			"f8"					},
	{ 41,			"f9"					},
	{ 42,			"f10"					},
	{ 43,			"f11"					},
	{ 44,			"f12"					},
	{ 45,			"f13"					},
	{ 46,			"f14"					},
	{ 47,			"f15"					},
	{ 48,			"f16"					},
	{ 49,			"f17"					},
	{ 50,			"f18"					},
	{ 51,			"f19"					},
	{ 52,			"f20"					},
	{ 53,			"f21"					},
	{ 54,			"f22"					},
	{ 55,			"f23"					},
	{ 56,			"f24"					},
	{ 57,			"f25"					},
	{ 58,			"f26"					},
	{ 59,			"f27"					},
	{ 60,			"f28"					},
	{ 61,			"f29"					},
	{ 62,			"f30"					},
	{ 63,			"f31"					},
	{ 64,			"pc"					},
	{ 65,			"cause"					},
	{ 66,			"badvaddr"				},
	{ 67,			"mmhi"					},
	{ 68,			"mmlo"					},
	{ 69,			"fpcsr"					},
	{ 70,			"fpeir"					},
#elif defined(TILE)
	{ PTREGS_OFFSET_REG(0),  "r0"  },
	{ PTREGS_OFFSET_REG(1),  "r1"  },
	{ PTREGS_OFFSET_REG(2),  "r2"  },
	{ PTREGS_OFFSET_REG(3),  "r3"  },
	{ PTREGS_OFFSET_REG(4),  "r4"  },
	{ PTREGS_OFFSET_REG(5),  "r5"  },
	{ PTREGS_OFFSET_REG(6),  "r6"  },
	{ PTREGS_OFFSET_REG(7),  "r7"  },
	{ PTREGS_OFFSET_REG(8),  "r8"  },
	{ PTREGS_OFFSET_REG(9),  "r9"  },
	{ PTREGS_OFFSET_REG(10), "r10" },
	{ PTREGS_OFFSET_REG(11), "r11" },
	{ PTREGS_OFFSET_REG(12), "r12" },
	{ PTREGS_OFFSET_REG(13), "r13" },
	{ PTREGS_OFFSET_REG(14), "r14" },
	{ PTREGS_OFFSET_REG(15), "r15" },
	{ PTREGS_OFFSET_REG(16), "r16" },
	{ PTREGS_OFFSET_REG(17), "r17" },
	{ PTREGS_OFFSET_REG(18), "r18" },
	{ PTREGS_OFFSET_REG(19), "r19" },
	{ PTREGS_OFFSET_REG(20), "r20" },
	{ PTREGS_OFFSET_REG(21), "r21" },
	{ PTREGS_OFFSET_REG(22), "r22" },
	{ PTREGS_OFFSET_REG(23), "r23" },
	{ PTREGS_OFFSET_REG(24), "r24" },
	{ PTREGS_OFFSET_REG(25), "r25" },
	{ PTREGS_OFFSET_REG(26), "r26" },
	{ PTREGS_OFFSET_REG(27), "r27" },
	{ PTREGS_OFFSET_REG(28), "r28" },
	{ PTREGS_OFFSET_REG(29), "r29" },
	{ PTREGS_OFFSET_REG(30), "r30" },
	{ PTREGS_OFFSET_REG(31), "r31" },
	{ PTREGS_OFFSET_REG(32), "r32" },
	{ PTREGS_OFFSET_REG(33), "r33" },
	{ PTREGS_OFFSET_REG(34), "r34" },
	{ PTREGS_OFFSET_REG(35), "r35" },
	{ PTREGS_OFFSET_REG(36), "r36" },
	{ PTREGS_OFFSET_REG(37), "r37" },
	{ PTREGS_OFFSET_REG(38), "r38" },
	{ PTREGS_OFFSET_REG(39), "r39" },
	{ PTREGS_OFFSET_REG(40), "r40" },
	{ PTREGS_OFFSET_REG(41), "r41" },
	{ PTREGS_OFFSET_REG(42), "r42" },
	{ PTREGS_OFFSET_REG(43), "r43" },
	{ PTREGS_OFFSET_REG(44), "r44" },
	{ PTREGS_OFFSET_REG(45), "r45" },
	{ PTREGS_OFFSET_REG(46), "r46" },
	{ PTREGS_OFFSET_REG(47), "r47" },
	{ PTREGS_OFFSET_REG(48), "r48" },
	{ PTREGS_OFFSET_REG(49), "r49" },
	{ PTREGS_OFFSET_REG(50), "r50" },
	{ PTREGS_OFFSET_REG(51), "r51" },
	{ PTREGS_OFFSET_REG(52), "r52" },
	{ PTREGS_OFFSET_TP, "tp" },
	{ PTREGS_OFFSET_SP, "sp" },
	{ PTREGS_OFFSET_LR, "lr" },
	{ PTREGS_OFFSET_PC, "pc" },
	{ PTREGS_OFFSET_EX1, "ex1" },
	{ PTREGS_OFFSET_FAULTNUM, "faultnum" },
	{ PTREGS_OFFSET_ORIG_R0, "orig_r0" },
	{ PTREGS_OFFSET_FLAGS, "flags" },
#endif
#ifdef CRISV10
	XLAT(4*PT_FRAMETYPE),
	XLAT(4*PT_ORIG_R10),
	XLAT(4*PT_R13),
	XLAT(4*PT_R12),
	XLAT(4*PT_R11),
	XLAT(4*PT_R10),
	XLAT(4*PT_R9),
	XLAT(4*PT_R8),
	XLAT(4*PT_R7),
	XLAT(4*PT_R6),
	XLAT(4*PT_R5),
	XLAT(4*PT_R4),
	XLAT(4*PT_R3),
	XLAT(4*PT_R2),
	XLAT(4*PT_R1),
	XLAT(4*PT_R0),
	XLAT(4*PT_MOF),
	XLAT(4*PT_DCCR),
	XLAT(4*PT_SRP),
	XLAT(4*PT_IRP),
	XLAT(4*PT_CSRINSTR),
	XLAT(4*PT_CSRADDR),
	XLAT(4*PT_CSRDATA),
	XLAT(4*PT_USP),
#endif
#ifdef CRISV32
	XLAT(4*PT_ORIG_R10),
	XLAT(4*PT_R0),
	XLAT(4*PT_R1),
	XLAT(4*PT_R2),
	XLAT(4*PT_R3),
	XLAT(4*PT_R4),
	XLAT(4*PT_R5),
	XLAT(4*PT_R6),
	XLAT(4*PT_R7),
	XLAT(4*PT_R8),
	XLAT(4*PT_R9),
	XLAT(4*PT_R10),
	XLAT(4*PT_R11),
	XLAT(4*PT_R12),
	XLAT(4*PT_R13),
	XLAT(4*PT_ACR),
	XLAT(4*PT_SRS),
	XLAT(4*PT_MOF),
	XLAT(4*PT_SPC),
	XLAT(4*PT_CCS),
	XLAT(4*PT_SRP),
	XLAT(4*PT_ERP),
	XLAT(4*PT_EXS),
	XLAT(4*PT_EDA),
	XLAT(4*PT_USP),
	XLAT(4*PT_PPC),
	XLAT(4*PT_BP_CTRL),
	XLAT(4*PT_BP+4),
	XLAT(4*PT_BP+8),
	XLAT(4*PT_BP+12),
	XLAT(4*PT_BP+16),
	XLAT(4*PT_BP+20),
	XLAT(4*PT_BP+24),
	XLAT(4*PT_BP+28),
	XLAT(4*PT_BP+32),
	XLAT(4*PT_BP+36),
	XLAT(4*PT_BP+40),
	XLAT(4*PT_BP+44),
	XLAT(4*PT_BP+48),
	XLAT(4*PT_BP+52),
	XLAT(4*PT_BP+56),
#endif
#ifdef MICROBLAZE
	{ PT_GPR(0),		"r0"					},
	{ PT_GPR(1),		"r1"					},
	{ PT_GPR(2),		"r2"					},
	{ PT_GPR(3),		"r3"					},
	{ PT_GPR(4),		"r4"					},
	{ PT_GPR(5),		"r5"					},
	{ PT_GPR(6),		"r6"					},
	{ PT_GPR(7),		"r7"					},
	{ PT_GPR(8),		"r8"					},
	{ PT_GPR(9),		"r9"					},
	{ PT_GPR(10),		"r10"					},
	{ PT_GPR(11),		"r11"					},
	{ PT_GPR(12),		"r12"					},
	{ PT_GPR(13),		"r13"					},
	{ PT_GPR(14),		"r14"					},
	{ PT_GPR(15),		"r15"					},
	{ PT_GPR(16),		"r16"					},
	{ PT_GPR(17),		"r17"					},
	{ PT_GPR(18),		"r18"					},
	{ PT_GPR(19),		"r19"					},
	{ PT_GPR(20),		"r20"					},
	{ PT_GPR(21),		"r21"					},
	{ PT_GPR(22),		"r22"					},
	{ PT_GPR(23),		"r23"					},
	{ PT_GPR(24),		"r24"					},
	{ PT_GPR(25),		"r25"					},
	{ PT_GPR(26),		"r26"					},
	{ PT_GPR(27),		"r27"					},
	{ PT_GPR(28),		"r28"					},
	{ PT_GPR(29),		"r29"					},
	{ PT_GPR(30),		"r30"					},
	{ PT_GPR(31),		"r31"					},
	{ PT_PC,		"rpc",					},
	{ PT_MSR,		"rmsr",					},
	{ PT_EAR,		"rear",					},
	{ PT_ESR,		"resr",					},
	{ PT_FSR,		"rfsr",					},
	{ PT_KERNEL_MODE,	"kernel_mode",				},
#endif
#ifdef OR1K
	{ 4*0,  "r0" },
	{ 4*1,  "r1" },
	{ 4*2,  "r2" },
	{ 4*3,  "r3" },
	{ 4*4,  "r4" },
	{ 4*5,  "r5" },
	{ 4*6,  "r6" },
	{ 4*7,  "r7" },
	{ 4*8,  "r8" },
	{ 4*9,  "r9" },
	{ 4*10, "r10" },
	{ 4*11, "r11" },
	{ 4*12, "r12" },
	{ 4*13, "r13" },
	{ 4*14, "r14" },
	{ 4*15, "r15" },
	{ 4*16, "r16" },
	{ 4*17, "r17" },
	{ 4*18, "r18" },
	{ 4*19, "r19" },
	{ 4*20, "r20" },
	{ 4*21, "r21" },
	{ 4*22, "r22" },
	{ 4*23, "r23" },
	{ 4*24, "r24" },
	{ 4*25, "r25" },
	{ 4*26, "r26" },
	{ 4*27, "r27" },
	{ 4*28, "r28" },
	{ 4*29, "r29" },
	{ 4*30, "r30" },
	{ 4*31, "r31" },
	{ 4*32, "pc" },
	{ 4*33, "sr" },
#endif
#ifdef XTENSA
	{ REG_A_BASE,           "a0"            },
	{ REG_A_BASE+1,         "a1"            },
	{ REG_A_BASE+2,         "a2"            },
	{ REG_A_BASE+3,         "a3"            },
	{ REG_A_BASE+4,         "a4"            },
	{ REG_A_BASE+5,         "a5"            },
	{ REG_A_BASE+6,         "a6"            },
	{ REG_A_BASE+7,         "a7"            },
	{ REG_A_BASE+8,         "a8"            },
	{ REG_A_BASE+9,         "a9"            },
	{ REG_A_BASE+10,        "a10"           },
	{ REG_A_BASE+11,        "a11"           },
	{ REG_A_BASE+12,        "a12"           },
	{ REG_A_BASE+13,        "a13"           },
	{ REG_A_BASE+14,        "a14"           },
	{ REG_A_BASE+15,        "a15"           },
	{ REG_PC,               "pc"            },
	{ SYSCALL_NR,           "syscall_nr"    },
	{ REG_AR_BASE,          "ar0"           },
	{ REG_AR_BASE+1,        "ar1"           },
	{ REG_AR_BASE+2,        "ar2"           },
	{ REG_AR_BASE+3,        "ar3"           },
	{ REG_AR_BASE+4,        "ar4"           },
	{ REG_AR_BASE+5,        "ar5"           },
	{ REG_AR_BASE+6,        "ar6"           },
	{ REG_AR_BASE+7,        "ar7"           },
	{ REG_AR_BASE+8,        "ar8"           },
	{ REG_AR_BASE+9,        "ar9"           },
	{ REG_AR_BASE+10,       "ar10"          },
	{ REG_AR_BASE+11,       "ar11"          },
	{ REG_AR_BASE+12,       "ar12"          },
	{ REG_AR_BASE+13,       "ar13"          },
	{ REG_AR_BASE+14,       "ar14"          },
	{ REG_AR_BASE+15,       "ar15"          },
	{ REG_AR_BASE+16,       "ar16"          },
	{ REG_AR_BASE+17,       "ar17"          },
	{ REG_AR_BASE+18,       "ar18"          },
	{ REG_AR_BASE+19,       "ar19"          },
	{ REG_AR_BASE+20,       "ar20"          },
	{ REG_AR_BASE+21,       "ar21"          },
	{ REG_AR_BASE+22,       "ar22"          },
	{ REG_AR_BASE+23,       "ar23"          },
	{ REG_AR_BASE+24,       "ar24"          },
	{ REG_AR_BASE+25,       "ar25"          },
	{ REG_AR_BASE+26,       "ar26"          },
	{ REG_AR_BASE+27,       "ar27"          },
	{ REG_AR_BASE+28,       "ar28"          },
	{ REG_AR_BASE+29,       "ar29"          },
	{ REG_AR_BASE+30,       "ar30"          },
	{ REG_AR_BASE+31,       "ar31"          },
	{ REG_AR_BASE+32,       "ar32"          },
	{ REG_AR_BASE+33,       "ar33"          },
	{ REG_AR_BASE+34,       "ar34"          },
	{ REG_AR_BASE+35,       "ar35"          },
	{ REG_AR_BASE+36,       "ar36"          },
	{ REG_AR_BASE+37,       "ar37"          },
	{ REG_AR_BASE+38,       "ar38"          },
	{ REG_AR_BASE+39,       "ar39"          },
	{ REG_AR_BASE+40,       "ar40"          },
	{ REG_AR_BASE+41,       "ar41"          },
	{ REG_AR_BASE+42,       "ar42"          },
	{ REG_AR_BASE+43,       "ar43"          },
	{ REG_AR_BASE+44,       "ar44"          },
	{ REG_AR_BASE+45,       "ar45"          },
	{ REG_AR_BASE+46,       "ar46"          },
	{ REG_AR_BASE+47,       "ar47"          },
	{ REG_AR_BASE+48,       "ar48"          },
	{ REG_AR_BASE+49,       "ar49"          },
	{ REG_AR_BASE+50,       "ar50"          },
	{ REG_AR_BASE+51,       "ar51"          },
	{ REG_AR_BASE+52,       "ar52"          },
	{ REG_AR_BASE+53,       "ar53"          },
	{ REG_AR_BASE+54,       "ar54"          },
	{ REG_AR_BASE+55,       "ar55"          },
	{ REG_AR_BASE+56,       "ar56"          },
	{ REG_AR_BASE+57,       "ar57"          },
	{ REG_AR_BASE+58,       "ar58"          },
	{ REG_AR_BASE+59,       "ar59"          },
	{ REG_AR_BASE+60,       "ar60"          },
	{ REG_AR_BASE+61,       "ar61"          },
	{ REG_AR_BASE+62,       "ar62"          },
	{ REG_AR_BASE+63,       "ar63"          },
	{ REG_LBEG,             "lbeg"          },
	{ REG_LEND,             "lend"          },
	{ REG_LCOUNT,           "lcount"        },
	{ REG_SAR,              "sar"           },
	{ REG_WB,               "wb"            },
	{ REG_WS,               "ws"            },
	{ REG_PS,               "ps"            },
#endif

	/* Other fields in "struct user" */
#if defined(S390) || defined(S390X)
	XLAT_UOFF(u_tsize),
	XLAT_UOFF(u_dsize),
	XLAT_UOFF(u_ssize),
	XLAT_UOFF(start_code),
	/* S390[X] has no start_data */
	XLAT_UOFF(start_stack),
	XLAT_UOFF(signal),
	XLAT_UOFF(u_ar0),
	XLAT_UOFF(magic),
	XLAT_UOFF(u_comm),
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(POWERPC)
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(I386) || defined(X86_64) || defined(X32)
	XLAT_UOFF(u_fpvalid),
	XLAT_UOFF(i387),
	XLAT_UOFF(u_tsize),
	XLAT_UOFF(u_dsize),
	XLAT_UOFF(u_ssize),
	XLAT_UOFF(start_code),
	XLAT_UOFF(start_stack),
	XLAT_UOFF(signal),
	XLAT_UOFF(reserved),
	XLAT_UOFF(u_ar0),
	XLAT_UOFF(u_fpstate),
	XLAT_UOFF(magic),
	XLAT_UOFF(u_comm),
	XLAT_UOFF(u_debugreg),
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(IA64)
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(ARM)
	XLAT_UOFF(u_fpvalid),
	XLAT_UOFF(u_tsize),
	XLAT_UOFF(u_dsize),
	XLAT_UOFF(u_ssize),
	XLAT_UOFF(start_code),
	XLAT_UOFF(start_stack),
	XLAT_UOFF(signal),
	XLAT_UOFF(reserved),
	XLAT_UOFF(u_ar0),
	XLAT_UOFF(magic),
	XLAT_UOFF(u_comm),
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(AARCH64)
	/* nothing */
#elif defined(M68K)
	XLAT_UOFF(u_fpvalid),
	XLAT_UOFF(m68kfp),
	XLAT_UOFF(u_tsize),
	XLAT_UOFF(u_dsize),
	XLAT_UOFF(u_ssize),
	XLAT_UOFF(start_code),
	XLAT_UOFF(start_stack),
	XLAT_UOFF(signal),
	XLAT_UOFF(reserved),
	XLAT_UOFF(u_ar0),
	XLAT_UOFF(u_fpstate),
	XLAT_UOFF(magic),
	XLAT_UOFF(u_comm),
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(MIPS) || defined(LINUX_MIPSN32)
	XLAT_UOFF(u_tsize),
	XLAT_UOFF(u_dsize),
	XLAT_UOFF(u_ssize),
	XLAT_UOFF(start_code),
	XLAT_UOFF(start_data),
	XLAT_UOFF(start_stack),
	XLAT_UOFF(signal),
	XLAT_UOFF(u_ar0),
	XLAT_UOFF(magic),
	XLAT_UOFF(u_comm),
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(ALPHA)
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(SPARC)
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(SPARC64)
	XLAT_UOFF(u_tsize),
	XLAT_UOFF(u_dsize),
	XLAT_UOFF(u_ssize),
	XLAT_UOFF(signal),
	XLAT_UOFF(magic),
	XLAT_UOFF(u_comm),
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(HPPA)
	/* nothing */
#elif defined(SH) || defined(SH64)
	XLAT_UOFF(u_fpvalid),
	XLAT_UOFF(u_tsize),
	XLAT_UOFF(u_dsize),
	XLAT_UOFF(u_ssize),
	XLAT_UOFF(start_code),
	XLAT_UOFF(start_data),
	XLAT_UOFF(start_stack),
	XLAT_UOFF(signal),
	XLAT_UOFF(u_ar0),
	XLAT_UOFF(u_fpstate),
	XLAT_UOFF(magic),
	XLAT_UOFF(u_comm),
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(CRISV10) || defined(CRISV32)
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(TILE)
	/* nothing */
#elif defined(MICROBLAZE)
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(AVR32)
	XLAT_UOFF(u_tsize),
	XLAT_UOFF(u_dsize),
	XLAT_UOFF(u_ssize),
	XLAT_UOFF(start_code),
	XLAT_UOFF(start_data),
	XLAT_UOFF(start_stack),
	XLAT_UOFF(signal),
	XLAT_UOFF(u_ar0),
	XLAT_UOFF(magic),
	XLAT_UOFF(u_comm),
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(BFIN)
	XLAT_UOFF(u_tsize),
	XLAT_UOFF(u_dsize),
	XLAT_UOFF(u_ssize),
	XLAT_UOFF(start_code),
	XLAT_UOFF(signal),
	XLAT_UOFF(u_ar0),
	XLAT_UOFF(magic),
	XLAT_UOFF(u_comm),
	{ sizeof(struct user),	"sizeof(struct user)"			},
#elif defined(OR1K)
	/* nothing */
#elif defined(METAG)
	/* nothing */
#elif defined(XTENSA)
	/* nothing */
#elif defined(ARC)
	/* nothing */
#endif
	XLAT_END
};

int
sys_ptrace(struct tcb *tcp)
{
	const struct xlat *x;
	unsigned long addr;

	if (entering(tcp)) {
		printxval(ptrace_cmds, tcp->u_arg[0], "PTRACE_???");
		tprintf(", %lu, ", tcp->u_arg[1]);

		addr = tcp->u_arg[2];
		if (tcp->u_arg[0] == PTRACE_PEEKUSER
		 || tcp->u_arg[0] == PTRACE_POKEUSER
		) {
			for (x = struct_user_offsets; x->str; x++) {
				if (x->val >= addr)
					break;
			}
			if (!x->str)
				tprintf("%#lx, ", addr);
			else if (x->val > addr && x != struct_user_offsets) {
				x--;
				tprintf("%s + %ld, ", x->str, addr - x->val);
			}
			else
				tprintf("%s, ", x->str);
		} else
#ifdef PTRACE_GETREGSET
		if (tcp->u_arg[0] == PTRACE_GETREGSET
		 || tcp->u_arg[0] == PTRACE_SETREGSET
		) {
			printxval(nt_descriptor_types, tcp->u_arg[2], "NT_???");
			tprints(", ");
		} else
#endif
			tprintf("%#lx, ", addr);


		switch (tcp->u_arg[0]) {
#ifndef IA64
		case PTRACE_PEEKDATA:
		case PTRACE_PEEKTEXT:
		case PTRACE_PEEKUSER:
			break;
#endif
		case PTRACE_CONT:
		case PTRACE_SINGLESTEP:
		case PTRACE_SYSCALL:
		case PTRACE_DETACH:
			printsignal(tcp->u_arg[3]);
			break;
#ifdef PTRACE_SETOPTIONS
		case PTRACE_SETOPTIONS:
			printflags(ptrace_setoptions_flags, tcp->u_arg[3], "PTRACE_O_???");
			break;
#endif
#ifdef PTRACE_SETSIGINFO
		case PTRACE_SETSIGINFO: {
			printsiginfo_at(tcp, tcp->u_arg[3]);
			break;
		}
#endif
#ifdef PTRACE_GETSIGINFO
		case PTRACE_GETSIGINFO:
			/* Don't print anything, do it at syscall return. */
			break;
#endif
#ifdef PTRACE_GETREGSET
		case PTRACE_GETREGSET:
			break;
		case PTRACE_SETREGSET:
			tprint_iov(tcp, /*len:*/ 1, tcp->u_arg[3], /*as string:*/ 0);
			break;
#endif
		default:
			tprintf("%#lx", tcp->u_arg[3]);
			break;
		}
	} else {
		switch (tcp->u_arg[0]) {
		case PTRACE_PEEKDATA:
		case PTRACE_PEEKTEXT:
		case PTRACE_PEEKUSER:
#ifdef IA64
			return RVAL_HEX;
#else
			printnum(tcp, tcp->u_arg[3], "%#lx");
			break;
#endif
#ifdef PTRACE_GETSIGINFO
		case PTRACE_GETSIGINFO: {
			printsiginfo_at(tcp, tcp->u_arg[3]);
			break;
		}
#endif
#ifdef PTRACE_GETREGSET
		case PTRACE_GETREGSET:
			tprint_iov(tcp, /*len:*/ 1, tcp->u_arg[3], /*as string:*/ 0);
			break;
#endif
		}
	}
	return 0;
}
