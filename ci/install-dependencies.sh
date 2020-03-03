#!/bin/sh -ex
#
# Copyright (c) 2018-2020 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: GPL-2.0-or-later

j=-j`nproc` || j=
type sudo >/dev/null 2>&1 && sudo=sudo || sudo=
common_packages='autoconf automake faketime file gawk git gzip libbluetooth-dev make xz-utils'

retry_if_failed()
{
	for i in `seq 0 99`; do
		"$@" && i= && break || sleep 1
	done
	[ -z "$i" ]
}

updated=
apt_get_install()
{
	[ -n "$updated" ] || {
		retry_if_failed $sudo apt-get -qq update
		updated=1
	}
	retry_if_failed $sudo \
		apt-get -qq --no-install-suggests --no-install-recommends \
		install -y "$@"
}

git_installed=
clone_repo()
{
	local src dst branch
	src="$1"; shift
	dst="$1"; shift
	branch="${1-}"

	[ -n "$git_installed" ] || {
		apt_get_install git ca-certificates
		git_installed=1
	}

	case "$src" in
		*://*)	;;
		*)	local url path
			url="$(git config remote.origin.url)"
			path="${url#*://*/}"
			src="${url%$path}$src"
			;;
	esac

	retry_if_failed \
		git clone --depth=1 ${branch:+--branch $branch} "$src" "$dst"
}

case "$TARGET" in
	aarch64)
		packages="$common_packages gcc-multilib-arm-linux-gnueabihf libc6-dev-armhf-cross linux-libc-dev-armhf-cross"
		;;
	x86_64|x32|x86|s390x)
		packages="$common_packages gcc-multilib"
		;;
	*)
		packages="$common_packages gcc"
		;;
esac

case "$CC" in
	gcc-*)
		retry_if_failed \
			$sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
		case "$TARGET" in
			aarch64)
				apt_get_install $packages "$CC"-multilib-arm-linux-gnueabihf "$CC"
				;;
			x86_64|x32|x86|s390x)
				apt_get_install $packages "$CC"-multilib "$CC"
				;;
			*)
				apt_get_install $packages "$CC"
				;;
		esac
		;;
	clang*)
		apt_get_install $packages "$CC"
		;;
	*)
		apt_get_install $packages
		;;
esac

case "$KHEADERS" in
	*/*)
		clone_repo https://github.com/"$KHEADERS" kernel ${KBRANCH-}
		apt_get_install rsync
		$sudo make $j -C kernel headers_install INSTALL_HDR_PATH=/opt/kernel
		$sudo rm -rf kernel
		KHEADERS_INC=/opt/kernel/include
		;;
	*)
		KHEADERS_INC=/usr/include
		;;
esac

case "$CC" in
	musl-gcc)
		clone_repo strace/musl musl esyr/mpers
		cd musl
			CC=gcc
			build=
			case "${TARGET-}" in
				x32)
					CC="$CC -mx32"
					;;
				x86)
					CC="$CC -m32"
					build='--build=i686-pc-linux-gnu --target=i686-pc-linux-gnu'
					;;
			esac
			./configure --prefix=/opt/musl --exec-prefix=/usr ${build}
			make $j
			$sudo make $j install
		cd -
		rm -rf musl
		$sudo ln -s \
			$KHEADERS_INC/asm* \
			$KHEADERS_INC/linux \
			$KHEADERS_INC/mtd \
			/opt/musl/include/
		;;
esac

case "${STACKTRACE-}" in
	libdw)
		apt_get_install libdw-dev libiberty-dev
		;;
	libunwind)
		apt_get_install libunwind8-dev libiberty-dev
		;;
esac

case "${CHECK-}" in
	coverage)
		apt_get_install lcov python-pip python-setuptools
		retry_if_failed \
			pip install --user codecov
		;;
	valgrind)
		apt_get_install valgrind
		;;
esac
