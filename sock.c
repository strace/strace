/*
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
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
 *
 *	$Id$
 */

#include "defs.h"

#ifdef LINUX
#include <sys/socket.h>
#include <linux/sockios.h>
#else
#include <sys/sockio.h>
#endif
#include <arpa/inet.h>

#if defined (ALPHA) || defined(SH) || defined(SH64)
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#elif defined(HAVE_IOCTLS_H)
#include <ioctls.h>
#endif
#endif
#include <net/if.h>

extern struct xlat addrfams[];

int
sock_ioctl(tcp, code, arg)
struct tcb *tcp;
long code, arg;
{
	struct ifreq ifr;
	struct ifconf ifc;

	if (entering(tcp)) {
		if (code == SIOCGIFCONF) {
			umove(tcp, tcp->u_arg[2], &ifc);
			if (ifc.ifc_buf == NULL)
				tprintf(", {%d -> ", ifc.ifc_len);
			else
				tprintf(", {");
		}
		return 0;
	}

	switch (code) {
#ifdef SIOCSHIWAT
	case SIOCSHIWAT:
#endif
#ifdef SIOCGHIWAT
	case SIOCGHIWAT:
#endif
#ifdef SIOCSLOWAT
	case SIOCSLOWAT:
#endif
#ifdef SIOCGLOWAT
	case SIOCGLOWAT:
#endif
#ifdef FIOSETOWN
	case FIOSETOWN:
#endif
#ifdef FIOGETOWN
	case FIOGETOWN:
#endif
#ifdef SIOCSPGRP
	case SIOCSPGRP:
#endif
#ifdef SIOCGPGRP
	case SIOCGPGRP:
#endif
#ifdef SIOCATMARK
	case SIOCATMARK:
#endif
		printnum(tcp, arg, ", %#d");
		return 1;
#ifdef LINUX
	case SIOCGIFNAME:
	case SIOCGIFINDEX:
		umove(tcp, tcp->u_arg[2], &ifr);
                if (syserror(tcp)) {
			if (code == SIOCGIFNAME)
				tprintf(", {%d, ???}", ifr.ifr_ifindex);
			else if (code == SIOCGIFINDEX)
				tprintf(", {???, \"%s\"}", ifr.ifr_name);
		} else
			tprintf(", {%d, \"%s\"}",
				ifr.ifr_ifindex, ifr.ifr_name);
		return 1;
	case SIOCGIFCONF:
		umove(tcp, tcp->u_arg[2], &ifc);
		tprintf("%d, ", ifc.ifc_len);
                if (syserror(tcp)) {
			tprintf("%lx", (unsigned long) ifc.ifc_buf);
		} else if (ifc.ifc_buf == NULL) {
			tprintf("NULL");
		} else {
			int i;
			unsigned nifra = ifc.ifc_len / sizeof(struct ifreq);
			struct ifreq ifra[nifra];
			umoven(tcp, (unsigned long) ifc.ifc_buf, sizeof(ifra),
			       (char *) ifra);
			tprintf("{");
			for (i = 0; i < nifra; ++i ) {
				if (i > 0)
					tprintf(", ");
				tprintf("{\"%s\", {",
					ifra[i].ifr_name);
				if (verbose(tcp)) {
					printxval(addrfams,
						  ifra[i].ifr_addr.sa_family,
						  "AF_???");
					tprintf(", ");
					if (ifra[i].ifr_addr.sa_family == AF_INET) {
						struct sockaddr_in *sinp;
						sinp = (struct sockaddr_in *) &ifra[i].ifr_addr;
						tprintf("inet_addr(\"%s\")",
							inet_ntoa(sinp->sin_addr));
					} else
						printstr(tcp,
							 (long) &ifra[i].ifr_addr.sa_data,
							 sizeof(ifra[i].ifr_addr.sa_data));
				} else
					tprintf("...");
				tprintf("}}");
			}
			tprintf("}");
		}
		tprintf("}");
		return 1;
#endif
	default:
		return 0;
	}
}
