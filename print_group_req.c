/*
 * Copyright (c) 2015-2017 The strace developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "defs.h"
#include <netinet/in.h>

#ifdef MCAST_JOIN_GROUP

# include DEF_MPERS_TYPE(struct_group_req)
typedef struct group_req struct_group_req;

#endif /* MCAST_JOIN_GROUP */

#include MPERS_DEFS

#ifdef MCAST_JOIN_GROUP

# include "print_fields.h"

MPERS_PRINTER_DECL(void, print_group_req, struct tcb *const tcp,
		   const kernel_ulong_t addr, const int len)
{
	struct_group_req greq;

	if (len < (int) sizeof(greq)) {
		printaddr(addr);
	} else if (!umove_or_printaddr(tcp, addr, &greq)) {
		PRINT_FIELD_IFINDEX("{", greq, gr_interface);
		PRINT_FIELD_SOCKADDR(", ", greq, gr_group);
		tprints("}");
	}
}

#endif /* MCAST_JOIN_GROUP */
