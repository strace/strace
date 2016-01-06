/*
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
 * Copyright (c) 2016 Dmitry V. Levin <ldv@altlinux.org>
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

#include "tests.h"

#ifdef HAVE_MQUEUE_H

# include <fcntl.h>
# include <mqueue.h>
# include <stdlib.h>
# include <unistd.h>
# include <sys/stat.h>

int
main (void)
{
	struct mq_attr attr;
	(void) close(0);
	if (mq_open("/strace-mq.test", O_CREAT, S_IRWXU, 0))
		perror_msg_and_skip("mq_open");
	if (mq_getattr(0, &attr))
		perror_msg_and_skip("mq_getattr");
	if (mq_setattr(0, &attr, 0))
		perror_msg_and_skip("mq_setattr");
	if (mq_unlink("/strace-mq.test"))
		perror_msg_and_skip("mq_unlink");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("HAVE_MQUEUE_H")

#endif
