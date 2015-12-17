/*
 * Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
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

#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>

#include "kernel_types.h"

#define text_string "STRACE_STRING"
#define msgsz sizeof(text_string)

int
main (void)
{
	const long mtype = 0xdefaced;
	struct {
		kernel_long_t mtype;
		char mtext[msgsz];
	} msg = {
		.mtype = mtype,
		.mtext = text_string
	};
	int msqid = msgget(IPC_PRIVATE, IPC_CREAT | S_IRWXU);
	if (msqid == -1)
		return 77;
	if (msgsnd(msqid, &msg, msgsz, 0) == -1)
		goto cleanup;
	if (msgrcv(msqid, &msg, msgsz, mtype, 0) != msgsz)
		goto cleanup;
	if (msgctl(msqid, IPC_RMID, 0) == -1)
		return 77;
	return 0;

cleanup:
	msgctl(msqid, IPC_RMID, 0);
	return 77;
}
