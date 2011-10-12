/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
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

/* Obsolete syscalls */
#define sys_otime	printargs
#define sys_osetuid	printargs
#define sys_ostime	printargs
#define sys_oalarm	printargs
#define sys_ofstat	printargs
#define sys_opause	printargs
#define sys_outime	printargs
#define sys_onice	printargs
#define sys_oftime	printargs
#define sys_osetpgrp	printargs
#define sys_otimes	printargs
#define sys_osetgid	printargs
#define sys_ossig	printargs
#define sys_owait3	printargs
#define sys_omsync	printargs
#define sys_ovadvise	printargs
#define sys_omadvise	printargs
#define sys_ovlimit	printargs
#define sys_owait	printargs
#define sys_ovtimes	printargs
#define sys_oldquota	printargs
#define sys_getdirentries	printargs

/* No interesting parameters or return values */
#define sys_vhangup	printargs
#define sys_sys_setsid	printargs
#define sys_errsys	printargs
#define sys_nosys	printargs

/* Don't know what to do with these */
#define sys_sstk	printargs
#define sys_profil	printargs
#define sys_vtrace	printargs
#define sys_async_daemon printargs
#define sys_nfs_getfh	printargs
#define sys_rtschedule	printargs
#define sys_auditsys	printargs
#define sys_rfssys	printargs
#define sys_vpixsys	printargs
#define sys_getdopt	printargs
#define sys_setdopt	printargs
#define sys_semsys	printargs
#define sys_msgsys	printargs
#define sys_shmsys	printargs
#define sys_semop	printargs
