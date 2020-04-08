#!/bin/sh
#
# Check verbose decoding of NETLINK_SOCK_DIAG protocol
#
# Copyright (c) 2017-2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

. "${srcdir=.}/init.sh"

run_prog ../netlink_netlink_diag
run_strace_match_diff -v -e trace=sendto
