#include "defs.h"
#include "regs.h"
#include "ptrace.h"

#if defined HAVE_ASM_SIGCONTEXT_H && !defined HAVE_STRUCT_SIGCONTEXT
# include <asm/sigcontext.h>
#endif

#ifndef NSIG
# warning NSIG is not defined, using 32
# define NSIG 32
#elif NSIG < 32
# error NSIG < 32
#endif

int
sys_sigreturn(struct tcb *tcp)
{
#if defined AARCH64 || defined ARM
	if (entering(tcp)) {
# define SIZEOF_STRUCT_SIGINFO 128
# define SIZEOF_STRUCT_SIGCONTEXT (21 * 4)
# define OFFSETOF_STRUCT_UCONTEXT_UC_SIGMASK (5 * 4 + SIZEOF_STRUCT_SIGCONTEXT)
		const long addr =
# ifdef AARCH64
			current_personality == 0 ?
				(*aarch64_sp_ptr + SIZEOF_STRUCT_SIGINFO +
				 offsetof(struct ucontext, uc_sigmask)) :
# endif
				(*arm_sp_ptr +
				 OFFSETOF_STRUCT_UCONTEXT_UC_SIGMASK);
		tprints("{mask=");
		print_sigset_addr_len(tcp, addr, NSIG / 8);
		tprints("}");
	}
#elif defined(S390) || defined(S390X)
	if (entering(tcp)) {
		long mask[NSIG / 8 / sizeof(long)];
		tprints("{mask=");
		const long addr = *s390_frame_ptr + __SIGNAL_FRAMESIZE;
		if (umove(tcp, addr, &mask) < 0) {
			tprintf("%#lx", addr);
		} else {
# ifdef S390
			long v = mask[0];
			mask[0] = mask[1];
			mask[1] = v;
# endif
			tprintsigmask_addr("", mask);
		}
		tprints("}");
	}
#elif defined I386 || defined X86_64 || defined X32
	if (entering(tcp)) {
# ifndef I386
		if (current_personality != 1) {
			const unsigned long addr =
				(unsigned long) *x86_64_rsp_ptr +
				offsetof(struct ucontext, uc_sigmask);
			tprints("{mask=");
			print_sigset_addr_len(tcp, addr, NSIG / 8);
			tprints("}");
			return 0;
		}
# endif
		/*
		 * On i386, sigcontext is followed on stack by struct fpstate
		 * and after it an additional u32 extramask which holds
		 * upper half of the mask.
		 */
		struct {
			uint32_t struct_sigcontext_padding1[20];
			uint32_t oldmask;
			uint32_t struct_sigcontext_padding2;
			uint32_t struct_fpstate_padding[156];
			uint32_t extramask;
		} frame;
		tprints("{mask=");
		if (umove(tcp, *i386_esp_ptr, &frame) < 0) {
			tprintf("%#lx", (unsigned long) *i386_esp_ptr);
		} else {
			uint32_t mask[2] = { frame.oldmask, frame.extramask };
			tprintsigmask_addr("", mask);
		}
		tprints("}");
	}
#elif defined(IA64)
	if (entering(tcp)) {
		/* offsetof(struct sigframe, sc) */
#		define OFFSETOF_STRUCT_SIGFRAME_SC	0xA0
		const long addr = *ia64_frame_ptr + 16 +
				  OFFSETOF_STRUCT_SIGFRAME_SC +
				  offsetof(struct sigcontext, sc_mask);
		tprints("{mask=");
		print_sigset_addr_len(tcp, addr, NSIG / 8);
		tprints("}");
	}
#elif defined(POWERPC)
	if (entering(tcp)) {
		long esp = ppc_regs.gpr[1];
		struct sigcontext sc;

		/* Skip dummy stack frame. */
#ifdef POWERPC64
		if (current_personality == 0)
			esp += 128;
		else
#endif
			esp += 64;

		tprints("{mask=");
		if (umove(tcp, esp, &sc) < 0) {
			tprintf("%#lx", esp);
		} else {
			unsigned long mask[NSIG / 8 / sizeof(long)];
#ifdef POWERPC64
			mask[0] = sc.oldmask | (sc._unused[3] << 32);
#else
			mask[0] = sc.oldmask;
			mask[1] = sc._unused[3];
#endif
			tprintsigmask_addr("", mask);
		}
		tprints("}");
	}
#elif defined(M68K)
	if (entering(tcp)) {
		long addr;
		if (upeek(tcp->pid, 4*PT_USP, &addr) < 0)
			return 0;
		tprints("{mask=");
		print_sigset_addr_len(tcp, addr, NSIG / 8);
		tprints("}");
	}
#elif defined(ALPHA)
	if (entering(tcp)) {
		long addr;
		if (upeek(tcp->pid, REG_FP, &addr) < 0)
			return 0;
		addr += offsetof(struct sigcontext, sc_mask);
		tprints("{mask=");
		print_sigset_addr_len(tcp, addr, NSIG / 8);
		tprints("}");
	}
#elif defined(SPARC) || defined(SPARC64)
	if (entering(tcp)) {
		long fp = sparc_regs.u_regs[U_REG_FP] + sizeof(struct sparc_stackf);
		struct {
			struct pt_regs si_regs;
			int si_mask;
			void *fpu_save;
			long insns[2] __attribute__ ((aligned (8)));
			unsigned int extramask[NSIG / 8 / sizeof(int) - 1];
		} frame;

		tprints("{mask=");
		if (umove(tcp, fp, &frame) < 0) {
			tprintf("%#lx", fp);
		} else {
			unsigned int mask[NSIG / 8 / sizeof(int)];

			mask[0] = frame.si_mask;
			memcpy(mask + 1, frame.extramask, sizeof(frame.extramask));
			tprintsigmask_addr("", mask);
		}
		tprints("}");
	}
#elif defined MIPS
	if (entering(tcp)) {
# if defined LINUX_MIPSO32
		/*
		 * offsetof(struct sigframe, sf_mask) ==
		 * sizeof(sf_ass) + sizeof(sf_pad) + sizeof(struct sigcontext)
		 */
		const long addr = mips_REG_SP + 6 * 4 +
				  sizeof(struct sigcontext);
# else
		/*
		 * This decodes rt_sigreturn.
		 * The 64-bit ABIs do not have sigreturn.
		 *
		 * offsetof(struct rt_sigframe, rs_uc) ==
		 * sizeof(sf_ass) + sizeof(sf_pad) + sizeof(struct siginfo)
		 */
		const long addr = mips_REG_SP + 6 * 4 + 128 +
				  offsetof(struct ucontext, uc_sigmask);
# endif
		tprints("{mask=");
		print_sigset_addr_len(tcp, addr, NSIG / 8);
		tprints("}");
	}
#elif defined(CRISV10) || defined(CRISV32)
	if (entering(tcp)) {
		long regs[PT_MAX+1];
		if (ptrace(PTRACE_GETREGS, tcp->pid, NULL, (long)regs) < 0) {
			perror_msg("sigreturn: PTRACE_GETREGS");
			return 0;
		}
		const long addr = regs[PT_USP] +
			offsetof(struct sigcontext, oldmask);
		tprints("{mask=");
		print_sigset_addr_len(tcp, addr, NSIG / 8);
		tprints("}");
	}
#elif defined(TILE)
	if (entering(tcp)) {
		/* offset of ucontext in the kernel's sigframe structure */
# define SIGFRAME_UC_OFFSET C_ABI_SAVE_AREA_SIZE + sizeof(siginfo_t)
		const long addr = tile_regs.sp + SIGFRAME_UC_OFFSET +
				  offsetof(struct ucontext, uc_sigmask);
		tprints("{mask=");
		print_sigset_addr_len(tcp, addr, NSIG / 8);
		tprints("}");
	}
#elif defined(MICROBLAZE)
	/* TODO: Verify that this is correct...  */
	if (entering(tcp)) {
		long addr;
		/* Read r1, the stack pointer.  */
		if (upeek(tcp->pid, 1 * 4, &addr) < 0)
			return 0;
		addr += offsetof(struct sigcontext, oldmask);
		tprints("{mask=");
		print_sigset_addr_len(tcp, addr, NSIG / 8);
		tprints("}");
	}
#else
# warning sigreturn/rt_sigreturn signal mask decoding is not implemented for this architecture
#endif
	return 0;
}
