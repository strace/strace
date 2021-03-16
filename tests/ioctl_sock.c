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

static void
test_int(const int fd)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(int, p_int);
	pid_t pid = getpid();

	const struct {
		uint32_t cmd;
		const char *str;
		int val;
	} cmd[] = {
		{ ARG_STR(FIOGETOWN), -1 },
		{ ARG_STR(FIOSETOWN), pid },
		{ ARG_STR(SIOCATMARK), -1 },
		{ ARG_STR(SIOCGPGRP), -1 },
		{ ARG_STR(SIOCSPGRP), pid },
		{ ARG_STR(SIOCSIFENCAP), -1 },
	};

	for (size_t i = 0; i < ARRAY_SIZE(cmd); ++i) {
		*p_int = cmd[i].val;

		do_ioctl_ptr(fd, cmd[i].cmd, p_int + 1);
		printf("ioctl(%d, %s, %p) = %s\n",
		       fd, cmd[i].str, p_int + 1, errstr);

		do_ioctl_ptr(fd, cmd[i].cmd, p_int);
		printf("ioctl(%d, %s, [%d]) = %s\n",
		       fd, cmd[i].str, *p_int, errstr);
	}
}

static void
test_str(void)
{
	char *const buf = tail_alloc(IFNAMSIZ);

	static const struct {
		uint32_t cmd;
		const char *str;
	} cmd[] = {
		{ ARG_STR(SIOCBRADDBR) },
		{ ARG_STR(SIOCBRDELBR) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(cmd); ++i) {
		fill_memory_ex(buf, IFNAMSIZ, '0', 10);
		do_ioctl_ptr(-1, cmd[i].cmd, buf + 1);
		printf("ioctl(%d, %s, %p) = %s\n",
		       -1, cmd[i].str, buf + 1, errstr);

		do_ioctl_ptr(-1, cmd[i].cmd, buf);
		printf("ioctl(%d, %s, \"%.*s\"...) = %s\n",
		       -1, cmd[i].str, IFNAMSIZ, buf, errstr);

		buf[IFNAMSIZ - 1] = '\0';
		do_ioctl_ptr(-1, cmd[i].cmd, buf);
		printf("ioctl(%d, %s, \"%s\") = %s\n",
		       -1, cmd[i].str, buf, errstr);
	}
}

static void
test_ifreq(const int fd)
{
	TAIL_ALLOC_OBJECT_CONST_PTR(struct ifreq, ifr);

#define SRC_IFR_INT(name, val0, val1)					\
	static const struct ifreq src_ ## name[] = {			\
		{ .name = val0 },					\
		{ .name = val1 }					\
	};								\
	char str0_ ## name[sizeof(int) * 3];				\
	snprintf(str0_ ## name, sizeof(str0_ ## name), "%d", val0);	\
	char str1_ ## name[sizeof(int) * 3];				\
	snprintf(str1_ ## name, sizeof(str1_ ## name), "%d", val1);	\
	const char *const str_ ## name[] = {				\
		str0_ ## name,						\
		str1_ ## name						\
	}								\
/* End of SRC_IFR_INT definition.  */

	SRC_IFR_INT(ifr_metric, 0x1defaced, 0xfacefed1);
	SRC_IFR_INT(ifr_mtu, 0x2defaced, 0xfacefed2);
	SRC_IFR_INT(ifr_qlen, 0x3defaced, 0xfacefed3);

#define SRC_IFR_FLAG(name, valid_flags,   valid_str,			\
			 invalid_flags, invalid_str)			\
	static const struct ifreq src_ ## name[] = {			\
		{ .name = valid_flags },				\
		{ .name = invalid_flags }				\
	};								\
	static const char *const str_ ## name[] = {			\
		valid_str,						\
		invalid_str,						\
	}								\
/* End of SRC_IFR_FLAG definition.  */

	SRC_IFR_FLAG(ifr_flags,
		     0xffff, XLAT_KNOWN(0xffff,
					"IFF_UP|"
					"IFF_BROADCAST|"
					"IFF_DEBUG|"
					"IFF_LOOPBACK|"
					"IFF_POINTOPOINT|"
					"IFF_NOTRAILERS|"
					"IFF_RUNNING|"
					"IFF_NOARP|"
					"IFF_PROMISC|"
					"IFF_ALLMULTI|"
					"IFF_MASTER|"
					"IFF_SLAVE|"
					"IFF_MULTICAST|"
					"IFF_PORTSEL|"
					"IFF_AUTOMEDIA|"
					"IFF_DYNAMIC"),
		     0, "0");

