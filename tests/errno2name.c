/*
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2016-2018 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>

#define CASE(x) case x: return #x

const char *
errno2name(void)
{
	switch (errno) {
	/* names taken from linux/errnoent.h */
#ifdef E2BIG
	CASE(E2BIG);
#endif
#ifdef EACCES
	CASE(EACCES);
#endif
#ifdef EADDRINUSE
	CASE(EADDRINUSE);
#endif
#ifdef EADDRNOTAVAIL
	CASE(EADDRNOTAVAIL);
#endif
#ifdef EADV
	CASE(EADV);
#endif
#ifdef EAFNOSUPPORT
	CASE(EAFNOSUPPORT);
#endif
#ifdef EAGAIN
	CASE(EAGAIN);
#endif
#ifdef EALREADY
	CASE(EALREADY);
#endif
#ifdef EBADCOOKIE
	CASE(EBADCOOKIE);
#endif
#ifdef EBADE
	CASE(EBADE);
#endif
#ifdef EBADF
	CASE(EBADF);
#endif
#ifdef EBADFD
	CASE(EBADFD);
#endif
#ifdef EBADHANDLE
	CASE(EBADHANDLE);
#endif
#ifdef EBADMSG
	CASE(EBADMSG);
#endif
#ifdef EBADR
	CASE(EBADR);
#endif
#ifdef EBADRQC
	CASE(EBADRQC);
#endif
#ifdef EBADSLT
	CASE(EBADSLT);
#endif
#ifdef EBADTYPE
	CASE(EBADTYPE);
#endif
#ifdef EBFONT
	CASE(EBFONT);
#endif
#ifdef EBUSY
	CASE(EBUSY);
#endif
#ifdef ECANCELED
	CASE(ECANCELED);
#endif
#ifdef ECHILD
	CASE(ECHILD);
#endif
#ifdef ECHRNG
	CASE(ECHRNG);
#endif
#ifdef ECOMM
	CASE(ECOMM);
#endif
#ifdef ECONNABORTED
	CASE(ECONNABORTED);
#endif
#ifdef ECONNREFUSED
	CASE(ECONNREFUSED);
#endif
#ifdef ECONNRESET
	CASE(ECONNRESET);
#endif
#ifdef EDEADLK
	CASE(EDEADLK);
#endif
#ifdef EDESTADDRREQ
	CASE(EDESTADDRREQ);
#endif
#ifdef EDOM
	CASE(EDOM);
#endif
#ifdef EDOTDOT
	CASE(EDOTDOT);
#endif
#ifdef EDQUOT
	CASE(EDQUOT);
#endif
#ifdef EEXIST
	CASE(EEXIST);
#endif
#ifdef EFAULT
	CASE(EFAULT);
#endif
#ifdef EFBIG
	CASE(EFBIG);
#endif
#ifdef EHOSTDOWN
	CASE(EHOSTDOWN);
#endif
#ifdef EHOSTUNREACH
	CASE(EHOSTUNREACH);
#endif
#ifdef EHWPOISON
	CASE(EHWPOISON);
#endif
#ifdef EIDRM
	CASE(EIDRM);
#endif
#ifdef EILSEQ
	CASE(EILSEQ);
#endif
#ifdef EINPROGRESS
	CASE(EINPROGRESS);
#endif
#ifdef EINTR
	CASE(EINTR);
#endif
#ifdef EINVAL
	CASE(EINVAL);
#endif
#ifdef EIO
	CASE(EIO);
#endif
#ifdef EIOCBQUEUED
	CASE(EIOCBQUEUED);
#endif
#ifdef EISCONN
	CASE(EISCONN);
#endif
#ifdef EISDIR
	CASE(EISDIR);
#endif
#ifdef EISNAM
	CASE(EISNAM);
#endif
#ifdef EJUKEBOX
	CASE(EJUKEBOX);
#endif
#ifdef EKEYEXPIRED
	CASE(EKEYEXPIRED);
