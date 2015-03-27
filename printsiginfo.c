#include "defs.h"

#include "xlat/sigbus_codes.h"
#include "xlat/sigchld_codes.h"
#include "xlat/sigfpe_codes.h"
#include "xlat/sigill_codes.h"
#include "xlat/siginfo_codes.h"
#include "xlat/sigpoll_codes.h"
#include "xlat/sigprof_codes.h"
#include "xlat/sigsegv_codes.h"
#include "xlat/sigsys_codes.h"
#include "xlat/sigtrap_codes.h"

#ifdef SIGEMT
# include "xlat/sigemt_codes.h"
#endif

#ifndef SI_FROMUSER
# define SI_FROMUSER(sip)	((sip)->si_code <= 0)
#endif

static void
printsigsource(const siginfo_t *sip)
{
	tprintf(", si_pid=%lu, si_uid=%lu",
		(unsigned long) sip->si_pid,
		(unsigned long) sip->si_uid);
}

static void
printsigval(const siginfo_t *sip, bool verbose)
{
	if (!verbose)
		tprints(", ...");
	else
		tprintf(", si_value={int=%u, ptr=%#lx}",
			sip->si_int,
			(unsigned long) sip->si_ptr);
}

static void
print_si_code(int si_signo, int si_code)
{
	const char *code = xlookup(siginfo_codes, si_code);

	if (!code) {
		switch (si_signo) {
		case SIGTRAP:
			code = xlookup(sigtrap_codes, si_code);
			break;
		case SIGCHLD:
			code = xlookup(sigchld_codes, si_code);
			break;
		case SIGPOLL:
			code = xlookup(sigpoll_codes, si_code);
			break;
		case SIGPROF:
			code = xlookup(sigprof_codes, si_code);
			break;
		case SIGILL:
			code = xlookup(sigill_codes, si_code);
			break;
#ifdef SIGEMT
		case SIGEMT:
			code = xlookup(sigemt_codes, si_code);
			break;
#endif
		case SIGFPE:
			code = xlookup(sigfpe_codes, si_code);
			break;
		case SIGSEGV:
			code = xlookup(sigsegv_codes, si_code);
			break;
		case SIGBUS:
			code = xlookup(sigbus_codes, si_code);
			break;
		case SIGSYS:
			code = xlookup(sigsys_codes, si_code);
			break;
		}
	}

	if (code)
		tprints(code);
	else
		tprintf("%#x", si_code);
}

static void
print_si_info(const siginfo_t *sip, bool verbose)
{
	if (sip->si_errno) {
		tprints(", si_errno=");
		if ((unsigned) sip->si_errno < nerrnos
		    && errnoent[sip->si_errno])
			tprints(errnoent[sip->si_errno]);
		else
			tprintf("%d", sip->si_errno);
	}

	if (SI_FROMUSER(sip)) {
		switch (sip->si_code) {
		case SI_USER:
			printsigsource(sip);
			break;
		case SI_TKILL:
			printsigsource(sip);
			break;
#if defined HAVE_SIGINFO_T_SI_TIMERID && defined HAVE_SIGINFO_T_SI_OVERRUN
		case SI_TIMER:
			tprintf(", si_timerid=%#x, si_overrun=%d",
				sip->si_timerid, sip->si_overrun);
			printsigval(sip, verbose);
			break;
#endif
		default:
			printsigsource(sip);
			if (sip->si_ptr)
				printsigval(sip, verbose);
			break;
		}
	} else {
		switch (sip->si_signo) {
		case SIGCHLD:
			printsigsource(sip);
			tprints(", si_status=");
			if (sip->si_code == CLD_EXITED)
				tprintf("%d", sip->si_status);
			else
				printsignal(sip->si_status);
			if (!verbose)
				tprints(", ...");
			else
				tprintf(", si_utime=%llu, si_stime=%llu",
					(unsigned long long) sip->si_utime,
					(unsigned long long) sip->si_stime);
			break;
		case SIGILL: case SIGFPE:
		case SIGSEGV: case SIGBUS:
			tprintf(", si_addr=%#lx",
				(unsigned long) sip->si_addr);
			break;
		case SIGPOLL:
			switch (sip->si_code) {
			case POLL_IN: case POLL_OUT: case POLL_MSG:
				tprintf(", si_band=%ld",
					(long) sip->si_band);
				break;
			}
			break;
#ifdef HAVE_SIGINFO_T_SI_SYSCALL
		case SIGSYS:
			tprintf(", si_call_addr=%#lx, si_syscall=%d, si_arch=%u",
				(unsigned long) sip->si_call_addr,
				sip->si_syscall, sip->si_arch);
			break;
#endif
		default:
			if (sip->si_pid || sip->si_uid)
				printsigsource(sip);
			if (sip->si_ptr)
				printsigval(sip, verbose);
		}
	}
}

void
printsiginfo(const siginfo_t *sip, bool verbose)
{
	if (sip->si_signo == 0) {
		tprints("{}");
		return;
	}
	tprints("{si_signo=");
	printsignal(sip->si_signo);

	tprints(", si_code=");
	print_si_code(sip->si_signo, sip->si_code);

#ifdef SI_NOINFO
	if (sip->si_code != SI_NOINFO)
#endif
		print_si_info(sip, verbose);

	tprints("}");
}

void
printsiginfo_at(struct tcb *tcp, long addr)
{
	siginfo_t si;
	if (!addr) {
		tprints("NULL");
		return;
	}
	if (syserror(tcp) || umove(tcp, addr, &si) < 0) {
		tprintf("%#lx", addr);
		return;
	}
	printsiginfo(&si, verbose(tcp));
}