#define SRC_IFR_STRING(name)						\
	struct ifreq src_ ## name[2];					\
	fill_memory_ex(src_ ## name[0].name,				\
		       sizeof(src_ ## name[0].name),			\
		       'a', 'z' - 'a' + 1);				\
	memcpy(src_ ## name[1].name, src_ ## name[0].name,		\
	       sizeof(src_ ## name[1].name) - 1);			\
	src_ ## name[1].name[sizeof(src_ ## name[1].name) - 1] = '\0';	\
	char str0_ ## name[sizeof(src_ ## name[0].name) + 5];		\
	snprintf(str0_ ## name, sizeof(str0_ ## name),			\
		 "\"%.*s\"...",						\
		 (int) sizeof(src_ ## name[0].name) - 1,		\
		 src_ ## name[0].name);					\
	char str1_ ## name[sizeof(src_ ## name[1].name) + 2];		\
	snprintf(str1_ ## name, sizeof(str1_ ## name),			\
		 "\"%s\"", src_ ## name[1].name);			\
	const char *const str_ ## name[] = {				\
		str0_ ## name,						\
		str1_ ## name						\
	}								\
/* End of SRC_IFR_STRING definition.  */

	SRC_IFR_STRING(ifr_newname);
	SRC_IFR_STRING(ifr_slave);

#define SRC_IFR_ADDR(name, port)					\
	const struct sockaddr_in src_in_ ## name = {			\
		.sin_family = AF_INET,					\
		.sin_addr.s_addr = htonl(INADDR_LOOPBACK),		\
		.sin_port = htons(port)					\
	};								\
	struct ifreq src_ ## name[2];					\
	memcpy(&src_ ## name[0].name, &src_in_ ## name,			\
	       sizeof(src_in_ ## name));				\
	memset(&src_ ## name[1], 0, sizeof(src_ ## name[1]));		\
	char str_in_ ## name[256];					\
	snprintf(str_in_ ## name, sizeof(str_in_ ## name),		\
		 "{sa_family=AF_INET, sin_port=htons(%u)"		\
		 ", sin_addr=inet_addr(\"%s\")}",			\
		 ntohs((src_in_ ## name).sin_port),			\
		 inet_ntoa((src_in_ ## name).sin_addr));		\
	const char *const str_ ## name[] = {				\
		str_in_ ## name, NULL					\
	}								\
/* End of SRC_IFR_ADDR definition.  */

	SRC_IFR_ADDR(ifr_addr, 0x1bad);
	SRC_IFR_ADDR(ifr_dstaddr, 0x2bad);
	SRC_IFR_ADDR(ifr_broadaddr, 0x3bad);
	SRC_IFR_ADDR(ifr_netmask, 0x4bad);

#define SRC_IFR_HWADDR(name)						\
	struct ifreq src_ ## name[2];					\
	fill_memory(src_ ## name, sizeof(src_ ## name));		\
	src_ ## name[0].name.sa_family = 1;				\
	char str_hw_ ## name[256];					\
	snprintf(str_hw_ ## name, sizeof(str_hw_ ## name),		\
		 "{sa_family=ARPHRD_ETHER"				\
		 ", sa_data=%02x:%02x:%02x:%02x:%02x:%02x}",		\
		 (uint8_t) src_ ## name[0].name.sa_data[0],		\
		 (uint8_t) src_ ## name[0].name.sa_data[1],		\
		 (uint8_t) src_ ## name[0].name.sa_data[2],		\
		 (uint8_t) src_ ## name[0].name.sa_data[3],		\
		 (uint8_t) src_ ## name[0].name.sa_data[4],		\
		 (uint8_t) src_ ## name[0].name.sa_data[5]);		\
	const char *const str_ ## name[] = {				\
		str_hw_ ## name, NULL					\
	}								\
/* End of SRC_IFR_HWADDR definition.  */

	SRC_IFR_HWADDR(ifr_hwaddr);

#define SRC_IFR_MAP(name)						\
	struct ifreq src_ ## name[2];					\
	fill_memory(src_ ## name, sizeof(src_ ## name));		\
	char str_map_ ## name[256];					\
	snprintf(str_map_ ## name, sizeof(str_map_ ## name),		\
		 "{mem_start=%#lx, mem_end=%#lx, base_addr=%#x"		\
		 ", irq=%#x, dma=%#x, port=%#x}",			\
		 src_ ## name[0].name.mem_start,			\
		 src_ ## name[0].name.mem_end,				\
		 src_ ## name[0].name.base_addr,			\
		 src_ ## name[0].name.irq,				\
		 src_ ## name[0].name.dma,				\
		 src_ ## name[0].name.port);				\
	const char *const str_ ## name[] = {				\
		str_map_ ## name, NULL					\
	}								\
/* End of SRC_IFR_MAP definition.  */

	SRC_IFR_MAP(ifr_map);

	const struct {
		const uint32_t cmd;
		const char *const str;
		const char *const name;
		const struct ifreq src_addr[2];
		const char *const src_str[2];
	} cmd[] = {
#define ARG_IFREQ(name)				\
	#name,					\
	{ src_ ## name[0], src_ ## name[1]},	\
	{ str_ ## name[0], str_ ## name[1]},	\
/* End of ARG_IFREQ definition.  */

		{ ARG_STR(SIOCSIFADDR),		ARG_IFREQ(ifr_addr) },
		{ ARG_STR(SIOCSIFBRDADDR),	ARG_IFREQ(ifr_broadaddr) },
		{ ARG_STR(SIOCSIFDSTADDR),	ARG_IFREQ(ifr_dstaddr) },
		{ ARG_STR(SIOCSIFFLAGS),	ARG_IFREQ(ifr_flags) },
		{ ARG_STR(SIOCSIFHWADDR),	ARG_IFREQ(ifr_hwaddr) },
		{ ARG_STR(SIOCSIFHWBROADCAST),	ARG_IFREQ(ifr_hwaddr) },
		{ ARG_STR(SIOCADDMULTI),	ARG_IFREQ(ifr_hwaddr) },
		{ ARG_STR(SIOCDELMULTI),	ARG_IFREQ(ifr_hwaddr) },
		{ ARG_STR(SIOCSIFMAP),		ARG_IFREQ(ifr_map) },
		{ ARG_STR(SIOCSIFMETRIC),	ARG_IFREQ(ifr_metric) },
		{ ARG_STR(SIOCSIFMTU),		ARG_IFREQ(ifr_mtu) },
		{ ARG_STR(SIOCSIFNAME),		ARG_IFREQ(ifr_newname) },
		{ ARG_STR(SIOCSIFNETMASK),	ARG_IFREQ(ifr_netmask) },
		{ ARG_STR(SIOCSIFSLAVE),	ARG_IFREQ(ifr_slave) },
		{ ARG_STR(SIOCSIFTXQLEN),	ARG_IFREQ(ifr_qlen) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(cmd); ++i) {
		do_ioctl_ptr(-1, cmd[i].cmd, (char *) ifr + 1);
		printf("ioctl(%d, %s, %p) = %s\n",
		       -1, cmd[i].str, (char *) ifr + 1, errstr);

		for (size_t j = 0; j < ARRAY_SIZE(cmd[i].src_addr); ++j) {
			if (!cmd[i].src_str[j])
				continue;
			memcpy(ifr, &cmd[i].src_addr[j], sizeof(*ifr));
			fill_memory_ex(ifr->ifr_name, sizeof(ifr->ifr_name),
				       '0', 10);
			do_ioctl_ptr(-1, cmd[i].cmd, ifr);
			printf("ioctl(%d, %s, {ifr_name=\"%.*s\"..., %s=%s})"
			       " = %s\n",
			       -1, cmd[i].str, IFNAMSIZ - 1, ifr->ifr_name,
			       cmd[i].name, cmd[i].src_str[j], errstr);

			ifr->ifr_name[IFNAMSIZ - 1] = '\0';
			do_ioctl_ptr(-1, cmd[i].cmd, ifr);
			printf("ioctl(%d, %s, {ifr_name=\"%s\", %s=%s}) = %s\n",
			       -1, cmd[i].str, ifr->ifr_name,
			       cmd[i].name, cmd[i].src_str[j], errstr);
		}
	}

	memset(ifr, 0, sizeof(*ifr));
	strcpy(ifr->ifr_name, "lo");
	if (do_ioctl_ptr(fd, SIOCGIFINDEX, ifr)) {
		printf("ioctl(%d, %s, {ifr_name=\"%s\"}) = %s\n",
		       fd, "SIOCGIFINDEX", ifr->ifr_name, errstr);
		ifr->ifr_ifindex = 1;
	} else {
		printf("ioctl(%d, %s, {ifr_name=\"%s\", ifr_ifindex=%d})"
		       " = %s\n",
		       fd, "SIOCGIFINDEX", ifr->ifr_name, ifr->ifr_ifindex,
		       errstr);
	}

	int ifindex = ifr->ifr_ifindex;

	do_ioctl_ptr(-1, SIOCGIFNAME, ifr);
	printf("ioctl(%d, %s, {ifr_ifindex=%d}) = %s\n",
	       -1, "SIOCGIFNAME", ifr->ifr_ifindex, errstr);

	if (do_ioctl_ptr(fd, SIOCGIFNAME, ifr))
		printf("ioctl(%d, %s, {ifr_ifindex=%d}) = %s\n",
		       fd, "SIOCGIFNAME", ifr->ifr_ifindex, errstr);
	else
		printf("ioctl(%d, %s, {ifr_ifindex=%d, ifr_name=\"%s\"})"
		       " = %s\n",
		       fd, "SIOCGIFNAME", ifr->ifr_ifindex, ifr->ifr_name,
		       errstr);

	static const struct {
		const uint32_t cmd;
		const char *const str;
	} ifindex_cmd[] = {
		{ ARG_STR(SIOCBRADDIF) },
		{ ARG_STR(SIOCBRDELIF) },
	};

	for (size_t i = 0; i < ARRAY_SIZE(ifindex_cmd); ++i) {
		ifr->ifr_ifindex = ifindex;
		do_ioctl_ptr(-1, ifindex_cmd[i].cmd, ifr);
		printf("ioctl(%d, %s, {ifr_ifindex=%s}) = %s\n",
		       -1, ifindex_cmd[i].str, IFINDEX_LO_STR, errstr);
	}
}

int
main(void)
{
	const int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0)
		perror_msg_and_skip("socket AF_INET");

	test_ptr();
	test_int(fd);
	test_str();
	test_ifreq(fd);

	puts("+++ exited with 0 +++");
	return 0;
}
