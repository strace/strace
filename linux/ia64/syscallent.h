/*
 * Copyright (c) 1999, 2001 Hewlett-Packard Co
 *	David Mosberger-Tang <davidm@hpl.hp.com>
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
 *
 *	$Id$
 */

/*
 * IA-32 syscalls that have pointer arguments which are incompatible
 * with 64-bit layout get redirected to printargs.
 */
#define sys_getrlimit		printargs
#define sys_afs_syscall		printargs
#define sys_getpmsg		printargs
#define sys_putpmsg		printargs
#define sys_ugetrlimit		printargs
#define sys_waitpid		printargs
#define sys_time		printargs
#define sys_break		printargs
#define sys_oldstat		printargs
#define sys_lseek		printargs
#define sys_stime		printargs
#define sys_ptrace		printargs
#define sys_oldfstat		printargs
#define sys_pause		printargs
#define sys_utime		printargs
#define sys_stty		printargs
#define sys_gtty		printargs
#define sys_ftime		printargs
#define sys_pipe		printargs
#define sys_times		printargs
#define sys_prof		printargs
#define sys_signal		printargs
#define sys_lock		printargs
#define sys_ioctl		printargs
#define sys_fcntl		printargs
#define sys_mpx			printargs
#define sys_ulimit		printargs
#define sys_oldolduname		printargs
#define sys_sigaction		printargs
#define sys_siggetmask		printargs
#define sys_sigsetmask		printargs
#define sys_sigsuspend		printargs
#define sys_sigpending		printargs
#define sys_setrlimit		printargs
#define sys_getrusage		printargs
#define sys_gettimeofday	printargs
#define sys_settimeofday	printargs
#define sys_getgroups		printargs
#define sys_setgroups		printargs
#define sys_select		printargs
#define sys_oldlstat		printargs
#define sys_readdir		printargs
#define sys_profil		printargs
#define sys_statfs		printargs
#define sys_fstatfs		printargs
#define sys_ioperm		printargs
#define sys_setitimer		printargs
#define sys_getitimer		printargs
#define sys_stat		printargs
#define sys_lstat		printargs
#define sys_fstat		printargs
#define sys_olduname		printargs
#define sys_iopl		printargs
#define sys_idle		printargs
#define sys_vm86old		printargs
#define sys_wait4		printargs
#define sys_sysinfo		printargs
#define sys_sigreturn		printargs
#define sys_uname		printargs
#define sys_modify_ldt		printargs
#define sys_adjtimex		printargs
#define sys_sigprocmask		printargs
#define sys_create_module	printargs
#define sys_init_module		printargs
#define sys_get_kernel_syms	printargs
#define sys_quotactl		printargs
#define sys_bdflush		printargs
#define sys_personality		printargs
#define sys_getdents		printargs
#define sys__newselect		printargs
#define sys_msync		printargs
#define sys_readv		printargs
#define sys_writev		printargs
#define sys__sysctl		printargs
#define sys_sched_rr_get_interval printargs
#define sys_getresuid		printargs
#define sys_vm86		printargs
#define sys_query_module	printargs
#define sys_nfsservctl		printargs
#define sys_rt_sigreturn	printargs
#define sys_rt_sigaction	printargs
#define sys_rt_sigprocmask	printargs
#define sys_rt_sigtimedwait	printargs
#define sys_rt_sigqueueinfo	printargs
#define sys_rt_sigsuspend	printargs
#define sys_pread		printargs
#define sys_pwrite		printargs
#define sys_sigaltstack		printargs
#define sys_sendfile		printargs
#define sys_truncate64		printargs
#define sys_ftruncate64		printargs
#define sys_stat64		printargs
#define sys_lstat64		printargs
#define sys_fstat64		printargs
#define sys_fcntl64		printargs

#include "i386/syscallent.h"

#undef sys_getrlimit
#undef sys_afs_syscall
#undef sys_getpmsg
#undef sys_putpmsg
#undef sys_ugetrlimit
#undef sys_waitpid
#undef sys_time
#undef sys_break
#undef sys_oldstat
#undef sys_lseek
#undef sys_stime
#undef sys_ptrace
#undef sys_oldfstat
#undef sys_pause
#undef sys_utime
#undef sys_stty
#undef sys_gtty
#undef sys_ftime
#undef sys_pipe
#undef sys_times
#undef sys_prof
#undef sys_signal
#undef sys_lock
#undef sys_ioctl
#undef sys_fcntl
#undef sys_mpx
#undef sys_ulimit
#undef sys_oldolduname
#undef sys_sigaction
#undef sys_siggetmask
#undef sys_sigsetmask
#undef sys_sigsuspend
#undef sys_sigpending
#undef sys_setrlimit
#undef sys_getrusage
#undef sys_gettimeofday
#undef sys_settimeofday
#undef sys_getgroups
#undef sys_setgroups
#undef sys_select
#undef sys_oldlstat
#undef sys_readdir
#undef sys_profil
#undef sys_statfs
#undef sys_fstatfs
#undef sys_ioperm
#undef sys_setitimer
#undef sys_getitimer
#undef sys_stat
#undef sys_lstat
#undef sys_fstat
#undef sys_olduname
#undef sys_iopl
#undef sys_idle
#undef sys_vm86old
#undef sys_wait4
#undef sys_sysinfo
#undef sys_sigreturn
#undef sys_uname
#undef sys_modify_ldt
#undef sys_adjtimex
#undef sys_sigprocmask
#undef sys_create_module
#undef sys_init_module
#undef sys_get_kernel_syms
#undef sys_quotactl
#undef sys_bdflush
#undef sys_personality
#undef sys_getdents
#undef sys__newselect
#undef sys_msync
#undef sys_readv
#undef sys_writev
#undef sys__sysctl
#undef sys_sched_rr_get_interval
#undef sys_getresuid
#undef sys_vm86
#undef sys_query_module
#undef sys_nfsservctl
#undef sys_rt_sigreturn
#undef sys_rt_sigaction
#undef sys_rt_sigprocmask
#undef sys_rt_sigtimedwait
#undef sys_rt_sigqueueinfo
#undef sys_rt_sigsuspend
#undef sys_pread
#undef sys_pwrite
#undef sys_sigaltstack
#undef sys_sendfile
#undef sys_truncate64
#undef sys_ftruncate64
#undef sys_stat64
#undef sys_lstat64
#undef sys_fstat64
#undef sys_fcntl64

#include "../dummy.h"

