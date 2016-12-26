/*
 * Copyright (c) 2015 Dmitry V. Levin <ldv@altlinux.org>
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
#include <fcntl.h>

#include "xlat/uffd_flags.h"

SYS_FUNC(userfaultfd)
{
	printflags(uffd_flags, tcp->u_arg[0], "UFFD_???");

	return RVAL_DECODED | RVAL_FD;
}

#ifdef HAVE_LINUX_USERFAULTFD_H
# include <linux/ioctl.h>
# include <linux/userfaultfd.h>

# include "xlat/uffd_api_flags.h"
# include "xlat/uffd_copy_flags.h"
# include "xlat/uffd_register_ioctl_flags.h"
# include "xlat/uffd_register_mode_flags.h"
# include "xlat/uffd_zeropage_flags.h"

static void
tprintf_uffdio_range(const struct uffdio_range *range)
{
	tprintf("{start=%#" PRI__x64 ", len=%#" PRI__x64 "}",
		range->start, range->len);
}

int
uffdio_ioctl(struct tcb *const tcp, const unsigned int code,
	     const kernel_ulong_t arg)
{
	switch (code) {
	case UFFDIO_API: {
		struct uffdio_api ua;
		if (entering(tcp)) {
			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &ua))
				return RVAL_DECODED | 1;
			/* Features is intended to contain some flags, but
			 * there aren't any defined yet.
			 */
			tprintf("{api=%#" PRI__x64
				", features=%#" PRI__x64,
				ua.api, ua.features);
		} else {
			if (!syserror(tcp) && !umove(tcp, arg, &ua)) {
				tprintf(", features.out=%#" PRI__x64
					", ioctls=", ua.features);
				printflags64(uffd_api_flags, ua.ioctls,
					     "_UFFDIO_???");
			}
			tprints("}");
		}
		return 1;
	}

	case UFFDIO_COPY: {
		struct uffdio_copy uc;
		if (entering(tcp)) {
			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &uc))
				return RVAL_DECODED | 1;
			tprintf("{dst=%#" PRI__x64 ", src=%#" PRI__x64
				", len=%#" PRI__x64 ", mode=",
				uc.dst, uc.src, uc.len);
			printflags64(uffd_copy_flags, uc.mode,
				     "UFFDIO_COPY_???");
		} else {
			if (!syserror(tcp) && !umove(tcp, arg, &uc))
				tprintf(", copy=%#" PRI__x64, uc.copy);
			tprints("}");
		}
		return 1;
	}

	case UFFDIO_REGISTER: {
		struct uffdio_register ur;
		if (entering(tcp)) {
			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &ur))
				return RVAL_DECODED | 1;
			tprints("{range=");
			tprintf_uffdio_range(&ur.range);
			tprints(", mode=");
			printflags64(uffd_register_mode_flags, ur.mode,
				     "UFFDIO_REGISTER_MODE_???");
		} else {
			if (!syserror(tcp) && !umove(tcp, arg, &ur)) {
				tprints(", ioctls=");
				printflags64(uffd_register_ioctl_flags,
					     ur.ioctls, "UFFDIO_???");
			}
			tprints("}");
		}
		return 1;
	}

	case UFFDIO_UNREGISTER:
	case UFFDIO_WAKE: {
		struct uffdio_range ura;
		tprints(", ");
		if (!umove_or_printaddr(tcp, arg, &ura))
			tprintf_uffdio_range(&ura);
		return RVAL_DECODED | 1;
	}

	case UFFDIO_ZEROPAGE: {
		struct uffdio_zeropage uz;
		if (entering(tcp)) {
			tprints(", ");
			if (umove_or_printaddr(tcp, arg, &uz))
				return RVAL_DECODED | 1;
			tprints("{range=");
			tprintf_uffdio_range(&uz.range);
			tprints(", mode=");
			printflags64(uffd_zeropage_flags, uz.mode,
				     "UFFDIO_ZEROPAGE_???");
		} else {
			if (!syserror(tcp) && !umove(tcp, arg, &uz))
				tprintf(", zeropage=%#" PRI__x64, uz.zeropage);
			tprints("}");
		}
		return 1;
	}

	default:
		return RVAL_DECODED;
	}
}
#endif /* HAVE_LINUX_USERFAULTFD_H */
