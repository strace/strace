/*
 * Copyright (c) 2014-2015 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2018 The strace developers.
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

#include "xlat/fan_classes.h"
#include "xlat/fan_init_flags.h"

#ifndef FAN_ALL_CLASS_BITS
# define FAN_ALL_CLASS_BITS (FAN_CLASS_NOTIF | FAN_CLASS_CONTENT | FAN_CLASS_PRE_CONTENT)
#endif
#ifndef FAN_NOFD
# define FAN_NOFD -1
#endif
#ifndef FAN_AUDIT
# define FAN_AUDIT 0x10
#endif

#include "xlat/fan_mark_flags.h"
#include "xlat/fan_event_flags.h"
#include "xlat/fan_responses.h"

static void
print_fanfd(struct tcb *tcp, int fd)
{
	if (fd == FAN_NOFD)
		print_xlat_d(FAN_NOFD);
	else
		printfd(tcp, fd);
}

#define PRINT_FIELD_FANFD(prefix_, where_, field_, tcp_)		\
	do {								\
		STRACE_PRINTF("%s%s=", (prefix_), #field_);		\
		print_fanfd((tcp_), (where_).field_);			\
	} while (0)

bool
decode_fanotify_read(struct tcb *tcp, int fd, const char *fdpath,
		     enum fileops op, kernel_ulong_t addr,
		     kernel_ulong_t addrlen)
{
	struct fev_hdr {
		uint32_t event_len;
		union {
			struct {
				uint8_t  vers;
				uint8_t  reserved;
				uint16_t metadata_len;
			};
			uint32_t vers_v2;
		};
	} fev_hdr;
	uint32_t fev_ver = 0;

	union fev_md {
		struct fev_md_v2 {
			uint64_t ATTRIBUTE_ALIGNED(8) mask;
			int32_t  fd;
			int32_t  pid;
		} v2;
		struct fev_md_v1 {
			int32_t  fd;
			uint64_t ATTRIBUTE_ALIGNED(8) mask;
			int64_t  pid;
		} ATTRIBUTE_PACKED v1;
	} fev_md;

	enum {
		FEV_V1_SIZE = sizeof(struct fev_hdr) + sizeof(struct fev_md_v1),
		FEV_V2_SIZE = sizeof(struct fev_hdr) + sizeof(struct fev_md_v2),
		FEV_MIN_SIZE = MIN(FEV_V1_SIZE, FEV_V2_SIZE),
	};

	kernel_ulong_t pos = 0;

	if (addrlen < sizeof(fev_hdr))
		return false;

	tprints("[");

	do {
		if (pos)
			tprints(", ");

		if (umove(tcp, addr + pos, &fev_hdr)) {
			printaddr_comment(addr + pos);
			break;
		}

		PRINT_FIELD_U("{", fev_hdr, event_len);

		if (fev_hdr.event_len < FEV_MIN_SIZE) {
			tprints(", ... /* invalid event_len */}");

			if (!fev_hdr.event_len)
				goto end_decoded;

			goto end_fev_decoded;
		}

		switch (fev_hdr.vers) {
		case 0: case 1: case 2:
			switch (fev_hdr.vers_v2) {
			case 1: case 2:
				PRINT_FIELD_U(", ", fev_hdr, vers_v2);
				fev_ver = fev_hdr.vers;
				break;

			default:
				tprints("}");
				pos += offsetof(struct fev_hdr, vers_v2);
				goto end_decoded;
			}
			break;

		case 3: default:
			PRINT_FIELD_U(", ", fev_hdr, vers);
			fev_ver = fev_hdr.vers;
			if (fev_hdr.reserved)
				PRINT_FIELD_U(", ", fev_hdr, reserved);
			PRINT_FIELD_U(", ", fev_hdr, metadata_len);

			if (fev_hdr.metadata_len < FEV_V2_SIZE) {
				tprints(", ... /* invalid metadata_len */}");
				goto end_fev_decoded;
			}
		}

		if (fev_ver < 1 || fev_ver > 3) {
			tprints(", ... /* invalid vers */}");
			goto end_fev_decoded;
		}

		switch (fev_ver) {
		case 1:
			if (umove(tcp, addr + pos + sizeof(fev_hdr),
				  &fev_md.v1)) {
				printf(", ...}");
				pos += sizeof(fev_hdr);
				goto end_decoded;
			}

			PRINT_FIELD_FANFD(", ", fev_md.v1, fd, tcp);
			PRINT_FIELD_FLAGS(", ", fev_md.v1, mask,
					  fan_event_flags, "FAN_???");
			PRINT_FIELD_D(", ", fev_md.v1, pid);

			if (FEV_V1_SIZE < fev_hdr.event_len) {
				tprints(", ");
				printstrn(tcp, addr + pos,
					  fev_hdr.event_len - FEV_V1_SIZE);
			}

			break;

		case 2: case 3:
			if (umove(tcp, addr + pos + sizeof(fev_hdr),
				  &fev_md.v2)) {
				printf(", ...}");
				pos += sizeof(fev_hdr);
				goto end_decoded;
			}

			PRINT_FIELD_FLAGS(", ", fev_md.v2, mask,
					  fan_event_flags, "FAN_???");
			PRINT_FIELD_FANFD(", ", fev_md.v2, fd, tcp);
			PRINT_FIELD_D(", ", fev_md.v2, pid);

			if (FEV_V2_SIZE < fev_hdr.event_len) {
				tprints(", ");
				printstrn(tcp, addr + pos,
					  fev_hdr.event_len - FEV_V2_SIZE);
			}
		}

		tprints("}");

