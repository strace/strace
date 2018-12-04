#include <stdint.h>

#define S390_SIGNAL_FRAMESIZE      96

#define SIGNAL_FRAMESIZE	S390_SIGNAL_FRAMESIZE
#define PTR_TYPE		uint32_t
#define arch_sigreturn	s390_arch_sigreturn
#include "s390/arch_sigreturn.c"
#undef arch_sigreturn
#undef PTR_TYPE
#undef SIGNAL_FRAMESIZE

#define arch_sigreturn	s390x_arch_sigreturn
#include "s390/arch_sigreturn.c"
#undef arch_sigreturn

static void
arch_sigreturn(struct tcb *tcp)
{
	if (tcp->currpers == 1)
		s390_arch_sigreturn(tcp);
	else
		s390x_arch_sigreturn(tcp);
}
