#include "defs.h"

#include <linux/personality.h>

#include "xlat/personality_options.h"

SYS_FUNC(personality)
{
	if (entering(tcp)) {
		const unsigned int pers = tcp->u_arg[0];
		if (0xffffffff == pers)
			tprints("0xffffffff");
		else
			printflags(personality_options, pers, "PER_???");
		return 0;
	}

	tcp->auxstr = sprintflags("", personality_options, tcp->u_rval);
	return RVAL_HEX | RVAL_STR;
}
