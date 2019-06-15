/*
 * Copyright (c) 2017 Intel Corporation
 * Copyright (c) 2019 Paul Chaignon <paul.chaignon@gmail.com>
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

/*
 * open_memstream returns a FILE stream that allows writing to a
 * dynamically growing buffer, that can be either copied to
 * tcp->outf (syscall successful) or dropped (syscall failed)
 */

#include "defs.h"

FILE *
strace_open_memstream(struct tcb *tcp)
{
	FILE *fp = NULL;

#if HAVE_OPEN_MEMSTREAM
	tcp->memfptr = NULL;
	fp = open_memstream(&tcp->memfptr, &tcp->memfloc);
	if (!fp)
		perror_msg_and_die("open_memstream");
	/*
	 * Call to fflush required to update tcp->memfptr,
	 * see open_memstream man page.
	 */
	fflush(fp);

	/* Store the FILE pointer for later restauration. */
	tcp->real_outf = tcp->outf;
	tcp->outf = fp;
#endif

	return fp;
}

void
strace_close_memstream(struct tcb *tcp, bool publish)
{
#if HAVE_OPEN_MEMSTREAM
	if (!tcp->real_outf) {
		debug_msg("memstream already closed");
		return;
	}

	if (fclose(tcp->outf))
		perror_msg("fclose(tcp->outf)");

	tcp->outf = tcp->real_outf;
	tcp->real_outf = NULL;
	if (tcp->memfptr) {
		if (publish)
			fputs_unlocked(tcp->memfptr, tcp->outf);
		else
			debug_msg("syscall output dropped: %s", tcp->memfptr);
		free(tcp->memfptr);
		tcp->memfptr = NULL;
	}
#endif
}
