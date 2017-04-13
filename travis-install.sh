#!/bin/sh -ex

updated=
apt_get_install()
{
	[ -n "$updated" ] || {
		sudo apt-get -qq update
		updated=1
	}
	sudo apt-get -qq --no-install-suggests --no-install-recommends \
		install -y "$@"
}

case "$KHEADERS" in
	*/*)
		git clone --depth=1 https://github.com/"$KHEADERS" kernel
		sudo make -C kernel headers_install INSTALL_HDR_PATH=/opt/kernel
		sudo rm -rf kernel
		KHEADERS_INC=/opt/kernel/include
		;;
	*)
		KHEADERS_INC=/usr/include
		;;
esac

case "$CC" in
	gcc)
		apt_get_install gcc-multilib
		;;
	gcc-*)
		sudo add-apt-repository -y ppa:ubuntu-toolchain-r/test
		apt_get_install gcc-multilib "$CC"-multilib
		;;
	clang-*)
		apt_get_install gcc-multilib "$CC"
		;;
	musl-gcc)
		apt_get_install gcc-multilib
		git clone --depth=1 https://github.com/strace/musl
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
			sudo make install
		cd -
		rm -rf musl
		sudo ln -s \
			$KHEADERS_INC/asm* \
			$KHEADERS_INC/linux \
			$KHEADERS_INC/mtd \
			/opt/musl/include/
		;;
esac

case "${CHECK-}" in
	coverage)
		apt_get_install lcov
		pip install --user codecov
		;;
esac
