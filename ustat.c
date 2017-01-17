/*
 * Copyright (c) 2017 JingPiao Chen <chenjingpiao@gmail.com>
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
#ifdef HAVE_USTAT_H
# include DEF_MPERS_TYPE(struct_ustat)
# include <ustat.h>
typedef struct ustat struct_ustat;
#endif /* HAVE_USTAT_H */

#include MPERS_DEFS

SYS_FUNC(ustat)
{
	if (entering(tcp))
		print_dev_t((unsigned int) tcp->u_arg[0]);
	else {
		tprints(", ");
#ifdef HAVE_USTAT_H
		struct_ustat ust;

		if (!umove_or_printaddr(tcp, tcp->u_arg[1], &ust))
			tprintf("{f_tfree=%llu, f_tinode=%llu}",
				zero_extend_signed_to_ull(ust.f_tfree),
				zero_extend_signed_to_ull(ust.f_tinode));
#else /* !HAVE_USTAT_H */
		printaddr(tcp->u_arg[1]);
#endif /* HAVE_USTAT_H */
	}

	return 0;
}
