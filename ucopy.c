/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 1999-2017 The strace developers.
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
#include <sys/uio.h>
#include <asm/unistd.h>

#include "scno.h"
#include "ptrace.h"

static bool process_vm_readv_not_supported;

#ifndef HAVE_PROCESS_VM_READV
/*
 * Need to do this since process_vm_readv() is not yet available in libc.
 * When libc is updated, only "static bool process_vm_readv_not_supported"
 * line remains.
 * The name is different to avoid potential collision with OS headers.
 */
static ssize_t strace_process_vm_readv(pid_t pid,
		 const struct iovec *lvec,
		 unsigned long liovcnt,
		 const struct iovec *rvec,
		 unsigned long riovcnt,
		 unsigned long flags)
{
	return syscall(__NR_process_vm_readv,
		       (long) pid, lvec, liovcnt, rvec, riovcnt, flags);
}
# define process_vm_readv strace_process_vm_readv
#endif /* !HAVE_PROCESS_VM_READV */

static ssize_t
vm_read_mem(const pid_t pid, void *const laddr,
	    const kernel_ulong_t raddr, const size_t len)
{
	const unsigned long truncated_raddr = raddr;

#if SIZEOF_LONG < SIZEOF_KERNEL_LONG_T
	if (raddr != (kernel_ulong_t) truncated_raddr) {
		errno = EIO;
		return -1;
	}
#endif

	const struct iovec local = {
		.iov_base = laddr,
		.iov_len = len
	};
	const struct iovec remote = {
		.iov_base = (void *) truncated_raddr,
		.iov_len = len
	};

	return process_vm_readv(pid, &local, 1, &remote, 1, 0);
}

static bool
tracee_addr_is_invalid(kernel_ulong_t addr)
{
	return
#if ANY_WORDSIZE_LESS_THAN_KERNEL_LONG
		current_wordsize < sizeof(addr) && addr & ~(kernel_ulong_t) -1U;
#else
		false;
#endif
}

/*
 * Copy `len' bytes of data from process `pid'
 * at address `addr' to our space at `our_addr'.
 */
int
umoven(struct tcb *const tcp, kernel_ulong_t addr, unsigned int len,
       void *const our_addr)
{
	char *laddr = our_addr;
	int pid = tcp->pid;
	unsigned int n, m, nread;
	union {
		long val;
		char x[sizeof(long)];
	} u;

	if (tracee_addr_is_invalid(addr))
		return -1;

	if (!process_vm_readv_not_supported) {
		int r = vm_read_mem(pid, laddr, addr, len);
		if ((unsigned int) r == len)
			return 0;
		if (r >= 0) {
			error_msg("umoven: short read (%u < %u) @0x%" PRI_klx,
				  (unsigned int) r, len, addr);
			return -1;
		}
		switch (errno) {
			case ENOSYS:
				/* never try it again */
				process_vm_readv_not_supported = 1;
				break;
			case EPERM:
				/* operation not permitted, try PTRACE_PEEKDATA */
				break;
			case ESRCH:
				/* the process is gone */
				return -1;
			case EFAULT: case EIO:
				/* address space is inaccessible */
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_msg("process_vm_readv: pid:%d @0x%" PRI_klx,
					    pid, addr);
				return -1;
		}
	}

	nread = 0;
	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr & (sizeof(long) - 1);	/* residue */
		addr &= -sizeof(long);		/* aligned address */
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, addr, 0);
		switch (errno) {
			case 0:
				break;
			case ESRCH: case EINVAL:
				/* these could be seen if the process is gone */
				return -1;
			case EFAULT: case EIO: case EPERM:
				/* address space is inaccessible */
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_msg("umoven: PTRACE_PEEKDATA pid:%d @0x%" PRI_klx,
					    pid, addr);
				return -1;
		}
		m = MIN(sizeof(long) - n, len);
		memcpy(laddr, &u.x[n], m);
		addr += sizeof(long);
		laddr += m;
		nread += m;
		len -= m;
	}
	while (len) {
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, addr, 0);
		switch (errno) {
			case 0:
				break;
			case ESRCH: case EINVAL:
				/* these could be seen if the process is gone */
				return -1;
			case EFAULT: case EIO: case EPERM:
				/* address space is inaccessible */
				if (nread) {
					perror_msg("umoven: short read (%u < %u) @0x%" PRI_klx,
						   nread, nread + len, addr - nread);
				}
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_msg("umoven: PTRACE_PEEKDATA pid:%d @0x%" PRI_klx,
					    pid, addr);
				return -1;
		}
		m = MIN(sizeof(long), len);
		memcpy(laddr, u.x, m);
		addr += sizeof(long);
		laddr += m;
		nread += m;
		len -= m;
	}

	return 0;
}