#endif
#ifdef EKEYREJECTED
	CASE(EKEYREJECTED);
#endif
#ifdef EKEYREVOKED
	CASE(EKEYREVOKED);
#endif
#ifdef EL2HLT
	CASE(EL2HLT);
#endif
#ifdef EL2NSYNC
	CASE(EL2NSYNC);
#endif
#ifdef EL3HLT
	CASE(EL3HLT);
#endif
#ifdef EL3RST
	CASE(EL3RST);
#endif
#ifdef ELIBACC
	CASE(ELIBACC);
#endif
#ifdef ELIBBAD
	CASE(ELIBBAD);
#endif
#ifdef ELIBEXEC
	CASE(ELIBEXEC);
#endif
#ifdef ELIBMAX
	CASE(ELIBMAX);
#endif
#ifdef ELIBSCN
	CASE(ELIBSCN);
#endif
#ifdef ELNRNG
	CASE(ELNRNG);
#endif
#ifdef ELOOP
	CASE(ELOOP);
#endif
#ifdef EMEDIUMTYPE
	CASE(EMEDIUMTYPE);
#endif
#ifdef EMFILE
	CASE(EMFILE);
#endif
#ifdef EMLINK
	CASE(EMLINK);
#endif
#ifdef EMSGSIZE
	CASE(EMSGSIZE);
#endif
#ifdef EMULTIHOP
	CASE(EMULTIHOP);
#endif
#ifdef ENAMETOOLONG
	CASE(ENAMETOOLONG);
#endif
#ifdef ENAVAIL
	CASE(ENAVAIL);
#endif
#ifdef ENETDOWN
	CASE(ENETDOWN);
#endif
#ifdef ENETRESET
	CASE(ENETRESET);
#endif
#ifdef ENETUNREACH
	CASE(ENETUNREACH);
#endif
#ifdef ENFILE
	CASE(ENFILE);
#endif
#ifdef ENOANO
	CASE(ENOANO);
#endif
#ifdef ENOBUFS
	CASE(ENOBUFS);
#endif
#ifdef ENOCSI
	CASE(ENOCSI);
#endif
#ifdef ENODATA
	CASE(ENODATA);
#endif
#ifdef ENODEV
	CASE(ENODEV);
#endif
#ifdef ENOENT
	CASE(ENOENT);
#endif
#ifdef ENOEXEC
	CASE(ENOEXEC);
#endif
#ifdef ENOIOCTLCMD
	CASE(ENOIOCTLCMD);
#endif
#ifdef ENOKEY
	CASE(ENOKEY);
#endif
#ifdef ENOLCK
	CASE(ENOLCK);
#endif
#ifdef ENOLINK
	CASE(ENOLINK);
#endif
#ifdef ENOMEDIUM
	CASE(ENOMEDIUM);
#endif
#ifdef ENOMEM
	CASE(ENOMEM);
#endif
#ifdef ENOMSG
	CASE(ENOMSG);
#endif
#ifdef ENONET
	CASE(ENONET);
#endif
#ifdef ENOPKG
	CASE(ENOPKG);
#endif
#ifdef ENOPROTOOPT
	CASE(ENOPROTOOPT);
#endif
#ifdef ENOSPC
	CASE(ENOSPC);
#endif
#ifdef ENOSR
	CASE(ENOSR);
#endif
#ifdef ENOSTR
	CASE(ENOSTR);
#endif
#ifdef ENOSYS
	CASE(ENOSYS);
#endif
#ifdef ENOTBLK
	CASE(ENOTBLK);
#endif
#ifdef ENOTCONN
	CASE(ENOTCONN);
#endif
#ifdef ENOTDIR
	CASE(ENOTDIR);
#endif
#ifdef ENOTEMPTY
	CASE(ENOTEMPTY);
#endif
#ifdef ENOTNAM
	CASE(ENOTNAM);
