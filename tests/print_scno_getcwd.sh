#! /bin/sh -efu
#
# Print the syscall number corresponding to getcwd syscall.
#
# Copyright (c) 2022 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

case "$STRACE_ARCH" in
	alpha)	echo 367 ;;
	arm|bfin|i386|m68k|microblaze|s390*|sh|sh64)
		echo 183 ;;
	avr32)	echo 48 ;;
	hppa)	echo 110 ;;
	ia64)	echo 1184 ;;
	mips)	case "$MIPS_ABI" in
			n64) echo 5077 ;;
			n32) echo 6077 ;;
			o32) echo 4203 ;;
		esac ;;
	powerpc*)
		echo 182 ;;
	sparc*)
		echo 119 ;;
	x32|x86_64)
		echo 79 ;;
	xtensa)	echo 43 ;;
	*)	echo 17 ;;
esac
