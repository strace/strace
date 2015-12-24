#include "defs.h"

#include <linux/personality.h>

#include "xlat/personality_options.h"

SYS_FUNC(personality)
{
	if (entering(tcp)) {
		printflags(personality_options, tcp->u_arg[0], "PER_???");
		return 0;
	}

	tcp->auxstr = sprintflags("", personality_options, tcp->u_rval);
	return RVAL_HEX | RVAL_STR;
}
