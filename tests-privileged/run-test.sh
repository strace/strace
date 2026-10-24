#!/bin/sh -eu
#
# Copyright (c) 2026 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

tests_srcdir="$1"
shift

for arg do
	test="$arg"
done
name="${test##*/}"
name="${name%.test}"

if [ "${RUN_PRIVILEGED_TESTS-}" != 1 ] ||
   ! command -v sudo > /dev/null 2>&1 ||
   ! sudo -n true > /dev/null 2>&1; then
	exit 77
fi

status=0
sudo -n env srcdir="$tests_srcdir" "$@" || status=$?

if [ -d "$name.dir" ]; then
	sudo -n chown -R "$(id -u):$(id -g)" "$name.dir"
fi

exit "$status"
