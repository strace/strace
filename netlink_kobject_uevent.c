/*
 * Copyright (c) 2018 Harsha Sharma <harshasharmaiitr@gmail.com>
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
#include "netlink_kobject_uevent.h"

#include <arpa/inet.h>

void
decode_netlink_kobject_uevent(struct tcb *tcp, kernel_ulong_t addr,
			      kernel_ulong_t len)
{
	struct udev_monitor_netlink_header uh;
	const char *prefix = "libudev";
	unsigned int offset = sizeof(uh);

	if (!verbose(tcp) || (exiting(tcp) && syserror(tcp)) ||
	    !addr || len < offset || umove(tcp, addr, &uh) ||
	    strcmp(uh.prefix, prefix) != 0) {
		printstrn(tcp, addr, len);
		return;
	}

	PRINT_FIELD_CSTRING("{{", uh, prefix);
	tprintf(", magic=htonl(%#x)", ntohl(uh.magic));
	PRINT_FIELD_U(", ", uh, header_size);
	PRINT_FIELD_U(", ", uh, properties_off);
	PRINT_FIELD_U(", ", uh, properties_len);
	tprintf(", filter_subsystem_hash=htonl(%#x)", ntohl(uh.filter_subsystem_hash));
	tprintf(", filter_devtype_hash=htonl(%#x)", ntohl(uh.filter_devtype_hash));
	tprintf(", filter_tag_bloom_hi=htonl(%#x)", ntohl(uh.filter_tag_bloom_hi));
	tprintf(", filter_tag_bloom_lo=htonl(%#x)", ntohl(uh.filter_tag_bloom_lo));
	tprints("}");
	if (len > offset) {
		tprints(", ");
		printstrn(tcp, addr + offset, len - offset);
	}
	tprints("}");
}
