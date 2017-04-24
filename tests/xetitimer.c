/*
 * Check decoding of setitimer and getitimer syscalls.
 *
 * Copyright (c) 2015-2017 Dmitry V. Levin <ldv@altlinux.org>
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
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <unistd.h>
#include <asm/unistd.h>

int
main(void)
{
	static const struct itimerval new = {
		.it_interval = { 0xc0de1, 0xc0de2 },
		.it_value = { 0xc0de3, 0xc0de4 }
	};
	static const kernel_ulong_t long_timer =
		F8ILL_KULONG_MASK | ITIMER_REAL;
	static const kernel_ulong_t bogus_timer =
		(kernel_ulong_t) 0xfacefeeddeadbeefULL;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct itimerval, p_old);
	struct itimerval *const p_new = tail_memdup(&new, sizeof(new));
	void *const efault = tail_alloc(sizeof(new) - 8);
	long rc;

	if (setitimer(ITIMER_REAL, p_new, NULL))
		perror_msg_and_skip("setitimer");
	printf("setitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}"
	       ", NULL) = 0\n",
	       (long long) new.it_interval.tv_sec,
	       zero_extend_signed_to_ull(new.it_interval.tv_usec),
	       (long long) new.it_value.tv_sec,
	       zero_extend_signed_to_ull(new.it_value.tv_usec));

	fill_memory(p_old, sizeof(*p_old));
	if (getitimer(ITIMER_REAL, p_old))
		perror_msg_and_skip("getitimer");
	printf("getitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}) = 0\n",
	       (long long) p_old->it_interval.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_interval.tv_usec),
	       (long long) p_old->it_value.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_value.tv_usec));

	fill_memory(p_old, sizeof(*p_old));
	setitimer(ITIMER_REAL, p_new, p_old);
	printf("setitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}) = 0\n",
	       (long long) new.it_interval.tv_sec,
	       zero_extend_signed_to_ull(new.it_interval.tv_usec),
	       (long long) new.it_value.tv_sec,
	       zero_extend_signed_to_ull(new.it_value.tv_usec),
	       (long long) p_old->it_interval.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_interval.tv_usec),
	       (long long) p_old->it_value.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_value.tv_usec));

	rc = getitimer(ITIMER_REAL, efault);
	printf("getitimer(ITIMER_REAL, %p) = %s\n", efault, sprintrc(rc));

	rc = setitimer(ITIMER_REAL, p_new, efault);
	printf("setitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}, %p) = %s\n",
	       (long long) new.it_interval.tv_sec,
	       zero_extend_signed_to_ull(new.it_interval.tv_usec),
	       (long long) new.it_value.tv_sec,
	       zero_extend_signed_to_ull(new.it_value.tv_usec),
	       efault, sprintrc(rc));

	rc = setitimer(ITIMER_REAL, efault, p_old);
	printf("setitimer(ITIMER_REAL, %p, %p) = %s\n",
	       efault, p_old, sprintrc(rc));

	fill_memory(p_old, sizeof(*p_old));
	rc = syscall(__NR_setitimer, long_timer, p_new, p_old);
	printf("setitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}) = %s\n",
	       (long long) new.it_interval.tv_sec,
	       zero_extend_signed_to_ull(new.it_interval.tv_usec),
	       (long long) new.it_value.tv_sec,
	       zero_extend_signed_to_ull(new.it_value.tv_usec),
	       (long long) p_old->it_interval.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_interval.tv_usec),
	       (long long) p_old->it_value.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_value.tv_usec),
	       sprintrc(rc));

	fill_memory(p_old, sizeof(*p_old));
	rc = syscall(__NR_getitimer, long_timer, p_old);
	printf("getitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}) = %s\n",
	       (long long) p_old->it_interval.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_interval.tv_usec),
	       (long long) p_old->it_value.tv_sec,
	       zero_extend_signed_to_ull(p_old->it_value.tv_usec),
	       sprintrc(rc));

	rc = syscall(__NR_setitimer, bogus_timer, p_new, p_old);
	printf("setitimer(%#x /* ITIMER_??? */"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}, %p) = %s\n",
	       (int) bogus_timer,
	       (long long) new.it_interval.tv_sec,
	       zero_extend_signed_to_ull(new.it_interval.tv_usec),
	       (long long) new.it_value.tv_sec,
	       zero_extend_signed_to_ull(new.it_value.tv_usec),
	       p_old, sprintrc(rc));

	rc = syscall(__NR_getitimer, bogus_timer, p_old);
	printf("getitimer(%#x /* ITIMER_??? */, %p) = %s\n",
	       (int) bogus_timer, p_old, sprintrc(rc));

	if (F8ILL_KULONG_SUPPORTED) {
		const kernel_ulong_t ill_new = f8ill_ptr_to_kulong(p_new);
		const kernel_ulong_t ill_old = f8ill_ptr_to_kulong(p_old);

		rc = syscall(__NR_setitimer, long_timer, ill_new, ill_old);
		printf("setitimer(ITIMER_REAL, %#llx, %#llx) = %s\n",
		       (unsigned long long) ill_new,
		       (unsigned long long) ill_old,
		       sprintrc(rc));

		rc = syscall(__NR_getitimer, long_timer, ill_old);
		printf("getitimer(ITIMER_REAL, %#llx) = %s\n",
		       (unsigned long long) ill_old, sprintrc(rc));
	}

	p_new->it_interval.tv_sec = 0xdeadbeefU;
	p_new->it_interval.tv_usec = 0xfacefeedU;
	p_new->it_value.tv_sec = (time_t) 0xcafef00ddeadbeefLL;
	p_new->it_value.tv_usec = (long) 0xbadc0dedfacefeedLL;

	rc = setitimer(ITIMER_REAL, p_new, p_old);
	printf("setitimer(ITIMER_REAL"
	       ", {it_interval={tv_sec=%lld, tv_usec=%llu}"
	       ", it_value={tv_sec=%lld, tv_usec=%llu}}, %p) = %s\n",
	       (long long) p_new->it_interval.tv_sec,
	       zero_extend_signed_to_ull(p_new->it_interval.tv_usec),
	       (long long) p_new->it_value.tv_sec,
	       zero_extend_signed_to_ull(p_new->it_value.tv_usec),
	       p_old, sprintrc(rc));

	puts("+++ exited with 0 +++");
	return 0;
}
