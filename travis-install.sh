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

case "$CC" in
	gcc)
		apt_get_install gcc-multilib
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
			/usr/include/linux \
			/usr/include/asm \
			/usr/include/asm-generic \
			/usr/include/mtd \
			/opt/musl/include/
		;;
esac

if [ "${COVERAGE-}" = true ]; then
	apt_get_install lcov
	pip install --user codecov
fi
