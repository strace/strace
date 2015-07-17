#include "defs.h"

#include <linux/personality.h>

#include "xlat/personality_options.h"

SYS_FUNC(personality)
{
	printxval(personality_options, tcp->u_arg[0], "PER_???");

	return RVAL_DECODED;
}
