/*
 * Copyright (c) 1991, 1992 Paul Kranenburg <pk@cs.few.eur.nl>
 * Copyright (c) 1993 Branko Lankester <branko@hacktic.nl>
 * Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
 * Copyright (c) 1996-1999 Wichert Akkerman <wichert@cistron.nl>
 * Copyright (c) 1999 IBM Deutschland Entwicklung GmbH, IBM Corporation
 *                     Linux for s390 port by D.J. Barrow
 *                    <barrow_dj@mail.yahoo.com,djbarrow@de.ibm.com>
 * Copyright (c) 1999-2020 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "defs.h"
#include <sys/uio.h>

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
process_read_mem(const pid_t pid, void *const laddr,
		 void *const raddr, const size_t len)
{
	const struct iovec local = {
		.iov_base = laddr,
		.iov_len = len
	};
	const struct iovec remote = {
		.iov_base = raddr,
		.iov_len = len
	};

	const ssize_t rc = process_vm_readv(pid, &local, 1, &remote, 1, 0);
	if (rc < 0 && errno == ENOSYS)
		process_vm_readv_not_supported = true;

	return rc;
}

static int cached_idx = -1;
static unsigned long cached_raddr[4];

void
invalidate_umove_cache(void)
{
	cached_idx = -1;
}

static int
get_next_unused_idx(void)
{
	return (cached_idx + 1) % ARRAY_SIZE(cached_raddr);
}

static int
lookup_cached_raddr_idx(const unsigned long raddr)
{
	if (cached_idx >= 0) {
		for (int i = cached_idx; i >= 0; --i)
			if (raddr == cached_raddr[i])
				return i;
		for (int i = (int) ARRAY_SIZE(cached_raddr) - 1;
		     i > cached_idx; --i)
			if (raddr == cached_raddr[i])
				return i;
	}
	return -1;
}

static void
set_cached_raddr_idx(const unsigned long raddr, const int idx)
{
	if (cached_idx < 0)
		memset(cached_raddr, 0, sizeof(cached_raddr));
	cached_raddr[idx] = raddr;
	cached_idx = idx;
}

static ssize_t
vm_read_mem(const pid_t pid, void *laddr,
	    const kernel_ulong_t kraddr, size_t len)
{
	if (!len)
		return len;

	unsigned long taddr = kraddr;

#if SIZEOF_LONG < SIZEOF_KERNEL_LONG_T
	if (kraddr != (kernel_ulong_t) taddr) {
		errno = EIO;
		return -1;
	}
#endif

	const size_t page_size = get_pagesize();
	const size_t page_mask = page_size - 1;
	unsigned long page_start = taddr & ~page_mask;
	const unsigned long page_after_last =
		(taddr + len + page_mask) & ~page_mask;

	if (!page_start ||
	    page_after_last < page_start ||
	    page_after_last - page_start > ARRAY_SIZE(cached_raddr) * page_size)
		return process_read_mem(pid, laddr, (void *) taddr, len);

	size_t total_read = 0;

	for (;;) {
		static char *buf[ARRAY_SIZE(cached_raddr)];
		int idx = lookup_cached_raddr_idx(page_start);

		if (idx == -1) {
			idx = get_next_unused_idx();

			if (!buf[idx])
				buf[idx] = xmalloc(page_size);

			const ssize_t rc =
				process_read_mem(pid, buf[idx],
						 (void *) page_start, page_size);
			if (rc < 0)
				return total_read ? (ssize_t) total_read : rc;

			set_cached_raddr_idx(page_start, idx);
		}

		const unsigned long offset = taddr - page_start;
		size_t copy_len, next_len;

		if (len <= page_size - offset) {
			copy_len = len;
			next_len = 0;
		} else {
			copy_len = page_size - offset;
			next_len = len - copy_len;
		}

		memcpy(laddr, buf[idx] + offset, copy_len);
		total_read += copy_len;

		if (!next_len)
			break;

		len = next_len;
		laddr += copy_len;
		page_start += page_size;
		taddr = page_start;
	}

	return total_read;
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

/* legacy method of copying from tracee */
static int
umoven_peekdata(const int pid, kernel_ulong_t addr, unsigned int len,
		void *laddr)
{
	unsigned int nread = 0;
	unsigned int residue = addr & (sizeof(long) - 1);

	while (len) {
		addr &= -sizeof(long);		/* aligned address */

		errno = 0;
		union {
			long val;
			char x[sizeof(long)];
		} u = { .val = ptrace(PTRACE_PEEKDATA, pid, addr, 0) };

		switch (errno) {
			case 0:
				break;
			case ESRCH: case EINVAL:
				/* these could be seen if the process is gone */
				return -1;
			case EFAULT: case EIO: case EPERM:
				/* address space is inaccessible */
				if (nread) {
					perror_func_msg("short read (%u < %u)"
							" @0x%" PRI_klx,
							nread, nread + len,
							addr - nread);
				}
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_func_msg("pid:%d @0x%" PRI_klx,
						pid, addr);
				return -1;
		}

		unsigned int m = MIN(sizeof(long) - residue, len);
		memcpy(laddr, &u.x[residue], m);
		residue = 0;
		addr += sizeof(long);
		laddr += m;
		nread += m;
		len -= m;
	}

	return 0;
}

