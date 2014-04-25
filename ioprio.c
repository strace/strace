#include "defs.h"

enum {
	IOPRIO_WHO_PROCESS = 1,
	IOPRIO_WHO_PGRP,
	IOPRIO_WHO_USER
};

#include "xlat/ioprio_who.h"

enum {
	IOPRIO_CLASS_NONE,
	IOPRIO_CLASS_RT,
	IOPRIO_CLASS_BE,
	IOPRIO_CLASS_IDLE
};

#include "xlat/ioprio_class.h"

#define IOPRIO_CLASS_SHIFT	(13)
#define IOPRIO_PRIO_MASK	((1ul << IOPRIO_CLASS_SHIFT) - 1)

#define IOPRIO_PRIO_CLASS(mask)	((mask) >> IOPRIO_CLASS_SHIFT)
#define IOPRIO_PRIO_DATA(mask)	((mask) & IOPRIO_PRIO_MASK)

static const char *
sprint_ioprio(int ioprio)
{
	static char outstr[256];
	const char *str;
	int class, data;

	class = IOPRIO_PRIO_CLASS(ioprio);
	data = IOPRIO_PRIO_DATA(ioprio);
	str = xlookup(ioprio_class, class);
	if (str)
		sprintf(outstr, "IOPRIO_PRIO_VALUE(%s,%d)", str, data);
	else
		sprintf(outstr, "IOPRIO_PRIO_VALUE(%#x /* %s */,%d)",
			class, "IOPRIO_CLASS_???", data);

	return outstr;
}

int
sys_ioprio_get(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* int which */
		printxval(ioprio_who, tcp->u_arg[0], "IOPRIO_WHO_???");
		/* int who */
		tprintf(", %d", (int) tcp->u_arg[1]);
		return 0;
	} else {
		if (syserror(tcp))
			return 0;

		tcp->auxstr = sprint_ioprio(tcp->u_rval);
		return RVAL_STR;
	}
}

int
sys_ioprio_set(struct tcb *tcp)
{
	if (entering(tcp)) {
		/* int which */
		printxval(ioprio_who, tcp->u_arg[0], "IOPRIO_WHO_???");
		/* int who */
		tprintf(", %d, ", (int) tcp->u_arg[1]);
		/* int ioprio */
		tprints(sprint_ioprio(tcp->u_arg[2]));
	}
	return 0;
}
