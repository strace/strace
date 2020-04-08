/*
 * Check decoding of sigaction syscall.
 *
 * Copyright (c) 2014-2018 Dmitry V. Levin <ldv@altlinux.org>
 * Copyright (c) 2014-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include "scno.h"

#ifdef __NR_sigaction

# include <signal.h>
# include <stdint.h>
# include <stdio.h>
# include <string.h>
# include <unistd.h>

struct set_sa {
# if defined MIPS
	unsigned int flags;
	unsigned long handler;
	unsigned long mask[1];
# elif defined ALPHA
	unsigned long handler;
	unsigned long mask[1];
	unsigned int flags;
# else
	unsigned long handler;
	unsigned long mask[1];
	unsigned long flags;
	unsigned long restorer;
# endif
}
# ifdef ALPHA
	ATTRIBUTE_PACKED
# endif
;

typedef struct set_sa struct_set_sa;

# ifdef MIPS

struct get_sa {
	unsigned int flags;
	unsigned long handler;
	unsigned long mask[4];
};

typedef struct get_sa struct_get_sa;

# else

typedef struct set_sa struct_get_sa;

# endif

static long
k_sigaction(const kernel_ulong_t signum, const kernel_ulong_t new_act,
	    const kernel_ulong_t old_act)
{
	return syscall(__NR_sigaction, signum, new_act, old_act);
}

# if defined SPARC || defined SPARC64
/*
 * See arch/sparc/kernel/sys_sparc_32.c:sys_sparc_sigaction
 * and arch/sparc/kernel/sys_sparc32.c:compat_sys_sparc_sigaction
 */
#  define ADDR_INT ((unsigned int) -0xdefaced)
#  define SIGNO_INT ((unsigned int) -SIGUSR1)
#  define SIG_STR "-SIGUSR1"
# else
#  define ADDR_INT ((unsigned int) 0xdefaced)
#  define SIGNO_INT ((unsigned int) SIGUSR1)
#  define SIG_STR "SIGUSR1"
# endif
static const kernel_ulong_t signo =
	(kernel_ulong_t) 0xbadc0ded00000000ULL | SIGNO_INT;
static const kernel_ulong_t addr =
	(kernel_ulong_t) 0xfacefeed00000000ULL | ADDR_INT;

int
main(void)
{
	union {
		sigset_t libc[1];
		unsigned long old[1];
	} mask;

	TAIL_ALLOC_OBJECT_CONST_PTR(struct_set_sa, new_act);
	TAIL_ALLOC_OBJECT_CONST_PTR(struct_get_sa, old_act);

	if (k_sigaction(signo, 0, 0))
		perror_msg_and_skip("sigaction");
	puts("sigaction(" SIG_STR ", NULL, NULL) = 0");

	k_sigaction(signo, 0, 0);
	puts("sigaction(" SIG_STR ", NULL, NULL) = 0");

	k_sigaction(signo, (uintptr_t) (new_act + 1), 0);
	printf("sigaction(" SIG_STR ", %p, NULL) = -1 EFAULT (%m)\n",
	       new_act + 1);

	k_sigaction(signo, (uintptr_t) new_act + 2, 0);
	printf("sigaction(" SIG_STR ", %#lx, NULL) = -1 EFAULT (%m)\n",
	       (unsigned long) new_act + 2);

	k_sigaction(signo, 0, (uintptr_t) (old_act + 1));
	printf("sigaction(" SIG_STR ", NULL, %p) = -1 EFAULT (%m)\n",
	       old_act + 1);

	k_sigaction(signo, 0, (uintptr_t) old_act + 2);
	printf("sigaction(" SIG_STR ", NULL, %#lx) = -1 EFAULT (%m)\n",
	       (unsigned long) old_act + 2);

	k_sigaction(addr, 0, 0);
	printf("sigaction(%d, NULL, NULL) = -1 EINVAL (%m)\n", ADDR_INT);

	memset(new_act, 0, sizeof(*new_act));

	k_sigaction(signo, (uintptr_t) new_act, 0);
	puts("sigaction(" SIG_STR ", {sa_handler=SIG_DFL, sa_mask=[]"
	     ", sa_flags=0}, NULL) = 0");

	sigemptyset(mask.libc);
	sigaddset(mask.libc, SIGHUP);
	sigaddset(mask.libc, SIGINT);

	new_act->handler = (uintptr_t) SIG_IGN;
	memcpy(new_act->mask, mask.old, sizeof(mask.old));
	new_act->flags = SA_SIGINFO;

	k_sigaction(signo, (uintptr_t) new_act, (uintptr_t) old_act);
	puts("sigaction(" SIG_STR ", {sa_handler=SIG_IGN, sa_mask=[HUP INT]"
	     ", sa_flags=SA_SIGINFO}, {sa_handler=SIG_DFL, sa_mask=[]"
	     ", sa_flags=0}) = 0");

	sigemptyset(mask.libc);
	sigaddset(mask.libc, SIGQUIT);
	sigaddset(mask.libc, SIGTERM);

	new_act->handler = (unsigned long) addr;
	memcpy(new_act->mask, mask.old, sizeof(mask.old));
	new_act->flags = SA_ONSTACK | SA_RESTART;

	k_sigaction(signo, (uintptr_t) new_act, (uintptr_t) old_act);
	printf("sigaction(" SIG_STR ", {sa_handler=%#lx, sa_mask=[QUIT TERM]"
	       ", sa_flags=SA_ONSTACK|SA_RESTART}, {sa_handler=SIG_IGN"
	       ", sa_mask=[HUP INT], sa_flags=SA_SIGINFO}) = 0\n",
	       new_act->handler);

	memset(mask.old, -1, sizeof(mask.old));
	sigdelset(mask.libc, SIGHUP);

	memcpy(new_act->mask, mask.old, sizeof(mask.old));
# if defined SA_RESTORER && !(defined ALPHA || defined MIPS)
	new_act->flags = SA_RESTORER;
	new_act->restorer = (unsigned long) 0xdeadfacecafef00dULL;
#  define SA_RESTORER_FMT ", sa_flags=SA_RESTORER, sa_restorer=%#lx"
#  define SA_RESTORER_ARGS , new_act->restorer
# else
	new_act->flags = SA_NODEFER;
#  define SA_RESTORER_FMT ", sa_flags=SA_NODEFER"
#  define SA_RESTORER_ARGS
# endif

	k_sigaction(signo, (uintptr_t) new_act, (uintptr_t) old_act);
	printf("sigaction(" SIG_STR ", {sa_handler=%#lx, sa_mask=~[HUP]"
	       SA_RESTORER_FMT "}, {sa_handler=%#lx, sa_mask=[QUIT TERM]"
	       ", sa_flags=SA_ONSTACK|SA_RESTART}) = 0\n",
	       new_act->handler SA_RESTORER_ARGS,
	       new_act->handler);

	puts("+++ exited with 0 +++");
	return 0;
}

#else

SKIP_MAIN_UNDEFINED("__NR_sigaction")

#endif
