#include "defs.h"
#include <signal.h>
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

#include "arch_sigreturn.c"

SYS_FUNC(sigreturn)
{
	arch_sigreturn(tcp);

	return RVAL_DECODED;
}