/*
 * Copy `len' bytes of data from process `pid'
 * at address `addr' to our space at `our_addr'.
 */
int
umoven(struct tcb *const tcp, kernel_ulong_t addr, unsigned int len,
       void *const our_addr)
{
	if (tracee_addr_is_invalid(addr))
		return -1;

	const int pid = tcp->pid;

	if (process_vm_readv_not_supported)
		return umoven_peekdata(pid, addr, len, our_addr);

	int r = vm_read_mem(pid, our_addr, addr, len);
	if ((unsigned int) r == len)
		return 0;
	if (r >= 0) {
		error_func_msg("short read (%u < %u) @0x%" PRI_klx,
			       (unsigned int) r, len, addr);
		return -1;
	}
	switch (errno) {
		case ENOSYS:
		case EPERM:
			/* try PTRACE_PEEKDATA */
			return umoven_peekdata(pid, addr, len, our_addr);
		case ESRCH:
			/* the process is gone */
			return -1;
		case EFAULT: case EIO:
			/* address space is inaccessible */
			return -1;
		default:
			/* all the rest is strange and should be reported */
			perror_func_msg("pid:%d @0x%" PRI_klx, pid, addr);
			return -1;
	}
}

/*
 * Like umoven_peekdata but make the additional effort of looking
 * for a terminating zero byte.
 */
static int
umovestr_peekdata(const int pid, kernel_ulong_t addr, unsigned int len,
		  void *laddr)
{
	unsigned int nread = 0;
	unsigned int residue = addr & (sizeof(long) - 1);
	void *const orig_addr = laddr;

	while (len) {
		addr &= -sizeof(long);		/* aligned address */

		errno = 0;
		union {
			unsigned long val;
			char x[sizeof(long)];
		} u = { .val = ptrace(PTRACE_PEEKDATA, pid, addr, 0) };

		switch (errno) {
			case 0:
				break;
			case ESRCH: case EINVAL:
				/* these could be seen if the process is gone */
				return -1;
			case EFAULT: case EIO: case EPERM:
				/* address space is inaccessible */
				if (nread) {
					perror_func_msg("short read (%d < %d)"
							" @0x%" PRI_klx,
							nread, nread + len,
							addr - nread);
				}
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_func_msg("pid:%d @0x%" PRI_klx,
						pid, addr);
				return -1;
		}

		unsigned int m = MIN(sizeof(long) - residue, len);
		memcpy(laddr, &u.x[residue], m);
		while (residue < sizeof(long))
			if (u.x[residue++] == '\0')
				return (laddr - orig_addr) + residue;
		residue = 0;
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
 * Returns < 0 on error, strlen + 1  if NUL was seen,
 * else 0 if len bytes were read but no NUL byte seen.
 *
 * Note: there is no guarantee we won't overwrite some bytes
 * in laddr[] _after_ terminating NUL (but, of course,
 * we never write past laddr[len-1]).
 */
int
umovestr(struct tcb *const tcp, kernel_ulong_t addr, unsigned int len,
	 char *laddr)
{
	if (tracee_addr_is_invalid(addr))
		return -1;

	const int pid = tcp->pid;

	if (process_vm_readv_not_supported)
		return umovestr_peekdata(pid, addr, len, laddr);

	const size_t page_size = get_pagesize();
	const size_t page_mask = page_size - 1;
	unsigned int nread = 0;

	while (len) {
		/*
		 * Don't cross pages, otherwise we can get EFAULT
		 * and fail to notice that terminating NUL lies
		 * in the existing (first) page.
		 */
		unsigned int chunk_len = len > page_size ? page_size : len;
		unsigned int end_in_page = (addr + chunk_len) & page_mask;
		if (chunk_len > end_in_page) /* crosses to the next page */
			chunk_len -= end_in_page;

		int r = vm_read_mem(pid, laddr, addr, chunk_len);
		if (r > 0) {
			char *nul_addr = memchr(laddr, '\0', r);

			if (nul_addr)
				return (nul_addr - laddr) + 1;
			addr += r;
			laddr += r;
			nread += r;
			len -= r;
			continue;
		}
		switch (errno) {
			case ENOSYS:
			case EPERM:
				/* try PTRACE_PEEKDATA */
				if (!nread)
					return umovestr_peekdata(pid, addr,
								 len, laddr);
				ATTRIBUTE_FALLTHROUGH;
			case EFAULT: case EIO:
				/* address space is inaccessible */
				if (nread)
					perror_func_msg("short read (%d < %d)"
							" @0x%" PRI_klx,
							nread, nread + len,
							addr - nread);
				return -1;
			case ESRCH:
				/* the process is gone */
				return -1;
			default:
				/* all the rest is strange and should be reported */
				perror_func_msg("pid:%d @0x%" PRI_klx, pid, addr);
				return -1;
		}
	}

	return 0;
}