/*
 * Like `umove' but make the additional effort of looking
 * for a terminating zero byte.
 *
 * Returns < 0 on error, > 0 if NUL was seen,
 * (TODO if useful: return count of bytes including NUL),
 * else 0 if len bytes were read but no NUL byte seen.
 *
 * Note: there is no guarantee we won't overwrite some bytes
 * in laddr[] _after_ terminating NUL (but, of course,
 * we never write past laddr[len-1]).
 */
int
umovestr(struct tcb *const tcp, kernel_ulong_t addr, unsigned int len, char *laddr)
{
	const unsigned long x01010101 = (unsigned long) 0x0101010101010101ULL;
	const unsigned long x80808080 = (unsigned long) 0x8080808080808080ULL;

	int pid = tcp->pid;
	unsigned int n, m, nread;
	union {
		unsigned long val;
		char x[sizeof(long)];
	} u;

	if (tracee_addr_is_invalid(addr))
		return -1;

	nread = 0;
	if (!process_vm_readv_not_supported) {
		const size_t page_size = get_pagesize();
		const size_t page_mask = page_size - 1;

		while (len > 0) {
			unsigned int chunk_len;
			unsigned int end_in_page;

			/*
			 * Don't cross pages, otherwise we can get EFAULT
			 * and fail to notice that terminating NUL lies
			 * in the existing (first) page.
			 */
			chunk_len = len > page_size ? page_size : len;
			end_in_page = (addr + chunk_len) & page_mask;
			if (chunk_len > end_in_page) /* crosses to the next page */
				chunk_len -= end_in_page;

			int r = vm_read_mem(pid, laddr, addr, chunk_len);
			if (r > 0) {
				if (memchr(laddr, '\0', r))
					return 1;
				addr += r;
				laddr += r;
				nread += r;
				len -= r;
				continue;
			}
			switch (errno) {
				case ENOSYS:
					/* never try it again */
					process_vm_readv_not_supported = 1;
					goto vm_readv_didnt_work;
				case ESRCH:
					/* the process is gone */
					return -1;
				case EPERM:
					/* operation not permitted, try PTRACE_PEEKDATA */
					if (!nread)
						goto vm_readv_didnt_work;
					/* fall through */
				case EFAULT: case EIO:
					/* address space is inaccessible */
					if (nread) {
						perror_msg("umovestr: short read (%d < %d) @0x%" PRI_klx,
							   nread, nread + len, addr - nread);
					}
					return -1;
				default:
					/* all the rest is strange and should be reported */
					perror_msg("process_vm_readv: pid:%d @0x%" PRI_klx,
						    pid, addr);
					return -1;
			}
		}
		return 0;
	}
 vm_readv_didnt_work:

	if (addr & (sizeof(long) - 1)) {
		/* addr not a multiple of sizeof(long) */
		n = addr & (sizeof(long) - 1);	/* residue */
		addr &= -sizeof(long);		/* aligned address */
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, addr, 0);
		switch (errno) {
			case 0:
				break;
			case ESRCH: case EINVAL:
				/* these could be seen if the process is gone */
				return -1;
			case EFAULT: case EIO: case EPERM:
				/* address space is inaccessible */
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_msg("umovestr: PTRACE_PEEKDATA pid:%d @0x%" PRI_klx,
					    pid, addr);
				return -1;
		}
		m = MIN(sizeof(long) - n, len);
		memcpy(laddr, &u.x[n], m);
		while (n & (sizeof(long) - 1))
			if (u.x[n++] == '\0')
				return 1;
		addr += sizeof(long);
		laddr += m;
		nread += m;
		len -= m;
	}

	while (len) {
		errno = 0;
		u.val = ptrace(PTRACE_PEEKDATA, pid, addr, 0);
		switch (errno) {
			case 0:
				break;
			case ESRCH: case EINVAL:
				/* these could be seen if the process is gone */
				return -1;
			case EFAULT: case EIO: case EPERM:
				/* address space is inaccessible */
				if (nread) {
					perror_msg("umovestr: short read (%d < %d) @0x%" PRI_klx,
						   nread, nread + len, addr - nread);
				}
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_msg("umovestr: PTRACE_PEEKDATA pid:%d @0x%" PRI_klx,
					   pid, addr);
				return -1;
		}
		m = MIN(sizeof(long), len);
		memcpy(laddr, u.x, m);
		/* "If a NUL char exists in this word" */
		if ((u.val - x01010101) & ~u.val & x80808080)
			return 1;
		addr += sizeof(long);
		laddr += m;
		nread += m;
		len -= m;
	}
	return 0;
}