end_fev_decoded:
		pos += fev_hdr.event_len;
	} while (pos <= addrlen - sizeof(fev_hdr));

end_decoded:
	if (pos < addrlen) {
		if (pos)
			tprints(", ");

		printstrn(tcp, addr + pos, addrlen - pos);
	}

	tprints("]");

	return true;
}

bool
decode_fanotify_write(struct tcb *tcp, int fd, const char *fdpath,
		      enum fileops op, kernel_ulong_t addr,
		      kernel_ulong_t addrlen)
{
	struct fresp {
		int32_t fd;
		uint32_t response;
	} fresp;
	kernel_ulong_t pos = 0;

	if (addrlen < sizeof(fresp))
		return false;

	tprints("[");

	do {
		if (pos)
			tprints(", ");

		if (umove(tcp, addr + pos, &fresp)) {
			printaddr_comment(addr + pos);
			break;
		}

		PRINT_FIELD_FD("{", fresp, fd, tcp);

		tprints(", response=");
		if (fresp.response | FAN_AUDIT) {
			print_xlat(FAN_AUDIT);
			tprints("|");
		}
		printxval(fan_responses, fresp.response, "FAN_???");
		tprints("}");

		pos += sizeof(fresp);
	} while (pos <= addrlen - sizeof(fresp));

	if (pos < addrlen) {
		if (pos)
			tprints(", ");

		printstrn(tcp, addr + pos, addrlen - pos);
	}

	tprints("]");

	return true;
}

SYS_FUNC(fanotify_init)
{
	unsigned int flags = tcp->u_arg[0];

	printxval(fan_classes, flags & FAN_ALL_CLASS_BITS, "FAN_CLASS_???");
	flags &= ~FAN_ALL_CLASS_BITS;
	if (flags) {
		tprints("|");
		printflags(fan_init_flags, flags, "FAN_???");
	}
	tprints(", ");
	tprint_open_modes((unsigned) tcp->u_arg[1]);

	return RVAL_DECODED | RVAL_FD;
}

SYS_FUNC(fanotify_mark)
{
	printfd(tcp, tcp->u_arg[0]);
	tprints(", ");
	printflags(fan_mark_flags, tcp->u_arg[1], "FAN_MARK_???");
	tprints(", ");
	/*
	 * the mask argument is defined as 64-bit,
	 * but kernel uses the lower 32 bits only.
	 */
	unsigned long long mask = 0;
	int argn = getllval(tcp, &mask, 2);
#ifdef HPPA
	/* Parsic is weird.  See arch/parisc/kernel/sys_parisc32.c.  */
	mask = (mask << 32) | (mask >> 32);
#endif
	printflags64(fan_event_flags, mask, "FAN_???");
	tprints(", ");
	if ((int) tcp->u_arg[argn] == FAN_NOFD) {
		print_xlat_d(FAN_NOFD);
		tprints(", ");
	} else {
		print_dirfd(tcp, tcp->u_arg[argn]);
	}
	printpath(tcp, tcp->u_arg[argn + 1]);

	return RVAL_DECODED;
}
