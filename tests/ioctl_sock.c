/*
 * Check decoding of SIOC* ioctl commands.
 *
 * Copyright (c) 2020-2021 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "tests.h"
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <linux/sockios.h>

static const char *errstr;

static int
do_ioctl(int fd, kernel_ulong_t cmd, kernel_ulong_t arg)
{
	int rc = ioctl(fd, cmd, arg);
	errstr = sprintrc(rc);

	return rc;
}

static int
do_ioctl_ptr(int fd, kernel_ulong_t cmd, const void *arg)
{
	return do_ioctl(fd, cmd, (uintptr_t) arg);
}

static void
test_ptr(void)
{
	const char *const efault = tail_alloc(1) + 1;

	static const struct {
		uint32_t cmd;
		const char *str;
	} cmd[] = {
		{ ARG_STR(FIOSETOWN) },
		{ ARG_STR(SIOCSPGRP) },
		{ ARG_STR(FIOGETOWN) },
		{ ARG_STR(SIOCGPGRP) },
		{ ARG_STR(SIOCATMARK) },
#ifdef SIOCGSTAMP_OLD
		{ ARG_STR(SIOCGSTAMP_OLD) },
#endif
#ifdef SIOCGSTAMP_NEW
		{ ARG_STR(SIOCGSTAMP_NEW) },
#endif
#ifdef SIOCGSTAMPNS_OLD
		{ ARG_STR(SIOCGSTAMPNS_OLD) },
#endif
#ifdef SIOCGSTAMPNS_NEW
		{ ARG_STR(SIOCGSTAMPNS_NEW) },
#endif
		{ ARG_STR(SIOCADDRT) },
		{ ARG_STR(SIOCDELRT) },
		{ ARG_STR(SIOCRTMSG) },
		{ ARG_STR(SIOCGIFNAME) },
		{ ARG_STR(SIOCSIFLINK) },
		{ ARG_STR(SIOCGIFCONF) },
		{ ARG_STR(SIOCGIFFLAGS) },
		{ ARG_STR(SIOCSIFFLAGS) },
		{ ARG_STR(SIOCGIFADDR) },
		{ ARG_STR(SIOCSIFADDR) },
		{ ARG_STR(SIOCGIFDSTADDR) },
		{ ARG_STR(SIOCSIFDSTADDR) },
		{ ARG_STR(SIOCGIFBRDADDR) },
		{ ARG_STR(SIOCSIFBRDADDR) },
		{ ARG_STR(SIOCGIFNETMASK) },
		{ ARG_STR(SIOCSIFNETMASK) },
		{ ARG_STR(SIOCGIFMETRIC) },
		{ ARG_STR(SIOCSIFMETRIC) },
		{ ARG_STR(SIOCGIFMEM) },
		{ ARG_STR(SIOCSIFMEM) },
		{ ARG_STR(SIOCGIFMTU) },
		{ ARG_STR(SIOCSIFMTU) },
		{ ARG_STR(SIOCSIFNAME) },
		{ ARG_STR(SIOCSIFHWADDR) },
		{ ARG_STR(SIOCGIFENCAP) },
		{ ARG_STR(SIOCSIFENCAP) },
		{ ARG_STR(SIOCGIFHWADDR) },
		{ ARG_STR(SIOCGIFSLAVE) },
		{ ARG_STR(SIOCSIFSLAVE) },
		{ ARG_STR(SIOCADDMULTI) },
		{ ARG_STR(SIOCDELMULTI) },
		{ ARG_STR(SIOCGIFINDEX) },
		{ ARG_STR(SIOCSIFPFLAGS) },
		{ ARG_STR(SIOCGIFPFLAGS) },
		{ ARG_STR(SIOCDIFADDR) },
		{ ARG_STR(SIOCSIFHWBROADCAST) },
		{ ARG_STR(SIOCGIFCOUNT) },
		{ ARG_STR(SIOCGIFBR) },
		{ ARG_STR(SIOCSIFBR) },
		{ ARG_STR(SIOCGIFTXQLEN) },
		{ ARG_STR(SIOCSIFTXQLEN) },
		{ ARG_STR(SIOCETHTOOL) },
		{ ARG_STR(SIOCGMIIPHY) },
		{ ARG_STR(SIOCGMIIREG) },
		{ ARG_STR(SIOCSMIIREG) },
		{ ARG_STR(SIOCWANDEV) },
#ifdef SIOCOUTQNSD
		{ ARG_STR(SIOCOUTQNSD) },
#endif
#ifdef SIOCGSKNS
		{ ARG_STR(SIOCGSKNS) },
#endif
		{ ARG_STR(SIOCDARP) },
		{ ARG_STR(SIOCGARP) },
		{ ARG_STR(SIOCSARP) },
		{ ARG_STR(SIOCDRARP) },
		{ ARG_STR(SIOCGRARP) },
		{ ARG_STR(SIOCSRARP) },
		{ ARG_STR(SIOCGIFMAP) },
		{ ARG_STR(SIOCSIFMAP) },
		{ ARG_STR(SIOCADDDLCI) },
		{ ARG_STR(SIOCDELDLCI) },
		{ ARG_STR(SIOCGIFVLAN) },
		{ ARG_STR(SIOCSIFVLAN) },
		{ ARG_STR(SIOCBONDENSLAVE) },
		{ ARG_STR(SIOCBONDRELEASE) },
		{ ARG_STR(SIOCBONDSETHWADDR) },
		{ ARG_STR(SIOCBONDSLAVEINFOQUERY) },
		{ ARG_STR(SIOCBONDINFOQUERY) },
		{ ARG_STR(SIOCBONDCHANGEACTIVE) },
		{ ARG_STR(SIOCBRADDBR) },
		{ ARG_STR(SIOCBRDELBR) },
		{ ARG_STR(SIOCBRADDIF) },
		{ ARG_STR(SIOCBRDELIF) },
#ifdef SIOCSHWTSTAMP
		{ ARG_STR(SIOCSHWTSTAMP) },
#endif
#ifdef SIOCGHWTSTAMP
		{ ARG_STR(SIOCGHWTSTAMP) },
#endif
	},
	unknown_cmd[] = {
		{ _IO(0x89, 0xff), "_IOC(_IOC_NONE, 0x89, 0xff, 0)" }
	};

	for (size_t i = 0; i < ARRAY_SIZE(cmd); ++i) {
		do_ioctl(-1, cmd[i].cmd, 0);
		printf("ioctl(%d, %s, NULL) = %s\n",
		       -1, cmd[i].str, errstr);

		do_ioctl_ptr(-1, cmd[i].cmd, efault);
		printf("ioctl(%d, %s, %p) = %s\n",
		       -1, cmd[i].str, efault, errstr);
	}

	for (size_t i = 0; i < ARRAY_SIZE(unknown_cmd); ++i) {
		do_ioctl(-1, unknown_cmd[i].cmd, 0);
		printf("ioctl(%d, %s, 0) = %s\n",
		       -1, unknown_cmd[i].str, errstr);

		do_ioctl_ptr(-1, unknown_cmd[i].cmd, efault);
		printf("ioctl(%d, %s, %p) = %s\n",
		       -1, unknown_cmd[i].str, efault, errstr);
	}
}

int
main(void)
{
	test_ptr();

	puts("+++ exited with 0 +++");
	return 0;
}