#endif
#ifdef ENOTRECOVERABLE
	CASE(ENOTRECOVERABLE);
#endif
#ifdef ENOTSOCK
	CASE(ENOTSOCK);
#endif
#ifdef ENOTSUPP
	CASE(ENOTSUPP);
#endif
#ifdef ENOTSYNC
	CASE(ENOTSYNC);
#endif
#ifdef ENOTTY
	CASE(ENOTTY);
#endif
#ifdef ENOTUNIQ
	CASE(ENOTUNIQ);
#endif
#ifdef ENXIO
	CASE(ENXIO);
#endif
#ifdef EOPENSTALE
	CASE(EOPENSTALE);
#endif
#ifdef EOPNOTSUPP
	CASE(EOPNOTSUPP);
#endif
#ifdef EOVERFLOW
	CASE(EOVERFLOW);
#endif
#ifdef EOWNERDEAD
	CASE(EOWNERDEAD);
#endif
#ifdef EPERM
	CASE(EPERM);
#endif
#ifdef EPFNOSUPPORT
	CASE(EPFNOSUPPORT);
#endif
#ifdef EPIPE
	CASE(EPIPE);
#endif
#ifdef EPROBE_DEFER
	CASE(EPROBE_DEFER);
#endif
#ifdef EPROTO
	CASE(EPROTO);
#endif
#ifdef EPROTONOSUPPORT
	CASE(EPROTONOSUPPORT);
#endif
#ifdef EPROTOTYPE
	CASE(EPROTOTYPE);
#endif
#ifdef ERANGE
	CASE(ERANGE);
#endif
#ifdef EREMCHG
	CASE(EREMCHG);
#endif
#ifdef EREMOTE
	CASE(EREMOTE);
#endif
#ifdef EREMOTEIO
	CASE(EREMOTEIO);
#endif
#ifdef ERESTART
	CASE(ERESTART);
#endif
#ifdef ERESTARTNOHAND
	CASE(ERESTARTNOHAND);
#endif
#ifdef ERESTARTNOINTR
	CASE(ERESTARTNOINTR);
#endif
#ifdef ERESTARTSYS
	CASE(ERESTARTSYS);
#endif
#ifdef ERESTART_RESTARTBLOCK
	CASE(ERESTART_RESTARTBLOCK);
#endif
#ifdef ERFKILL
	CASE(ERFKILL);
#endif
#ifdef EROFS
	CASE(EROFS);
#endif
#ifdef ESERVERFAULT
	CASE(ESERVERFAULT);
#endif
#ifdef ESHUTDOWN
	CASE(ESHUTDOWN);
#endif
#ifdef ESOCKTNOSUPPORT
	CASE(ESOCKTNOSUPPORT);
#endif
#ifdef ESPIPE
	CASE(ESPIPE);
#endif
#ifdef ESRCH
	CASE(ESRCH);
#endif
#ifdef ESRMNT
	CASE(ESRMNT);
#endif
#ifdef ESTALE
	CASE(ESTALE);
#endif
#ifdef ESTRPIPE
	CASE(ESTRPIPE);
#endif
#ifdef ETIME
	CASE(ETIME);
#endif
#ifdef ETIMEDOUT
	CASE(ETIMEDOUT);
#endif
#ifdef ETOOMANYREFS
	CASE(ETOOMANYREFS);
#endif
#ifdef ETOOSMALL
	CASE(ETOOSMALL);
#endif
#ifdef ETXTBSY
	CASE(ETXTBSY);
#endif
#ifdef EUCLEAN
	CASE(EUCLEAN);
#endif
#ifdef EUNATCH
	CASE(EUNATCH);
#endif
#ifdef EUSERS
	CASE(EUSERS);
#endif
#ifdef EXDEV
	CASE(EXDEV);
#endif
#ifdef EXFULL
	CASE(EXFULL);
#endif
	default:
		perror_msg_and_fail("unknown errno %d", errno);
	}
}
