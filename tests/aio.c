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

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif

#include <inttypes.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/syscall.h>

#if defined __NR_io_setup \
 && defined __NR_io_submit \
 && defined __NR_io_getevents \
 && defined __NR_io_destroy
# include <linux/aio_abi.h>

int
main(void)
{
	static char data0[4096];
	static char data1[8192];

	struct iocb cb[2] = {
		{
			.aio_data = 0x11111111,
			.aio_reqprio = 11,
			.aio_buf = (unsigned long) data0,
			.aio_offset = 0xdefacedfacefeed,
			.aio_nbytes = sizeof(data0)
		},
		{
			.aio_data = 0x22222222,
			.aio_reqprio = 22,
			.aio_buf = (unsigned long) data1,
			.aio_offset = 0xdefacedcafef00d,
			.aio_nbytes = sizeof(data1)
		}
	};

	long cbs[4] = {
		(long) &cb[0], (long) &cb[1],
		0xdeadbeef, 0xbadc0ded
	};

	unsigned long ctx = 0;
	const unsigned int nr = sizeof(cb) / sizeof(*cb);
	const unsigned long lnr = (unsigned long) (0xdeadbeef00000000ULL | nr);

	struct io_event ev[nr];
	struct timespec ts = { .tv_nsec = 123456789 };

	(void) close(0);
	if (open("/dev/zero", O_RDONLY))
		return 77;

	if (syscall(__NR_io_setup, lnr, &ctx))
		return 77;
	printf("io_setup(%u, [%lu]) = 0\n", nr, ctx);

	if (syscall(__NR_io_submit, ctx, nr, cbs) != (long) nr)
		return 77;
	printf("io_submit(%lu, %u, ["
		"{data=%#llx, pread, reqprio=11, filedes=0, "
			"buf=%p, nbytes=%u, offset=%lld}, "
		"{data=%#llx, pread, reqprio=22, filedes=0, "
			"buf=%p, nbytes=%u, offset=%lld}"
		"]) = %u\n",
	       ctx, nr,
	       (unsigned long long) cb[0].aio_data, data0,
	       (unsigned int) sizeof(data0), (long long) cb[0].aio_offset,
	       (unsigned long long) cb[1].aio_data, data1,
	       (unsigned int) sizeof(data1), (long long) cb[1].aio_offset,
	       nr);

	if (syscall(__NR_io_getevents, ctx, nr, nr, ev, &ts)  != (long) nr)
		return 77;
	printf("io_getevents(%lu, %u, %u, ["
		"{data=%#llx, obj=%p, res=%u, res2=0}, "
		"{data=%#llx, obj=%p, res=%u, res2=0}"
		"], {0, 123456789}) = %u\n",
	       ctx, nr, nr,
	       (unsigned long long) cb[0].aio_data, &cb[0],
	       (unsigned int) sizeof(data0),
	       (unsigned long long) cb[1].aio_data, &cb[1],
	       (unsigned int) sizeof(data1),
	       nr);

	if (syscall(__NR_io_destroy, ctx))
		return 77;
	printf("io_destroy(%lu) = 0\n", ctx);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

int
main(void)
{
	return 77;
}

#endif
