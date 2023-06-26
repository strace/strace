/*
 * Copyright (c) 2017 Intel Corporation
 * Copyright (c) 2019 Paul Chaignon <paul.chaignon@gmail.com>
 * Copyright (c) 2019-2023 The strace developers.
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

struct staged_output_data {
	char *memfptr;
	size_t memfloc;
	FILE *real_outf;	/* Backup for real outf while staging */
};

FILE *
strace_open_memstream(struct tcb *tcp)
{
	FILE *fp = NULL;

#ifdef HAVE_OPEN_MEMSTREAM
	tcp->staged_output_data = xmalloc(sizeof(*tcp->staged_output_data));
	fp = open_memstream(&tcp->staged_output_data->memfptr,
			    &tcp->staged_output_data->memfloc);
	if (!fp)
		perror_msg_and_die("open_memstream");
	/*
	 * Call to fflush required to update tcp->memfptr,
	 * see open_memstream man page.
	 */
	fflush(fp);

	/* Store the FILE pointer for later restoration. */
	tcp->staged_output_data->real_outf = tcp->outf;
	tcp->outf = fp;
#endif

	return fp;
}

void
strace_close_memstream(struct tcb *tcp, bool publish)
{
#ifdef HAVE_OPEN_MEMSTREAM
	if (!tcp->staged_output_data) {
		debug_msg("memstream already closed");
		return;
	}

	if (fclose(tcp->outf))
		perror_msg("fclose(tcp->outf)");

	tcp->outf = tcp->staged_output_data->real_outf;
	if (tcp->staged_output_data->memfptr) {
		if (publish)
			fputs_unlocked(tcp->staged_output_data->memfptr,
				       tcp->outf);
		else
			debug_msg("syscall output dropped: %s",
				  tcp->staged_output_data->memfptr);

		free(tcp->staged_output_data->memfptr);
		tcp->staged_output_data->memfptr = NULL;
	}
	free(tcp->staged_output_data);
	tcp->staged_output_data = NULL;
#endif
}
