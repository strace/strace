#include "defs.h"

#include <linux/personality.h>

#include "xlat/personality_options.h"

int
sys_personality(struct tcb *tcp)
{
	if (entering(tcp))
		printxval(personality_options, tcp->u_arg[0], "PER_???");
	return 0;
}
