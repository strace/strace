#include "defs.h"

#include DEF_MPERS_TYPE(time_t)
#include DEF_MPERS_TYPE(timespec_t)
#include DEF_MPERS_TYPE(timeval_t)

typedef struct timespec timespec_t;
typedef struct timeval timeval_t;

#include MPERS_DEFS

#ifndef UTIME_NOW
# define UTIME_NOW ((1l << 30) - 1l)
#endif
#ifndef UTIME_OMIT
# define UTIME_OMIT ((1l << 30) - 2l)
#endif

static const char time_fmt[] = "{%jd, %jd}";

static void
print_timespec_t(const timespec_t *t)
{
	tprintf(time_fmt, (intmax_t) t->tv_sec, (intmax_t) t->tv_nsec);
}

static void
print_timespec_t_utime(const timespec_t *t)
{
	switch (t->tv_nsec) {
	case UTIME_NOW:
		tprints("UTIME_NOW");
		break;
	case UTIME_OMIT:
		tprints("UTIME_OMIT");
		break;
	default:
		print_timespec_t(t);
		break;
	}
}

static void
print_timeval_t(const timeval_t *t)
{
	tprintf(time_fmt, (intmax_t) t->tv_sec, (intmax_t) t->tv_usec);
}

MPERS_PRINTER_DECL(void, print_timespec_utime_pair)(struct tcb *tcp, const long addr)
{
	timespec_t t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return;

	tprints("[");
	print_timespec_t_utime(&t[0]);
	tprints(", ");
	print_timespec_t_utime(&t[1]);
	tprints("]");
}

MPERS_PRINTER_DECL(void, print_timeval_pair)(struct tcb *tcp, const long addr)
{
	timeval_t t[2];

	if (umove_or_printaddr(tcp, addr, &t))
		return;

	tprints("[");
	print_timeval_t(&t[0]);
	tprints(", ");
	print_timeval_t(&t[1]);
	tprints("]");
}

SYS_FUNC(time)
{
	if (exiting(tcp)) {
		time_t t;

		if (!umove_or_printaddr(tcp, tcp->u_arg[0], &t))
			tprintf("[%jd]", (intmax_t) t);
	}

	return 0;
}
