#!/bin/sh -ex

type sudo >/dev/null 2>&1 && sudo=sudo || sudo=
common_packages='autoconf automake faketime file gawk gcc-multilib git gzip libbluetooth-dev make xz-utils'

updated=
apt_get_install()
{
	[ -n "$updated" ] || {
		$sudo apt-get -qq update
		updated=1
	}
	$sudo apt-get -qq --no-install-suggests --no-install-recommends \
		install -y "$@"
}

git_installed=
clone_repo()
{
	local src dst
	src="$1"; shift
	dst="$1"; shift

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

	git clone --depth=1 "$src" "$dst"
}

case "$KHEADERS" in
	*/*)
		clone_repo https://github.com/"$KHEADERS" kernel
		apt_get_install $common_packages
		$sudo make -C kernel headers_install INSTALL_HDR_PATH=/opt/kernel
		$sudo rm -rf kernel
		KHEADERS_INC=/opt/kernel/include
		;;
	*)
		KHEADERS_INC=/usr/include
		;;
esac

case "$CC" in
	gcc)
		apt_get_install $common_packages
		;;
	gcc-*)
		$sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
		apt_get_install $common_packages "$CC"-multilib
		;;
	clang*)
		apt_get_install $common_packages "$CC"
		;;
	musl-gcc)
		clone_repo strace/musl musl
		apt_get_install $common_packages
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
			make
			$sudo make install
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
		apt_get_install libdw-dev
		;;
	libunwind)
		apt_get_install libunwind8-dev
		;;
esac

case "${CHECK-}" in
	coverage)
		apt_get_install lcov
		pip install --user codecov
		;;
	valgrind)
		apt_get_install valgrind
		;;
esac
