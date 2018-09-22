/*
 * Copyright (c) 2018 The strace developers.
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
#include "print_fields.h"
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/nbd.h>

#define XLAT_MACROS_ONLY
# include "xlat/nbd_ioctl_cmds.h"
#undef XLAT_MACROS_ONLY

#include "xlat/nbd_ioctl_flags.h"

int
nbd_ioctl(struct tcb *const tcp, const unsigned int code,
	  const kernel_ulong_t arg)
{
	switch (code) {
	case NBD_DISCONNECT:
	case NBD_CLEAR_SOCK:
	case NBD_DO_IT:
	case NBD_CLEAR_QUE:
	case NBD_PRINT_DEBUG:
		return RVAL_IOCTL_DECODED;

	case NBD_SET_SOCK:
		tprints(", ");
		printfd(tcp, arg);
		return RVAL_IOCTL_DECODED;

	case NBD_SET_BLKSIZE:
	case NBD_SET_SIZE:
	case NBD_SET_SIZE_BLOCKS:
	case NBD_SET_TIMEOUT:
		tprints(", ");
		tprintf("%" PRI_klu, arg);
		return RVAL_IOCTL_DECODED;

	case NBD_SET_FLAGS:
		tprints(", ");
		printflags(nbd_ioctl_flags, arg, "NBD_IOC_FLAG_???");
		return RVAL_IOCTL_DECODED;

	default:
		return RVAL_DECODED;
	}
}
