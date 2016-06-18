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
		sudo add-apt-repository ppa:bortis/musl -y
		apt_get_install gcc-multilib musl-tools linux-musl-dev
		;;
esac

if [ "${COVERAGE-}" = true ]; then
	apt_get_install lcov
	pip install --user codecov
fi
