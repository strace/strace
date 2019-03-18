/*
 * Copyright (c) 2018-2019 The strace developers.
 * All rights reserved.
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#define ADDR_FAMILY_FIELD sin6_family
#define ADDR_FAMILY AF_INET6
#define AF_STR "AF_INET6"
#define LOOPBACK_FIELD .sin6_addr = IN6ADDR_LOOPBACK_INIT
#define LOOPBACK "[::1]"
#define SOCKADDR_TYPE sockaddr_in6
#define TCP_STR "TCPv6"
#define INPORT sin6_port
#define INPORT_STR "sin6_port"
#define INADDR_STR "sin6_flowinfo=htonl(0)" \
	", inet_pton(AF_INET6, \"::1\", &sin6_addr)"
#define SA_FIELDS ", sin6_scope_id=0"

#include "net-yy-inet.c"