/* You must be careful to check ../i386/syscallent.h so that this table
   starts where that one leaves off.
*/
#if SYS_ipc_subcall + SYS_ipc_nsubcalls != 445
# error fix me
#endif

	{ MA,	0,	printargs,		"SYS_445"	}, /* 445 */
	{ MA,	0,	printargs,		"SYS_446"	}, /* 446 */
	{ MA,	0,	printargs,		"SYS_447"	}, /* 447 */
	{ MA,	0,	printargs,		"SYS_448"	}, /* 448 */
	{ MA,	0,	printargs,		"SYS_449"	}, /* 449 */
	{ MA,	0,	printargs,		"SYS_450"	}, /* 450 */
	{ MA,	0,	printargs,		"SYS_451"	}, /* 451 */
	{ MA,	0,	printargs,		"SYS_452"	}, /* 452 */
	{ MA,	0,	printargs,		"SYS_453"	}, /* 453 */
	{ MA,	0,	printargs,		"SYS_454"	}, /* 454 */
	{ MA,	0,	printargs,		"SYS_455"	}, /* 455 */
	{ MA,	0,	printargs,		"SYS_456"	}, /* 456 */
	{ MA,	0,	printargs,		"SYS_457"	}, /* 457 */
	{ MA,	0,	printargs,		"SYS_458"	}, /* 458 */
	{ MA,	0,	printargs,		"SYS_459"	}, /* 459 */
	{ MA,	0,	printargs,		"SYS_460"	}, /* 460 */
	{ MA,	0,	printargs,		"SYS_461"	}, /* 461 */
	{ MA,	0,	printargs,		"SYS_462"	}, /* 462 */
	{ MA,	0,	printargs,		"SYS_463"	}, /* 463 */
	{ MA,	0,	printargs,		"SYS_464"	}, /* 464 */
	{ MA,	0,	printargs,		"SYS_465"	}, /* 465 */
	{ MA,	0,	printargs,		"SYS_466"	}, /* 466 */
	{ MA,	0,	printargs,		"SYS_467"	}, /* 467 */
	{ MA,	0,	printargs,		"SYS_468"	}, /* 468 */
	{ MA,	0,	printargs,		"SYS_469"	}, /* 469 */
	{ MA,	0,	printargs,		"SYS_470"	}, /* 470 */
	{ MA,	0,	printargs,		"SYS_471"	}, /* 471 */
	{ MA,	0,	printargs,		"SYS_472"	}, /* 472 */
	{ MA,	0,	printargs,		"SYS_473"	}, /* 473 */
	{ MA,	0,	printargs,		"SYS_474"	}, /* 474 */
	{ MA,	0,	printargs,		"SYS_475"	}, /* 475 */
	{ MA,	0,	printargs,		"SYS_476"	}, /* 476 */
	{ MA,	0,	printargs,		"SYS_477"	}, /* 477 */
	{ MA,	0,	printargs,		"SYS_478"	}, /* 478 */
	{ MA,	0,	printargs,		"SYS_479"	}, /* 479 */
	{ MA,	0,	printargs,		"SYS_480"	}, /* 480 */
	{ MA,	0,	printargs,		"SYS_481"	}, /* 481 */
	{ MA,	0,	printargs,		"SYS_482"	}, /* 482 */
	{ MA,	0,	printargs,		"SYS_483"	}, /* 483 */
	{ MA,	0,	printargs,		"SYS_484"	}, /* 484 */
	{ MA,	0,	printargs,		"SYS_485"	}, /* 485 */
	{ MA,	0,	printargs,		"SYS_486"	}, /* 486 */
	{ MA,	0,	printargs,		"SYS_487"	}, /* 487 */
	{ MA,	0,	printargs,		"SYS_488"	}, /* 488 */
	{ MA,	0,	printargs,		"SYS_489"	}, /* 489 */
	{ MA,	0,	printargs,		"SYS_490"	}, /* 490 */
	{ MA,	0,	printargs,		"SYS_491"	}, /* 491 */
	{ MA,	0,	printargs,		"SYS_492"	}, /* 492 */
	{ MA,	0,	printargs,		"SYS_493"	}, /* 493 */
	{ MA,	0,	printargs,		"SYS_494"	}, /* 494 */
	{ MA,	0,	printargs,		"SYS_495"	}, /* 495 */
	{ MA,	0,	printargs,		"SYS_496"	}, /* 496 */
	{ MA,	0,	printargs,		"SYS_497"	}, /* 497 */
	{ MA,	0,	printargs,		"SYS_498"	}, /* 498 */
	{ MA,	0,	printargs,		"SYS_499"	}, /* 499 */
	{ MA,	0,	printargs,		"SYS_500"	}, /* 500 */
	{ MA,	0,	printargs,		"SYS_501"	}, /* 501 */
	{ MA,	0,	printargs,		"SYS_502"	}, /* 502 */
	{ MA,	0,	printargs,		"SYS_503"	}, /* 503 */
	{ MA,	0,	printargs,		"SYS_504"	}, /* 504 */
	{ MA,	0,	printargs,		"SYS_505"	}, /* 505 */
	{ MA,	0,	printargs,		"SYS_506"	}, /* 506 */
	{ MA,	0,	printargs,		"SYS_507"	}, /* 507 */
	{ MA,	0,	printargs,		"SYS_508"	}, /* 508 */
	{ MA,	0,	printargs,		"SYS_509"	}, /* 509 */
	{ MA,	0,	printargs,		"SYS_510"	}, /* 510 */
	{ MA,	0,	printargs,		"SYS_511"	}, /* 511 */
	{ MA,	0,	printargs,		"SYS_512"	}, /* 512 */
	{ MA,	0,	printargs,		"SYS_513"	}, /* 513 */
	{ MA,	0,	printargs,		"SYS_514"	}, /* 514 */
	{ MA,	0,	printargs,		"SYS_515"	}, /* 515 */
	{ MA,	0,	printargs,		"SYS_516"	}, /* 516 */
	{ MA,	0,	printargs,		"SYS_517"	}, /* 517 */
	{ MA,	0,	printargs,		"SYS_518"	}, /* 518 */
	{ MA,	0,	printargs,		"SYS_519"	}, /* 519 */
	{ MA,	0,	printargs,		"SYS_520"	}, /* 520 */
	{ MA,	0,	printargs,		"SYS_521"	}, /* 521 */
	{ MA,	0,	printargs,		"SYS_522"	}, /* 522 */
	{ MA,	0,	printargs,		"SYS_523"	}, /* 523 */
	{ MA,	0,	printargs,		"SYS_524"	}, /* 524 */
	{ MA,	0,	printargs,		"SYS_525"	}, /* 525 */
	{ MA,	0,	printargs,		"SYS_526"	}, /* 526 */
	{ MA,	0,	printargs,		"SYS_527"	}, /* 527 */
	{ MA,	0,	printargs,		"SYS_528"	}, /* 528 */
	{ MA,	0,	printargs,		"SYS_529"	}, /* 529 */
	{ MA,	0,	printargs,		"SYS_530"	}, /* 530 */
	{ MA,	0,	printargs,		"SYS_531"	}, /* 531 */
	{ MA,	0,	printargs,		"SYS_532"	}, /* 532 */
	{ MA,	0,	printargs,		"SYS_533"	}, /* 533 */
	{ MA,	0,	printargs,		"SYS_534"	}, /* 534 */
	{ MA,	0,	printargs,		"SYS_535"	}, /* 535 */
	{ MA,	0,	printargs,		"SYS_536"	}, /* 536 */
	{ MA,	0,	printargs,		"SYS_537"	}, /* 537 */
	{ MA,	0,	printargs,		"SYS_538"	}, /* 538 */
	{ MA,	0,	printargs,		"SYS_539"	}, /* 539 */
	{ MA,	0,	printargs,		"SYS_540"	}, /* 540 */
	{ MA,	0,	printargs,		"SYS_541"	}, /* 541 */
	{ MA,	0,	printargs,		"SYS_542"	}, /* 542 */
	{ MA,	0,	printargs,		"SYS_543"	}, /* 543 */
	{ MA,	0,	printargs,		"SYS_544"	}, /* 544 */
	{ MA,	0,	printargs,		"SYS_545"	}, /* 545 */
	{ MA,	0,	printargs,		"SYS_546"	}, /* 546 */
	{ MA,	0,	printargs,		"SYS_547"	}, /* 547 */
	{ MA,	0,	printargs,		"SYS_548"	}, /* 548 */
	{ MA,	0,	printargs,		"SYS_549"	}, /* 549 */
	{ MA,	0,	printargs,		"SYS_550"	}, /* 550 */
	{ MA,	0,	printargs,		"SYS_551"	}, /* 551 */
	{ MA,	0,	printargs,		"SYS_552"	}, /* 552 */
	{ MA,	0,	printargs,		"SYS_553"	}, /* 553 */
	{ MA,	0,	printargs,		"SYS_554"	}, /* 554 */
	{ MA,	0,	printargs,		"SYS_555"	}, /* 555 */
	{ MA,	0,	printargs,		"SYS_556"	}, /* 556 */
	{ MA,	0,	printargs,		"SYS_557"	}, /* 557 */
	{ MA,	0,	printargs,		"SYS_558"	}, /* 558 */
	{ MA,	0,	printargs,		"SYS_559"	}, /* 559 */
	{ MA,	0,	printargs,		"SYS_560"	}, /* 560 */
	{ MA,	0,	printargs,		"SYS_561"	}, /* 561 */
	{ MA,	0,	printargs,		"SYS_562"	}, /* 562 */
	{ MA,	0,	printargs,		"SYS_563"	}, /* 563 */
	{ MA,	0,	printargs,		"SYS_564"	}, /* 564 */
	{ MA,	0,	printargs,		"SYS_565"	}, /* 565 */
	{ MA,	0,	printargs,		"SYS_566"	}, /* 566 */
	{ MA,	0,	printargs,		"SYS_567"	}, /* 567 */
	{ MA,	0,	printargs,		"SYS_568"	}, /* 568 */
	{ MA,	0,	printargs,		"SYS_569"	}, /* 569 */
	{ MA,	0,	printargs,		"SYS_570"	}, /* 570 */
	{ MA,	0,	printargs,		"SYS_571"	}, /* 571 */
	{ MA,	0,	printargs,		"SYS_572"	}, /* 572 */
	{ MA,	0,	printargs,		"SYS_573"	}, /* 573 */
	{ MA,	0,	printargs,		"SYS_574"	}, /* 574 */
	{ MA,	0,	printargs,		"SYS_575"	}, /* 575 */
	{ MA,	0,	printargs,		"SYS_576"	}, /* 576 */
	{ MA,	0,	printargs,		"SYS_577"	}, /* 577 */
	{ MA,	0,	printargs,		"SYS_578"	}, /* 578 */
	{ MA,	0,	printargs,		"SYS_579"	}, /* 579 */
	{ MA,	0,	printargs,		"SYS_580"	}, /* 580 */
	{ MA,	0,	printargs,		"SYS_581"	}, /* 581 */
	{ MA,	0,	printargs,		"SYS_582"	}, /* 582 */
	{ MA,	0,	printargs,		"SYS_583"	}, /* 583 */
	{ MA,	0,	printargs,		"SYS_584"	}, /* 584 */
	{ MA,	0,	printargs,		"SYS_585"	}, /* 585 */
	{ MA,	0,	printargs,		"SYS_586"	}, /* 586 */
	{ MA,	0,	printargs,		"SYS_587"	}, /* 587 */
	{ MA,	0,	printargs,		"SYS_588"	}, /* 588 */
	{ MA,	0,	printargs,		"SYS_589"	}, /* 589 */
	{ MA,	0,	printargs,		"SYS_590"	}, /* 590 */
	{ MA,	0,	printargs,		"SYS_591"	}, /* 591 */
	{ MA,	0,	printargs,		"SYS_592"	}, /* 592 */
	{ MA,	0,	printargs,		"SYS_593"	}, /* 593 */
	{ MA,	0,	printargs,		"SYS_594"	}, /* 594 */
	{ MA,	0,	printargs,		"SYS_595"	}, /* 595 */
	{ MA,	0,	printargs,		"SYS_596"	}, /* 596 */
	{ MA,	0,	printargs,		"SYS_597"	}, /* 597 */
	{ MA,	0,	printargs,		"SYS_598"	}, /* 598 */
	{ MA,	0,	printargs,		"SYS_599"	}, /* 599 */
	{ MA,	0,	printargs,		"SYS_600"	}, /* 600 */
	{ MA,	0,	printargs,		"SYS_601"	}, /* 601 */
	{ MA,	0,	printargs,		"SYS_602"	}, /* 602 */
	{ MA,	0,	printargs,		"SYS_603"	}, /* 603 */
	{ MA,	0,	printargs,		"SYS_604"	}, /* 604 */
	{ MA,	0,	printargs,		"SYS_605"	}, /* 605 */
	{ MA,	0,	printargs,		"SYS_606"	}, /* 606 */
	{ MA,	0,	printargs,		"SYS_607"	}, /* 607 */
	{ MA,	0,	printargs,		"SYS_608"	}, /* 608 */
	{ MA,	0,	printargs,		"SYS_609"	}, /* 609 */
	{ MA,	0,	printargs,		"SYS_610"	}, /* 610 */
	{ MA,	0,	printargs,		"SYS_611"	}, /* 611 */
	{ MA,	0,	printargs,		"SYS_612"	}, /* 612 */
	{ MA,	0,	printargs,		"SYS_613"	}, /* 613 */
	{ MA,	0,	printargs,		"SYS_614"	}, /* 614 */
	{ MA,	0,	printargs,		"SYS_615"	}, /* 615 */
	{ MA,	0,	printargs,		"SYS_616"	}, /* 616 */
	{ MA,	0,	printargs,		"SYS_617"	}, /* 617 */
	{ MA,	0,	printargs,		"SYS_618"	}, /* 618 */
	{ MA,	0,	printargs,		"SYS_619"	}, /* 619 */
	{ MA,	0,	printargs,		"SYS_620"	}, /* 620 */
	{ MA,	0,	printargs,		"SYS_621"	}, /* 621 */
	{ MA,	0,	printargs,		"SYS_622"	}, /* 622 */
	{ MA,	0,	printargs,		"SYS_623"	}, /* 623 */
	{ MA,	0,	printargs,		"SYS_624"	}, /* 624 */
	{ MA,	0,	printargs,		"SYS_625"	}, /* 625 */
	{ MA,	0,	printargs,		"SYS_626"	}, /* 626 */
	{ MA,	0,	printargs,		"SYS_627"	}, /* 627 */
	{ MA,	0,	printargs,		"SYS_628"	}, /* 628 */
	{ MA,	0,	printargs,		"SYS_629"	}, /* 629 */
	{ MA,	0,	printargs,		"SYS_630"	}, /* 630 */
	{ MA,	0,	printargs,		"SYS_631"	}, /* 631 */
	{ MA,	0,	printargs,		"SYS_632"	}, /* 632 */
	{ MA,	0,	printargs,		"SYS_633"	}, /* 633 */
	{ MA,	0,	printargs,		"SYS_634"	}, /* 634 */
	{ MA,	0,	printargs,		"SYS_635"	}, /* 635 */
	{ MA,	0,	printargs,		"SYS_636"	}, /* 636 */
	{ MA,	0,	printargs,		"SYS_637"	}, /* 637 */
	{ MA,	0,	printargs,		"SYS_638"	}, /* 638 */
	{ MA,	0,	printargs,		"SYS_639"	}, /* 639 */
	{ MA,	0,	printargs,		"SYS_640"	}, /* 640 */
	{ MA,	0,	printargs,		"SYS_641"	}, /* 641 */
	{ MA,	0,	printargs,		"SYS_642"	}, /* 642 */
	{ MA,	0,	printargs,		"SYS_643"	}, /* 643 */
	{ MA,	0,	printargs,		"SYS_644"	}, /* 644 */
	{ MA,	0,	printargs,		"SYS_645"	}, /* 645 */
	{ MA,	0,	printargs,		"SYS_646"	}, /* 646 */
	{ MA,	0,	printargs,		"SYS_647"	}, /* 647 */
	{ MA,	0,	printargs,		"SYS_648"	}, /* 648 */
	{ MA,	0,	printargs,		"SYS_649"	}, /* 649 */
	{ MA,	0,	printargs,		"SYS_650"	}, /* 650 */
	{ MA,	0,	printargs,		"SYS_651"	}, /* 651 */
	{ MA,	0,	printargs,		"SYS_652"	}, /* 652 */
	{ MA,	0,	printargs,		"SYS_653"	}, /* 653 */
	{ MA,	0,	printargs,		"SYS_654"	}, /* 654 */
	{ MA,	0,	printargs,		"SYS_655"	}, /* 655 */
	{ MA,	0,	printargs,		"SYS_656"	}, /* 656 */
	{ MA,	0,	printargs,		"SYS_657"	}, /* 657 */
	{ MA,	0,	printargs,		"SYS_658"	}, /* 658 */
	{ MA,	0,	printargs,		"SYS_659"	}, /* 659 */
	{ MA,	0,	printargs,		"SYS_660"	}, /* 660 */
	{ MA,	0,	printargs,		"SYS_661"	}, /* 661 */
	{ MA,	0,	printargs,		"SYS_662"	}, /* 662 */
	{ MA,	0,	printargs,		"SYS_663"	}, /* 663 */
	{ MA,	0,	printargs,		"SYS_664"	}, /* 664 */
	{ MA,	0,	printargs,		"SYS_665"	}, /* 665 */
	{ MA,	0,	printargs,		"SYS_666"	}, /* 666 */
	{ MA,	0,	printargs,		"SYS_667"	}, /* 667 */
	{ MA,	0,	printargs,		"SYS_668"	}, /* 668 */
	{ MA,	0,	printargs,		"SYS_669"	}, /* 669 */
	{ MA,	0,	printargs,		"SYS_670"	}, /* 670 */
	{ MA,	0,	printargs,		"SYS_671"	}, /* 671 */
	{ MA,	0,	printargs,		"SYS_672"	}, /* 672 */
	{ MA,	0,	printargs,		"SYS_673"	}, /* 673 */
	{ MA,	0,	printargs,		"SYS_674"	}, /* 674 */
	{ MA,	0,	printargs,		"SYS_675"	}, /* 675 */
	{ MA,	0,	printargs,		"SYS_676"	}, /* 676 */
	{ MA,	0,	printargs,		"SYS_677"	}, /* 677 */
	{ MA,	0,	printargs,		"SYS_678"	}, /* 678 */
	{ MA,	0,	printargs,		"SYS_679"	}, /* 679 */
	{ MA,	0,	printargs,		"SYS_680"	}, /* 680 */
	{ MA,	0,	printargs,		"SYS_681"	}, /* 681 */
	{ MA,	0,	printargs,		"SYS_682"	}, /* 682 */
	{ MA,	0,	printargs,		"SYS_683"	}, /* 683 */
	{ MA,	0,	printargs,		"SYS_684"	}, /* 684 */
	{ MA,	0,	printargs,		"SYS_685"	}, /* 685 */
	{ MA,	0,	printargs,		"SYS_686"	}, /* 686 */
	{ MA,	0,	printargs,		"SYS_687"	}, /* 687 */
	{ MA,	0,	printargs,		"SYS_688"	}, /* 688 */
	{ MA,	0,	printargs,		"SYS_689"	}, /* 689 */
	{ MA,	0,	printargs,		"SYS_690"	}, /* 690 */
	{ MA,	0,	printargs,		"SYS_691"	}, /* 691 */
	{ MA,	0,	printargs,		"SYS_692"	}, /* 692 */
	{ MA,	0,	printargs,		"SYS_693"	}, /* 693 */
	{ MA,	0,	printargs,		"SYS_694"	}, /* 694 */
	{ MA,	0,	printargs,		"SYS_695"	}, /* 695 */
	{ MA,	0,	printargs,		"SYS_696"	}, /* 696 */
	{ MA,	0,	printargs,		"SYS_697"	}, /* 697 */
	{ MA,	0,	printargs,		"SYS_698"	}, /* 698 */
	{ MA,	0,	printargs,		"SYS_699"	}, /* 699 */
	{ MA,	0,	printargs,		"SYS_700"	}, /* 700 */
	{ MA,	0,	printargs,		"SYS_701"	}, /* 701 */
	{ MA,	0,	printargs,		"SYS_702"	}, /* 702 */
	{ MA,	0,	printargs,		"SYS_703"	}, /* 703 */
	{ MA,	0,	printargs,		"SYS_704"	}, /* 704 */
	{ MA,	0,	printargs,		"SYS_705"	}, /* 705 */
	{ MA,	0,	printargs,		"SYS_706"	}, /* 706 */
	{ MA,	0,	printargs,		"SYS_707"	}, /* 707 */
	{ MA,	0,	printargs,		"SYS_708"	}, /* 708 */
	{ MA,	0,	printargs,		"SYS_709"	}, /* 709 */
	{ MA,	0,	printargs,		"SYS_710"	}, /* 710 */
	{ MA,	0,	printargs,		"SYS_711"	}, /* 711 */
	{ MA,	0,	printargs,		"SYS_712"	}, /* 712 */
	{ MA,	0,	printargs,		"SYS_713"	}, /* 713 */
	{ MA,	0,	printargs,		"SYS_714"	}, /* 714 */
	{ MA,	0,	printargs,		"SYS_715"	}, /* 715 */
	{ MA,	0,	printargs,		"SYS_716"	}, /* 716 */
	{ MA,	0,	printargs,		"SYS_717"	}, /* 717 */
	{ MA,	0,	printargs,		"SYS_718"	}, /* 718 */
	{ MA,	0,	printargs,		"SYS_719"	}, /* 719 */
	{ MA,	0,	printargs,		"SYS_720"	}, /* 720 */
	{ MA,	0,	printargs,		"SYS_721"	}, /* 721 */
	{ MA,	0,	printargs,		"SYS_722"	}, /* 722 */
	{ MA,	0,	printargs,		"SYS_723"	}, /* 723 */
	{ MA,	0,	printargs,		"SYS_724"	}, /* 724 */
	{ MA,	0,	printargs,		"SYS_725"	}, /* 725 */
	{ MA,	0,	printargs,		"SYS_726"	}, /* 726 */
	{ MA,	0,	printargs,		"SYS_727"	}, /* 727 */
	{ MA,	0,	printargs,		"SYS_728"	}, /* 728 */
	{ MA,	0,	printargs,		"SYS_729"	}, /* 729 */
	{ MA,	0,	printargs,		"SYS_730"	}, /* 730 */
	{ MA,	0,	printargs,		"SYS_731"	}, /* 731 */
	{ MA,	0,	printargs,		"SYS_732"	}, /* 732 */
	{ MA,	0,	printargs,		"SYS_733"	}, /* 733 */
	{ MA,	0,	printargs,		"SYS_734"	}, /* 734 */
	{ MA,	0,	printargs,		"SYS_735"	}, /* 735 */
	{ MA,	0,	printargs,		"SYS_736"	}, /* 736 */
	{ MA,	0,	printargs,		"SYS_737"	}, /* 737 */
	{ MA,	0,	printargs,		"SYS_738"	}, /* 738 */
	{ MA,	0,	printargs,		"SYS_739"	}, /* 739 */
	{ MA,	0,	printargs,		"SYS_740"	}, /* 740 */
	{ MA,	0,	printargs,		"SYS_741"	}, /* 741 */
	{ MA,	0,	printargs,		"SYS_742"	}, /* 742 */
	{ MA,	0,	printargs,		"SYS_743"	}, /* 743 */
	{ MA,	0,	printargs,		"SYS_744"	}, /* 744 */
	{ MA,	0,	printargs,		"SYS_745"	}, /* 745 */
	{ MA,	0,	printargs,		"SYS_746"	}, /* 746 */
	{ MA,	0,	printargs,		"SYS_747"	}, /* 747 */
	{ MA,	0,	printargs,		"SYS_748"	}, /* 748 */
	{ MA,	0,	printargs,		"SYS_749"	}, /* 749 */
	{ MA,	0,	printargs,		"SYS_750"	}, /* 750 */
	{ MA,	0,	printargs,		"SYS_751"	}, /* 751 */
	{ MA,	0,	printargs,		"SYS_752"	}, /* 752 */
	{ MA,	0,	printargs,		"SYS_753"	}, /* 753 */
	{ MA,	0,	printargs,		"SYS_754"	}, /* 754 */
	{ MA,	0,	printargs,		"SYS_755"	}, /* 755 */
	{ MA,	0,	printargs,		"SYS_756"	}, /* 756 */
	{ MA,	0,	printargs,		"SYS_757"	}, /* 757 */
	{ MA,	0,	printargs,		"SYS_758"	}, /* 758 */
	{ MA,	0,	printargs,		"SYS_759"	}, /* 759 */
	{ MA,	0,	printargs,		"SYS_760"	}, /* 760 */
	{ MA,	0,	printargs,		"SYS_761"	}, /* 761 */
	{ MA,	0,	printargs,		"SYS_762"	}, /* 762 */
	{ MA,	0,	printargs,		"SYS_763"	}, /* 763 */
	{ MA,	0,	printargs,		"SYS_764"	}, /* 764 */
	{ MA,	0,	printargs,		"SYS_765"	}, /* 765 */
	{ MA,	0,	printargs,		"SYS_766"	}, /* 766 */
	{ MA,	0,	printargs,		"SYS_767"	}, /* 767 */
	{ MA,	0,	printargs,		"SYS_768"	}, /* 768 */
	{ MA,	0,	printargs,		"SYS_769"	}, /* 769 */
	{ MA,	0,	printargs,		"SYS_770"	}, /* 770 */
	{ MA,	0,	printargs,		"SYS_771"	}, /* 771 */
	{ MA,	0,	printargs,		"SYS_772"	}, /* 772 */
	{ MA,	0,	printargs,		"SYS_773"	}, /* 773 */
	{ MA,	0,	printargs,		"SYS_774"	}, /* 774 */
	{ MA,	0,	printargs,		"SYS_775"	}, /* 775 */
	{ MA,	0,	printargs,		"SYS_776"	}, /* 776 */
	{ MA,	0,	printargs,		"SYS_777"	}, /* 777 */
	{ MA,	0,	printargs,		"SYS_778"	}, /* 778 */
	{ MA,	0,	printargs,		"SYS_779"	}, /* 779 */
	{ MA,	0,	printargs,		"SYS_780"	}, /* 780 */
	{ MA,	0,	printargs,		"SYS_781"	}, /* 781 */
	{ MA,	0,	printargs,		"SYS_782"	}, /* 782 */
	{ MA,	0,	printargs,		"SYS_783"	}, /* 783 */
	{ MA,	0,	printargs,		"SYS_784"	}, /* 784 */
	{ MA,	0,	printargs,		"SYS_785"	}, /* 785 */
	{ MA,	0,	printargs,		"SYS_786"	}, /* 786 */
	{ MA,	0,	printargs,		"SYS_787"	}, /* 787 */
	{ MA,	0,	printargs,		"SYS_788"	}, /* 788 */
	{ MA,	0,	printargs,		"SYS_789"	}, /* 789 */
	{ MA,	0,	printargs,		"SYS_790"	}, /* 790 */
	{ MA,	0,	printargs,		"SYS_791"	}, /* 791 */
	{ MA,	0,	printargs,		"SYS_792"	}, /* 792 */
	{ MA,	0,	printargs,		"SYS_793"	}, /* 793 */
	{ MA,	0,	printargs,		"SYS_794"	}, /* 794 */
	{ MA,	0,	printargs,		"SYS_795"	}, /* 795 */
	{ MA,	0,	printargs,		"SYS_796"	}, /* 796 */
	{ MA,	0,	printargs,		"SYS_797"	}, /* 797 */
	{ MA,	0,	printargs,		"SYS_798"	}, /* 798 */
	{ MA,	0,	printargs,		"SYS_799"	}, /* 799 */
	{ MA,	0,	printargs,		"SYS_800"	}, /* 800 */
	{ MA,	0,	printargs,		"SYS_801"	}, /* 801 */
	{ MA,	0,	printargs,		"SYS_802"	}, /* 802 */
	{ MA,	0,	printargs,		"SYS_803"	}, /* 803 */
	{ MA,	0,	printargs,		"SYS_804"	}, /* 804 */
	{ MA,	0,	printargs,		"SYS_805"	}, /* 805 */
	{ MA,	0,	printargs,		"SYS_806"	}, /* 806 */
	{ MA,	0,	printargs,		"SYS_807"	}, /* 807 */
	{ MA,	0,	printargs,		"SYS_808"	}, /* 808 */
	{ MA,	0,	printargs,		"SYS_809"	}, /* 809 */
	{ MA,	0,	printargs,		"SYS_810"	}, /* 810 */
	{ MA,	0,	printargs,		"SYS_811"	}, /* 811 */
	{ MA,	0,	printargs,		"SYS_812"	}, /* 812 */
	{ MA,	0,	printargs,		"SYS_813"	}, /* 813 */
	{ MA,	0,	printargs,		"SYS_814"	}, /* 814 */
	{ MA,	0,	printargs,		"SYS_815"	}, /* 815 */
	{ MA,	0,	printargs,		"SYS_816"	}, /* 816 */
	{ MA,	0,	printargs,		"SYS_817"	}, /* 817 */
	{ MA,	0,	printargs,		"SYS_818"	}, /* 818 */
	{ MA,	0,	printargs,		"SYS_819"	}, /* 819 */
	{ MA,	0,	printargs,		"SYS_820"	}, /* 820 */
	{ MA,	0,	printargs,		"SYS_821"	}, /* 821 */
	{ MA,	0,	printargs,		"SYS_822"	}, /* 822 */
	{ MA,	0,	printargs,		"SYS_823"	}, /* 823 */
	{ MA,	0,	printargs,		"SYS_824"	}, /* 824 */
	{ MA,	0,	printargs,		"SYS_825"	}, /* 825 */
	{ MA,	0,	printargs,		"SYS_826"	}, /* 826 */
	{ MA,	0,	printargs,		"SYS_827"	}, /* 827 */
	{ MA,	0,	printargs,		"SYS_828"	}, /* 828 */
	{ MA,	0,	printargs,		"SYS_829"	}, /* 829 */
	{ MA,	0,	printargs,		"SYS_830"	}, /* 830 */
	{ MA,	0,	printargs,		"SYS_831"	}, /* 831 */
	{ MA,	0,	printargs,		"SYS_832"	}, /* 832 */
	{ MA,	0,	printargs,		"SYS_833"	}, /* 833 */
	{ MA,	0,	printargs,		"SYS_834"	}, /* 834 */
	{ MA,	0,	printargs,		"SYS_835"	}, /* 835 */
	{ MA,	0,	printargs,		"SYS_836"	}, /* 836 */
	{ MA,	0,	printargs,		"SYS_837"	}, /* 837 */
	{ MA,	0,	printargs,		"SYS_838"	}, /* 838 */
	{ MA,	0,	printargs,		"SYS_839"	}, /* 839 */
	{ MA,	0,	printargs,		"SYS_840"	}, /* 840 */
	{ MA,	0,	printargs,		"SYS_841"	}, /* 841 */
	{ MA,	0,	printargs,		"SYS_842"	}, /* 842 */
	{ MA,	0,	printargs,		"SYS_843"	}, /* 843 */
	{ MA,	0,	printargs,		"SYS_844"	}, /* 844 */
	{ MA,	0,	printargs,		"SYS_845"	}, /* 845 */
	{ MA,	0,	printargs,		"SYS_846"	}, /* 846 */
	{ MA,	0,	printargs,		"SYS_847"	}, /* 847 */
	{ MA,	0,	printargs,		"SYS_848"	}, /* 848 */
	{ MA,	0,	printargs,		"SYS_849"	}, /* 849 */
	{ MA,	0,	printargs,		"SYS_850"	}, /* 850 */
	{ MA,	0,	printargs,		"SYS_851"	}, /* 851 */
	{ MA,	0,	printargs,		"SYS_852"	}, /* 852 */
	{ MA,	0,	printargs,		"SYS_853"	}, /* 853 */
	{ MA,	0,	printargs,		"SYS_854"	}, /* 854 */
	{ MA,	0,	printargs,		"SYS_855"	}, /* 855 */
	{ MA,	0,	printargs,		"SYS_856"	}, /* 856 */
	{ MA,	0,	printargs,		"SYS_857"	}, /* 857 */
	{ MA,	0,	printargs,		"SYS_858"	}, /* 858 */
	{ MA,	0,	printargs,		"SYS_859"	}, /* 859 */
	{ MA,	0,	printargs,		"SYS_860"	}, /* 860 */
	{ MA,	0,	printargs,		"SYS_861"	}, /* 861 */
	{ MA,	0,	printargs,		"SYS_862"	}, /* 862 */
	{ MA,	0,	printargs,		"SYS_863"	}, /* 863 */
	{ MA,	0,	printargs,		"SYS_864"	}, /* 864 */
	{ MA,	0,	printargs,		"SYS_865"	}, /* 865 */
	{ MA,	0,	printargs,		"SYS_866"	}, /* 866 */
	{ MA,	0,	printargs,		"SYS_867"	}, /* 867 */
	{ MA,	0,	printargs,		"SYS_868"	}, /* 868 */
	{ MA,	0,	printargs,		"SYS_869"	}, /* 869 */
	{ MA,	0,	printargs,		"SYS_870"	}, /* 870 */
	{ MA,	0,	printargs,		"SYS_871"	}, /* 871 */
	{ MA,	0,	printargs,		"SYS_872"	}, /* 872 */
	{ MA,	0,	printargs,		"SYS_873"	}, /* 873 */
	{ MA,	0,	printargs,		"SYS_874"	}, /* 874 */
	{ MA,	0,	printargs,		"SYS_875"	}, /* 875 */
	{ MA,	0,	printargs,		"SYS_876"	}, /* 876 */
	{ MA,	0,	printargs,		"SYS_877"	}, /* 877 */
	{ MA,	0,	printargs,		"SYS_878"	}, /* 878 */
	{ MA,	0,	printargs,		"SYS_879"	}, /* 879 */
	{ MA,	0,	printargs,		"SYS_880"	}, /* 880 */
	{ MA,	0,	printargs,		"SYS_881"	}, /* 881 */
	{ MA,	0,	printargs,		"SYS_882"	}, /* 882 */
	{ MA,	0,	printargs,		"SYS_883"	}, /* 883 */
	{ MA,	0,	printargs,		"SYS_884"	}, /* 884 */
	{ MA,	0,	printargs,		"SYS_885"	}, /* 885 */
	{ MA,	0,	printargs,		"SYS_886"	}, /* 886 */
	{ MA,	0,	printargs,		"SYS_887"	}, /* 887 */
	{ MA,	0,	printargs,		"SYS_888"	}, /* 888 */
	{ MA,	0,	printargs,		"SYS_889"	}, /* 889 */
	{ MA,	0,	printargs,		"SYS_890"	}, /* 890 */
	{ MA,	0,	printargs,		"SYS_891"	}, /* 891 */
	{ MA,	0,	printargs,		"SYS_892"	}, /* 892 */
	{ MA,	0,	printargs,		"SYS_893"	}, /* 893 */
	{ MA,	0,	printargs,		"SYS_894"	}, /* 894 */
	{ MA,	0,	printargs,		"SYS_895"	}, /* 895 */
	{ MA,	0,	printargs,		"SYS_896"	}, /* 896 */
	{ MA,	0,	printargs,		"SYS_897"	}, /* 897 */
	{ MA,	0,	printargs,		"SYS_898"	}, /* 898 */
	{ MA,	0,	printargs,		"SYS_899"	}, /* 899 */
	{ MA,	0,	printargs,		"SYS_900"	}, /* 900 */
	{ MA,	0,	printargs,		"SYS_901"	}, /* 901 */
	{ MA,	0,	printargs,		"SYS_902"	}, /* 902 */
	{ MA,	0,	printargs,		"SYS_903"	}, /* 903 */
	{ MA,	0,	printargs,		"SYS_904"	}, /* 904 */
	{ MA,	0,	printargs,		"SYS_905"	}, /* 905 */
	{ MA,	0,	printargs,		"SYS_906"	}, /* 906 */
	{ MA,	0,	printargs,		"SYS_907"	}, /* 907 */
	{ MA,	0,	printargs,		"SYS_908"	}, /* 908 */
	{ MA,	0,	printargs,		"SYS_909"	}, /* 909 */
	{ MA,	0,	printargs,		"SYS_910"	}, /* 910 */
	{ MA,	0,	printargs,		"SYS_911"	}, /* 911 */
	{ MA,	0,	printargs,		"SYS_912"	}, /* 912 */
	{ MA,	0,	printargs,		"SYS_913"	}, /* 913 */
	{ MA,	0,	printargs,		"SYS_914"	}, /* 914 */
	{ MA,	0,	printargs,		"SYS_915"	}, /* 915 */
	{ MA,	0,	printargs,		"SYS_916"	}, /* 916 */
	{ MA,	0,	printargs,		"SYS_917"	}, /* 917 */
	{ MA,	0,	printargs,		"SYS_918"	}, /* 918 */
	{ MA,	0,	printargs,		"SYS_919"	}, /* 919 */
	{ MA,	0,	printargs,		"SYS_920"	}, /* 920 */
	{ MA,	0,	printargs,		"SYS_921"	}, /* 921 */
	{ MA,	0,	printargs,		"SYS_922"	}, /* 922 */
	{ MA,	0,	printargs,		"SYS_923"	}, /* 923 */
	{ MA,	0,	printargs,		"SYS_924"	}, /* 924 */
	{ MA,	0,	printargs,		"SYS_925"	}, /* 925 */
	{ MA,	0,	printargs,		"SYS_926"	}, /* 926 */
	{ MA,	0,	printargs,		"SYS_927"	}, /* 927 */
	{ MA,	0,	printargs,		"SYS_928"	}, /* 928 */
	{ MA,	0,	printargs,		"SYS_929"	}, /* 929 */
	{ MA,	0,	printargs,		"SYS_930"	}, /* 930 */
	{ MA,	0,	printargs,		"SYS_931"	}, /* 931 */
	{ MA,	0,	printargs,		"SYS_932"	}, /* 932 */
	{ MA,	0,	printargs,		"SYS_933"	}, /* 933 */
	{ MA,	0,	printargs,		"SYS_934"	}, /* 934 */
	{ MA,	0,	printargs,		"SYS_935"	}, /* 935 */
	{ MA,	0,	printargs,		"SYS_936"	}, /* 936 */
	{ MA,	0,	printargs,		"SYS_937"	}, /* 937 */
	{ MA,	0,	printargs,		"SYS_938"	}, /* 938 */
	{ MA,	0,	printargs,		"SYS_939"	}, /* 939 */
	{ MA,	0,	printargs,		"SYS_940"	}, /* 940 */
	{ MA,	0,	printargs,		"SYS_941"	}, /* 941 */
	{ MA,	0,	printargs,		"SYS_942"	}, /* 942 */
	{ MA,	0,	printargs,		"SYS_943"	}, /* 943 */
	{ MA,	0,	printargs,		"SYS_944"	}, /* 944 */
	{ MA,	0,	printargs,		"SYS_945"	}, /* 945 */
	{ MA,	0,	printargs,		"SYS_946"	}, /* 946 */
	{ MA,	0,	printargs,		"SYS_947"	}, /* 947 */
	{ MA,	0,	printargs,		"SYS_948"	}, /* 948 */
	{ MA,	0,	printargs,		"SYS_949"	}, /* 949 */
	{ MA,	0,	printargs,		"SYS_950"	}, /* 950 */
	{ MA,	0,	printargs,		"SYS_951"	}, /* 951 */
	{ MA,	0,	printargs,		"SYS_952"	}, /* 952 */
	{ MA,	0,	printargs,		"SYS_953"	}, /* 953 */
	{ MA,	0,	printargs,		"SYS_954"	}, /* 954 */
	{ MA,	0,	printargs,		"SYS_955"	}, /* 955 */
	{ MA,	0,	printargs,		"SYS_956"	}, /* 956 */
	{ MA,	0,	printargs,		"SYS_957"	}, /* 957 */
	{ MA,	0,	printargs,		"SYS_958"	}, /* 958 */
	{ MA,	0,	printargs,		"SYS_959"	}, /* 959 */
	{ MA,	0,	printargs,		"SYS_960"	}, /* 960 */
	{ MA,	0,	printargs,		"SYS_961"	}, /* 961 */
	{ MA,	0,	printargs,		"SYS_962"	}, /* 962 */
	{ MA,	0,	printargs,		"SYS_963"	}, /* 963 */
	{ MA,	0,	printargs,		"SYS_964"	}, /* 964 */
	{ MA,	0,	printargs,		"SYS_965"	}, /* 965 */
	{ MA,	0,	printargs,		"SYS_966"	}, /* 966 */
	{ MA,	0,	printargs,		"SYS_967"	}, /* 967 */
	{ MA,	0,	printargs,		"SYS_968"	}, /* 968 */
	{ MA,	0,	printargs,		"SYS_969"	}, /* 969 */
	{ MA,	0,	printargs,		"SYS_970"	}, /* 970 */
	{ MA,	0,	printargs,		"SYS_971"	}, /* 971 */
	{ MA,	0,	printargs,		"SYS_972"	}, /* 972 */
	{ MA,	0,	printargs,		"SYS_973"	}, /* 973 */
	{ MA,	0,	printargs,		"SYS_974"	}, /* 974 */
	{ MA,	0,	printargs,		"SYS_975"	}, /* 975 */
	{ MA,	0,	printargs,		"SYS_976"	}, /* 976 */
	{ MA,	0,	printargs,		"SYS_977"	}, /* 977 */
	{ MA,	0,	printargs,		"SYS_978"	}, /* 978 */
	{ MA,	0,	printargs,		"SYS_979"	}, /* 979 */
	{ MA,	0,	printargs,		"SYS_980"	}, /* 980 */
	{ MA,	0,	printargs,		"SYS_981"	}, /* 981 */
	{ MA,	0,	printargs,		"SYS_982"	}, /* 982 */
	{ MA,	0,	printargs,		"SYS_983"	}, /* 983 */
	{ MA,	0,	printargs,		"SYS_984"	}, /* 984 */
	{ MA,	0,	printargs,		"SYS_985"	}, /* 985 */
	{ MA,	0,	printargs,		"SYS_986"	}, /* 986 */
	{ MA,	0,	printargs,		"SYS_987"	}, /* 987 */
	{ MA,	0,	printargs,		"SYS_988"	}, /* 988 */
	{ MA,	0,	printargs,		"SYS_989"	}, /* 989 */
	{ MA,	0,	printargs,		"SYS_990"	}, /* 990 */
	{ MA,	0,	printargs,		"SYS_991"	}, /* 991 */
	{ MA,	0,	printargs,		"SYS_992"	}, /* 992 */
	{ MA,	0,	printargs,		"SYS_993"	}, /* 993 */
	{ MA,	0,	printargs,		"SYS_994"	}, /* 994 */
	{ MA,	0,	printargs,		"SYS_995"	}, /* 995 */
	{ MA,	0,	printargs,		"SYS_996"	}, /* 996 */
	{ MA,	0,	printargs,		"SYS_997"	}, /* 997 */
	{ MA,	0,	printargs,		"SYS_998"	}, /* 998 */
	{ MA,	0,	printargs,		"SYS_999"	}, /* 999 */
	{ MA,	0,	printargs,		"SYS_1000"	}, /* 1000 */
	{ MA,	0,	printargs,		"SYS_1001"	}, /* 1001 */
	{ MA,	0,	printargs,		"SYS_1002"	}, /* 1002 */
	{ MA,	0,	printargs,		"SYS_1003"	}, /* 1003 */
	{ MA,	0,	printargs,		"SYS_1004"	}, /* 1004 */
	{ MA,	0,	printargs,		"SYS_1005"	}, /* 1005 */
	{ MA,	0,	printargs,		"SYS_1006"	}, /* 1006 */
	{ MA,	0,	printargs,		"SYS_1007"	}, /* 1007 */
	{ MA,	0,	printargs,		"SYS_1008"	}, /* 1008 */
	{ MA,	0,	printargs,		"SYS_1009"	}, /* 1009 */
	{ MA,	0,	printargs,		"SYS_1010"	}, /* 1010 */
	{ MA,	0,	printargs,		"SYS_1011"	}, /* 1011 */
	{ MA,	0,	printargs,		"SYS_1012"	}, /* 1012 */
	{ MA,	0,	printargs,		"SYS_1013"	}, /* 1013 */
	{ MA,	0,	printargs,		"SYS_1014"	}, /* 1014 */
	{ MA,	0,	printargs,		"SYS_1015"	}, /* 1015 */
	{ MA,	0,	printargs,		"SYS_1016"	}, /* 1016 */
	{ MA,	0,	printargs,		"SYS_1017"	}, /* 1017 */
	{ MA,	0,	printargs,		"SYS_1018"	}, /* 1018 */
	{ MA,	0,	printargs,		"SYS_1019"	}, /* 1019 */
	{ MA,	0,	printargs,		"SYS_1020"	}, /* 1020 */
	{ MA,	0,	printargs,		"SYS_1021"	}, /* 1021 */
	{ MA,	0,	printargs,		"SYS_1022"	}, /* 1022 */
	{ MA,	0,	printargs,		"SYS_1023"	}, /* 1023 */
	{ 0,	0,	printargs,		"ni_syscall"	}, /* 1024 */
	{ 1,	TP,	sys_exit,		"exit"		}, /* 1025 */
	{ 3,	TD,	sys_read,		"read"		}, /* 1026 */
	{ 3,	TD,	sys_write,		"write"		}, /* 1027 */
	{ 3,	TD|TF,	sys_open,		"open"		}, /* 1028 */
	{ 1,	TD,	sys_close,		"close"		}, /* 1029 */
	{ 2,	0,	sys_creat,		"creat"		}, /* 1030 */
	{ 2,	TF,	sys_link,		"link"		}, /* 1031 */
	{ 1,	TF,	sys_unlink,		"unlink"	}, /* 1032 */
	{ 3,	TF|TP,	sys_execve,		"execve"	}, /* 1033 */
	{ 1,	TF,	sys_chdir,		"chdir"		}, /* 1034 */
	{ 1,	TF,	sys_fchdir,		"fchdir"	}, /* 1035 */
	{ 2,	0,	sys_utimes,		"utimes"	}, /* 1036 */
	{ 3,	TF,	sys_mknod,		"mknod"		}, /* 1037 */
	{ 2,	TF,	sys_chmod,		"chmod"		}, /* 1038 */
	{ 3,	TF,	sys_chown,		"chown"		}, /* 1039 */
	{ 3,	TF,	sys_lseek,		"lseek"		}, /* 1040 */
	{ 0,	0,	sys_getpid,		"getpid"	}, /* 1041 */
	{ 0,	0,	sys_getppid,		"getppid"	}, /* 1042 */
	{ 5,	TF,	sys_mount,		"mount"		}, /* 1043 */
	{ 1,	0,	sys_umount2,		"umount"	}, /* 1044 */
	{ 1,	0,	sys_setuid,		"setuid"	}, /* 1045 */
	{ 0,	NF,	sys_getuid,		"getuid"	}, /* 1046 */
	{ 0,	NF,	sys_geteuid,		"geteuid"	}, /* 1047 */
	{ 4,	0,	sys_ptrace,		"ptrace"	}, /* 1048 */
	{ 2,	TF,	sys_access,		"access"	}, /* 1049 */
	{ 0,	0,	sys_sync,		"sync"		}, /* 1050 */
	{ 1,	TD,	sys_fsync,		"fsync"		}, /* 1051 */
	{ 1,	TD,	sys_fdatasync,		"fdatasync"	}, /* 1052 */
	{ 2,	TS,	sys_kill,		"kill"		}, /* 1053 */
	{ 2,	TF,	sys_rename,		"rename"	}, /* 1054 */
	{ 2,	TF,	sys_mkdir,		"mkdir"		}, /* 1055 */
	{ 1,	TF,	sys_rmdir,		"rmdir"		}, /* 1056 */
	{ 1,	TD,	sys_dup,		"dup"		}, /* 1057 */
	{ 1,	TD,	sys_pipe,		"pipe"		}, /* 1058 */
	{ 1,	0,	sys_times,		"times"		}, /* 1059 */
	{ 1,	0,	sys_brk,		"brk"		}, /* 1060 */
	{ 1,	0,	sys_setgid,		"setgid"	}, /* 1061 */
	{ 0,	NF,	sys_getgid,		"getgid"	}, /* 1062 */
	{ 0,	NF,	sys_getegid,		"getegid"	}, /* 1063 */
	{ 1,	TF,	sys_acct,		"acct"		}, /* 1064 */
	{ 3,	TD,	sys_ioctl,		"ioctl"		}, /* 1065 */
	{ 3,	TD,	sys_fcntl,		"fcntl"		}, /* 1066 */
	{ 1,	0,	sys_umask,		"umask"		}, /* 1067 */
	{ 1,	TF,	sys_chroot,		"chroot"	}, /* 1068 */
	{ 2,	0,	sys_ustat,		"ustat"		}, /* 1069 */
	{ 2,	TD,	sys_dup2,		"dup2"		}, /* 1070 */
	{ 2,	0,	sys_setreuid,		"setreuid"	}, /* 1071 */
	{ 2,	0,	sys_setregid,		"setregid"	}, /* 1072 */
	{ 3,	0,	printargs,		"getresuid"	}, /* 1073 */
	{ 3,	0,	sys_setresuid,		"setresuid"	}, /* 1074 */
	{ 3,	0,	sys_getresuid,		"getresgid"	}, /* 1075 */
	{ 3,	0,	printargs,		"setresgid"	}, /* 1076 */
	{ 2,	0,	sys_getgroups,		"getgroups"	}, /* 1077 */
	{ 2,	0,	sys_setgroups,		"setgroups"	}, /* 1078 */
	{ 1,	0,	sys_getpgid,		"getpgid"	}, /* 1079 */
	{ 2,	0,	sys_setpgid,		"setpgid"	}, /* 1080 */
	{ 0,	0,	sys_setsid,		"setsid"	}, /* 1081 */
	{ 1,	0,	sys_getsid,		"getsid"	}, /* 1082 */
	{ 2,	0,	sys_sethostname,	"sethostname"	}, /* 1083 */
	{ 2,	0,	sys_setrlimit,		"setrlimit"	}, /* 1084 */
	{ 2,	0,	sys_getrlimit,		"getrlimit"	}, /* 1085 */
	{ 2,	0,	sys_getrusage,		"getrusage"	}, /* 1086 */
	{ 2,	0,	sys_gettimeofday,	"gettimeofday"	}, /* 1087 */
	{ 2,	0,	sys_settimeofday,	"settimeofday"	}, /* 1088 */
	{ 5,	TD,	sys_select,		"select"	}, /* 1089 */
	{ 3,	TD,	sys_poll,		"poll"		}, /* 1090 */
	{ 2,	TF,	sys_symlink,		"symlink"	}, /* 1091 */
	{ 3,	TF,	sys_readlink,		"readlink"	}, /* 1092 */
	{ 1,	0,	sys_uselib,		"uselib"	}, /* 1093 */
	{ 2,	TF,	sys_swapon,		"swapon"	}, /* 1094 */
	{ 1,	TF,	sys_swapoff,		"swapoff"	}, /* 1095 */
	{ 4,	0,	sys_reboot,		"reboot"	}, /* 1096 */
	{ 2,	TF,	sys_truncate,		"truncate"	}, /* 1097 */
	{ 2,	TD,	sys_ftruncate,		"ftruncate"	}, /* 1098 */
	{ 2,	TD,	sys_fchmod,		"fchmod"	}, /* 1099 */
	{ 3,	TD,	sys_fchown,		"fchown"	}, /* 1100 */
	{ 2,	0,	sys_getpriority,	"getpriority"	}, /* 1101 */
	{ 3,	0,	sys_setpriority,	"setpriority"	}, /* 1102 */
	{ 2,	TF,	sys_statfs,		"statfs"	}, /* 1103 */
	{ 2,	TD,	sys_fstatfs,		"fstatfs"	}, /* 1104 */
	{ 3,	0,	sys_gettid,		"gettid"	}, /* 1105 */
	{ 3,	TI,	sys_semget,		"semget"	}, /* 1106 */
	{ 3,	TI,	printargs,		"semop"		}, /* 1107 */
	{ 4,	TI,	sys_semctl,		"semctl"	}, /* 1108 */
	{ 2,	TI,	sys_msgget,		"msgget"	}, /* 1109 */
	{ 4,	TI,	sys_msgsnd,		"msgsnd"	}, /* 1110 */
	{ 5,	TI,	sys_msgrcv,		"msgrcv"	}, /* 1111 */
	{ 3,	TI,	sys_msgctl,		"msgctl"	}, /* 1112 */
	{ 3,	TI,	sys_shmget,		"shmget"	}, /* 1113 */
	{ 3,	TI,	sys_shmat,		"shmat"		}, /* 1114 */
	{ 1,	TI,	sys_shmdt,		"shmdt"		}, /* 1115 */
	{ 3,	TI,	sys_shmctl,		"shmctl"	}, /* 1116 */
	{ 3,	0,	sys_syslog,		"syslog"	}, /* 1117 */
	{ 3,	0,	sys_setitimer,		"setitimer"	}, /* 1118 */
	{ 2,	0,	sys_getitimer,		"getitimer"	}, /* 1119 */
	{ 2,	TF,	sys_stat,		"stat"		}, /* 1120 */
	{ 2,	TF,	sys_lstat,		"lstat"		}, /* 1121 */
	{ 2,	TD,	sys_fstat,		"fstat"		}, /* 1122 */
	{ 0,	0,	sys_vhangup,		"vhangup"	}, /* 1123 */
	{ 3,	TF,	sys_chown,		"lchown"	}, /* 1124 */
	{ 5,	0,	sys_vm86,		"vm86"		}, /* 1125 */
	{ 4,	TP,	sys_wait4,		"wait4"		}, /* 1126 */
	{ 1,	0,	sys_sysinfo,		"sysinfo"	}, /* 1127 */
	{ 5,	TP,	sys_clone,		"clone"		}, /* 1128 */
	{ 2,	0,	sys_setdomainname,	"setdomainname"	}, /* 1129 */
	{ 1,	0,	sys_uname,		"uname"		}, /* 1130 */
	{ 1,	0,	sys_adjtimex,		"adjtimex"	}, /* 1131 */
	{ 2,	0,	sys_create_module,	"create_module"	}, /* 1132 */
	{ 4,	0,	sys_init_module,	"init_module"	}, /* 1133 */
	{ 2,	0,	sys_delete_module,	"delete_module"	}, /* 1134 */
	{ 1,	0,	sys_get_kernel_syms,	"get_kernel_syms"}, /* 1135 */
	{ 5,	0,	sys_query_module,	"query_module"	}, /* 1136 */
	{ 4,	0,	sys_quotactl,		"quotactl"	}, /* 1137 */
	{ 0,	0,	sys_bdflush,		"bdflush"	}, /* 1138 */
	{ 3,	0,	sys_sysfs,		"sysfs"		}, /* 1139 */
	{ 1,	0,	sys_personality,	"personality"	}, /* 1140 */
	{ 5,	0,	sys_afs_syscall,	"afs_syscall"	}, /* 1141 */
	{ 1,	NF,	sys_setfsuid,		"setfsuid"	}, /* 1142 */
	{ 1,	NF,	sys_setfsgid,		"setfsgid"	}, /* 1143 */
	{ 3,	TD,	sys_getdents,		"getdents"	}, /* 1144 */
	{ 2,	TD,	sys_flock,		"flock"		}, /* 1145 */
	{ 5,	TD,	sys_readv,		"readv"		}, /* 1146 */
	{ 5,	TD,	sys_writev,		"writev"	}, /* 1147 */
	{ 4,	TD,	sys_pread,		"pread"		}, /* 1148 */
	{ 4,	TD,	sys_pwrite,		"pwrite"	}, /* 1149 */
	{ 1,	0,	printargs,		"_sysctl"	}, /* 1150 */
	{ 6,	TD,	sys_mmap,		"mmap"		}, /* 1151 */
	{ 2,	0,	sys_munmap,		"munmap"	}, /* 1152 */
	{ 2,	0,	sys_mlock,		"mlock"		}, /* 1153 */
	{ 1,	0,	sys_mlockall,		"mlockall"	}, /* 1154 */
	{ 3,	0,	sys_mprotect,		"mprotect"	}, /* 1155 */
	{ 5,	0,	sys_mremap,		"mremap"	}, /* 1156 */
	{ 3,	0,	sys_msync,		"msync"		}, /* 1157 */
	{ 2,	0,	sys_munlock,		"munlock"	}, /* 1158 */
	{ 0,	0,	sys_munlockall,		"munlockall"	}, /* 1159 */
	{ 2,	0,	sys_sched_getparam,	"sched_getparam"}, /* 1160 */
	{ 2,	0,	sys_sched_setparam,	"sched_setparam"}, /* 1161 */
	{ 2,	0,	sys_sched_getscheduler,	"sched_getscheduler"}, /* 1162 */
	{ 3,	0,	sys_sched_setscheduler,	"sched_setscheduler"}, /* 1163 */
	{ 0,	0,	sys_sched_yield,	"sched_yield"	}, /* 1164 */
	{ 1,	0,	sys_sched_get_priority_max,"sched_get_priority_max"}, /* 1165 */
	{ 1,	0,	sys_sched_get_priority_min,"sched_get_priority_min"}, /* 1166 */
	{ 2,	0,	sys_sched_rr_get_interval,"sched_rr_get_interval"}, /* 1167 */
	{ 2,	0,	sys_nanosleep,		"nanosleep"	}, /* 1168 */
	{ 3,	0,	sys_nfsservctl,		"nfsservctl"	}, /* 1169 */
	{ 5,	0,	sys_prctl,		"prctl"		}, /* 1170 */
	{ 1,	0,	sys_getpagesize,	"getpagesize"	}, /* 1171 */
	{ 6,	TD,	sys_mmap,		"mmap2"		}, /* 1172 */
	{ 5,	0,	printargs,		"pciconfig_read"}, /* 1173 */
	{ 5,	0,	printargs,		"pciconfig_write"}, /* 1174 */
	{ MA,	0,	printargs,		"perfmonctl"	}, /* 1175 */
	{ 2,	TS,	sys_sigaltstack,	"sigaltstack"	}, /* 1176 */
	{ 4,	TS,	sys_rt_sigaction,	"rt_sigaction"	}, /* 1177 */
	{ 2,	TS,	sys_rt_sigpending,	"rt_sigpending"	}, /* 1178 */
	{ 4,	TS,	sys_rt_sigprocmask,	"rt_sigprocmask"}, /* 1179 */
	{ 3,	TS,	sys_rt_sigqueueinfo,	"rt_sigqueueinfo"}, /* 1180 */
	{ 0,	TS,	sys_sigreturn,		"rt_sigreturn"	}, /* 1181 */
	{ 2,	TS,	sys_rt_sigsuspend,	"rt_sigsuspend"	}, /* 1182 */
	{ 4,	TS,	sys_rt_sigtimedwait,	"rt_sigtimedwait"}, /* 1183 */
	{ 2,	TF,	sys_getcwd,		"getcwd"	}, /* 1184 */
	{ 2,	0,	sys_capget,		"capget"	}, /* 1185 */
	{ 2,	0,	sys_capset,		"capset"	}, /* 1186 */
	{ 4,	TD|TN,	sys_sendfile,		"sendfile"	}, /* 1187 */
	{ 5,	TN,	printargs,		"getpmsg"	}, /* 1188 */
	{ 5,	TN,	printargs,		"putpmsg"	}, /* 1189 */
	{ 3,	TN,	sys_socket,		"socket"	}, /* 1190 */
	{ 3,	TN,	sys_bind,		"bind"		}, /* 1191 */
	{ 3,	TN,	sys_connect,		"connect"	}, /* 1192 */
	{ 2,	TN,	sys_listen,		"listen"	}, /* 1193 */
	{ 3,	TN,	sys_accept,		"accept"	}, /* 1194 */
	{ 3,	TN,	sys_getsockname,	"getsockname"	}, /* 1195 */
	{ 3,	TN,	sys_getpeername,	"getpeername"	}, /* 1196 */
	{ 4,	TN,	sys_socketpair,		"socketpair"	}, /* 1197 */
	{ 4,	TN,	sys_send,		"send"		}, /* 1198 */
	{ 6,	TN,	sys_sendto,		"sendto"	}, /* 1199 */
	{ 4,	TN,	sys_recv,		"recv"		}, /* 1200 */
	{ 6,	TN,	sys_recvfrom,		"recvfrom"	}, /* 1201 */
	{ 2,	TN,	sys_shutdown,		"shutdown"	}, /* 1202 */
	{ 5,	TN,	sys_setsockopt,		"setsockopt"	}, /* 1203 */
	{ 5,	TN,	sys_getsockopt,		"getsockopt"	}, /* 1204 */
	{ 3,	TN,	sys_sendmsg,		"sendmsg"	}, /* 1205 */
	{ 3,	TN,	sys_recvmsg,		"recvmsg"	}, /* 1206 */
	{ 2,	TF,	sys_pivotroot,		"pivot_root"	}, /* 1207 */
	{ 3,	0,	sys_mincore,		"mincore"	}, /* 1208 */
	{ 3,	0,	sys_madvise,		"madvise"	}, /* 1209 */
	{ 2,	TF,	sys_stat,		"stat"		}, /* 1210 */
	{ 2,	0,	sys_lstat,		"lstat"		}, /* 1211 */
	{ 2,	TD,	sys_fstat,		"fstat"		}, /* 1212 */
	{ 6,	TP,	sys_clone,		"clone2"	}, /* 1213 */
	{ 3,	TD,	sys_getdents64,		"getdents64"	}, /* 1214 */
	{ 2,	0,	printargs,		"getunwind"	}, /* 1215 */
	{ 4,	TD,	sys_readahead,		"readahead"	}, /* 1216 */
	{ 5,	TF,	sys_setxattr,		"setxattr"	}, /* 1217 */
	{ 5,	TF,	sys_setxattr,		"lsetxattr"	}, /* 1218 */
	{ 5,	TD,	sys_setxattr,		"fsetxattr"	}, /* 1219 */
	{ 4,	TF,	sys_getxattr,		"getxattr"	}, /* 1220 */
	{ 4,	TF,	sys_getxattr,		"lgetxattr"	}, /* 1221 */
	{ 4,	TD,	sys_getxattr,		"fgetxattr"	}, /* 1222 */
	{ 3,	TF,	sys_listxattr,		"listxattr"	}, /* 1223 */
	{ 3,	TF,	sys_listxattr,		"llistxattr"	}, /* 1224 */
	{ 3,	TD,	sys_listxattr,		"flistxattr"	}, /* 1225 */
	{ 2,	TF,	sys_removexattr,	"removexattr"	}, /* 1226 */
	{ 2,	TF,	sys_removexattr,	"lremovexattr"	}, /* 1227 */
	{ 2,	TD,	sys_removexattr,	"fremovexattr"	}, /* 1228 */
	{ 2,	TS,	sys_kill,		"tkill"		}, /* 1229 */
	{ 6,	0,	sys_futex,		"futex"		}, /* 1230 */
	{ 3,	0,	sys_sched_setaffinity,	"sched_setaffinity"},/* 1231 */
	{ 3,	0,	sys_sched_getaffinity,	"sched_getaffinity"},/* 1232 */
	{ 1,	0,	sys_set_tid_address,	"set_tid_address"}, /* 1233 */
	{ 5,	TD,	sys_fadvise64,		"fadvise64"	}, /* 1234 */
	{ 3,	TS,	sys_tgkill,		"tgkill"	}, /* 1235 */
	{ 1,	TP,	sys_exit,		"exit_group"	}, /* 1236 */
	{ 4,	0,	sys_lookup_dcookie,	"lookup_dcookie"}, /* 1237 */
	{ 2,	0,	sys_io_setup,		"io_setup"	}, /* 1238 */
	{ 1,	0,	sys_io_destroy,		"io_destroy"	}, /* 1239 */
	{ 5,	0,	sys_io_getevents,		"io_getevents"	}, /* 1240 */
	{ 3,	0,	sys_io_submit,		"io_submit"	}, /* 1241 */
	{ 3,	0,	sys_io_cancel,		"io_cancel"	}, /* 1242 */
	{ 1,	TD,	sys_epoll_create,	"epoll_create"	}, /* 1243 */
	{ 4,	TD,	sys_epoll_ctl,		"epoll_ctl"	}, /* 1244 */
	{ 4,	TD,	sys_epoll_wait,		"epoll_wait"	}, /* 1245 */
	{ 0,	0,	sys_restart_syscall,	"restart_syscall"}, /* 1246 */
	{ 5,	TI,	sys_semtimedop,		"semtimedop"	}, /* 1247 */
	{ 3,	0,	sys_timer_create,	"timer_create"	}, /* 1248 */
	{ 4,	0,	sys_timer_settime,	"timer_settime"	}, /* 1249 */
	{ 2,	0,	sys_timer_gettime,	"timer_gettime"	}, /* 1250 */
	{ 1,	0,	sys_timer_getoverrun,	"timer_getoverrun"}, /* 1251 */
	{ 1,	0,	sys_timer_delete,	"timer_delete"	}, /* 1252 */
	{ 2,	0,	sys_clock_settime,	"clock_settime"	}, /* 1253 */
	{ 2,	0,	sys_clock_gettime,	"clock_gettime"	}, /* 1254 */
	{ 2,	0,	sys_clock_getres,	"clock_getres"	}, /* 1255 */
	{ 4,	0,	sys_clock_nanosleep,	"clock_nanosleep"}, /* 1256 */
	{ MA,	0,	printargs,		"fstatfs64"	}, /* 1257 */
	{ MA,	0,	printargs,		"statfs64"	}, /* 1258 */
	{ 6,	0,	sys_mbind,		"mbind"		}, /* 1259 */
	{ 5,	0,	sys_get_mempolicy,	"get_mempolicy"	}, /* 1260 */
	{ 3,	0,	sys_set_mempolicy,	"set_mempolicy"	}, /* 1261 */
	{ 4,	0,	sys_mq_open,		"mq_open"	}, /* 1262 */
	{ 1,	0,	sys_mq_unlink,		"mq_unlink"	}, /* 1263 */
	{ 5,	0,	sys_mq_timedsend,	"mq_timedsend"	}, /* 1264 */
	{ 5,	0,	sys_mq_timedreceive,	"mq_timedreceive" }, /* 1265 */
	{ 2,	0,	sys_mq_notify,		"mq_notify"	}, /* 1266 */
	{ 3,	0,	sys_mq_getsetattr,	"mq_getsetattr"	}, /* 1267 */
	{ 4,	0,	sys_kexec_load,		"kexec_load"	}, /* 1268 */
	{ 5,	0,	sys_vserver,		"vserver"	}, /* 1269 */
	{ 5,	TP,	sys_waitid,		"waitid"	}, /* 1270 */
	{ 5,	0,	sys_add_key,		"add_key"	}, /* 1271 */
	{ 4,	0,	sys_request_key,	"request_key"	}, /* 1272 */
	{ 5,	0,	sys_keyctl,		"keyctl"	}, /* 1273 */
	{ 3,	0,	sys_ioprio_set,		"ioprio_set"	}, /* 1274 */
	{ 2,	0,	sys_ioprio_get,		"ioprio_get"	}, /* 1275 */
	{ 6,	0,	sys_move_pages,		"move_pages"	}, /* 1276 */
	{ 0,	TD,	sys_inotify_init,	"inotify_init"	}, /* 1277 */
	{ 3,	TD,	sys_inotify_add_watch,	"inotify_add_watch" }, /* 1278 */
	{ 2,	TD,	sys_inotify_rm_watch,	"inotify_rm_watch" }, /* 1279 */
	{ 4,	0,	sys_migrate_pages,	"migrate_pages"	}, /* 1280 */
	{ 4,	TD|TF,	sys_openat,		"openat"	}, /* 1281 */
	{ 3,	TD|TF,	sys_mkdirat,		"mkdirat"	}, /* 1282 */
	{ 4,	TD|TF,	sys_mknodat,		"mknodat"	}, /* 1283 */
	{ 5,	TD|TF,	sys_fchownat,		"fchownat"	}, /* 1284 */
	{ 3,	TD|TF,	sys_futimesat,		"futimesat"	}, /* 1285 */
	{ 4,	TD|TF,	sys_newfstatat,		"newfstatat"	}, /* 1286 */
	{ 3,	TD|TF,	sys_unlinkat,		"unlinkat"	}, /* 1287 */
	{ 4,	TD|TF,	sys_renameat,		"renameat"	}, /* 1288 */
	{ 5,	TD|TF,	sys_linkat,		"linkat"	}, /* 1289 */
	{ 3,	TD|TF,	sys_symlinkat,		"symlinkat"	}, /* 1290 */
	{ 4,	TD|TF,	sys_readlinkat,		"readlinkat"	}, /* 1291 */
	{ 3,	TD|TF,	sys_fchmodat,		"fchmodat"	}, /* 1292 */
	{ 3,	TD|TF,	sys_faccessat,		"faccessat"	}, /* 1293 */
	{ 6,	TD,	sys_pselect6,		"pselect6"	}, /* 1294 */
	{ 5,	TD,	sys_ppoll,		"ppoll"		}, /* 1295 */
	{ 1,	TP,	sys_unshare,		"unshare"	}, /* 1296 */
	{ 2,	0,	sys_set_robust_list,	"set_robust_list" }, /* 1297 */
	{ 3,	0,	sys_get_robust_list,	"get_robust_list" }, /* 1298 */
	{ 6,	TD,	sys_splice,		"splice"	}, /* 1299 */
	{ 4,	TD,	sys_sync_file_range,	"sync_file_range" }, /* 1300 */
	{ 4,	TD,	sys_tee,		"tee"		}, /* 1301 */
	{ 4,	TD,	sys_vmsplice,		"vmsplice"	}, /* 1302 */
	{ MA,	0,	printargs,		"SYS_1303"	}, /* 1303 */
	{ 3,	0,	sys_getcpu,		"getcpu"	}, /* 1304 */
	{ 6,	TD,	sys_epoll_pwait,	"epoll_pwait"	}, /* 1305 */
	{ MA,	0,	printargs,		"SYS_1306"	}, /* 1306 */
	{ 3,	TD|TS,	sys_signalfd,		"signalfd"	}, /* 1307 */
	{ 4,	TD,	sys_timerfd,		"timerfd"	}, /* 1308 */
	{ 1,	TD,	sys_eventfd,		"eventfd"	}, /* 1309 */
	{ 5,	TD,	sys_preadv,		"preadv"	}, /* 1319 */
	{ 5,	TD,	sys_pwritev,		"pwritev"	}, /* 1320 */
	{ 4,	TP|TS,	sys_rt_tgsigqueueinfo,	"rt_tgsigqueueinfo"}, /* 1321 */
	{ 5,	TN,	sys_recvmmsg,		"recvmmsg"	}, /* 1322 */
	{ 2,	TD,	sys_fanotify_init,	"fanotify_init"	}, /* 1323 */
	{ 5,	TD|TF,	sys_fanotify_mark,	"fanotify_mark"	}, /* 1324 */
	{ 4,	0,	sys_prlimit64,		"prlimit64"	}, /* 1325 */
	{ 5,	TD|TF,	sys_name_to_handle_at,	"name_to_handle_at"}, /* 1326 */
	{ 3,	TD,	sys_open_by_handle_at,	"open_by_handle_at"}, /* 1327 */
	{ 2,	0,	sys_clock_adjtime,	"clock_adjtime"	}, /* 1328 */
	{ 1,	TD,	sys_syncfs,		"syncfs"	}, /* 1329 */
	{ 2,	TD,	sys_setns,		"setns"		}, /* 1330 */
	{ 4,	TN,	sys_sendmmsg,		"sendmmsg"	}, /* 1331 */
	{ 6,	0,	sys_process_vm_readv,	"process_vm_readv"	}, /* 1332 */
	{ 6,	0,	sys_process_vm_writev,	"process_vm_writev"	}, /* 1333 */
	{ 4,	TN,	sys_accept4,		"accept4"	}, /* 1334 */
