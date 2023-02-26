#!/bin/sh -e
#
# Copyright (c) 2015 Elvira Khabirova <lineprinter0@gmail.com>
# Copyright (c) 2015-2023 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

[ "x${D:-0}" != x1 ] || set -x

export LC_ALL=C

MPERS_AWK="${0%/*}/mpers.awk"
ARCH_FLAG=$1
CC_ARCH_FLAG=$2
PARSER_FILE=$3

READELF="${READELF:-readelf}"
CC="${CC-gcc}"
CFLAGS="$CFLAGS -gdwarf-2 -c"
CPP="${CPP-$CC -E}"
CPPFLAGS="$CPPFLAGS -MM -MG"

VAR_NAME='mpers_target_var'
BITS_DIR="mpers-${ARCH_FLAG}"

mkdir -p ${BITS_DIR}
set -- $(sed -E -n \
	's/^#[[:space:]]*include[[:space:]]+DEF_MPERS_TYPE\(([^)[:space:]]*)\)$/\1/p' \
		"${PARSER_FILE}")
for m_type; do
	f_h="${BITS_DIR}/${m_type}.h"
	f_c="${BITS_DIR}/${m_type}.c"
	f_i="${BITS_DIR}/${m_type}.i"
	f_o="${BITS_DIR}/${m_type}.o"
	f_d1="${BITS_DIR}/${m_type}.d1"
	f_d2="${BITS_DIR}/${m_type}.d2"
	sed -e '
		/DEF_MPERS_TYPE('"${m_type}"')$/n
		/DEF_MPERS_TYPE/d
		/^[[:space:]]*#[[:space:]]*include[[:space:]]*"xlat\//d
		/^#[[:space:]]*include[[:space:]][[:space:]]*MPERS_DEFS$/ {s//'"${m_type} ${VAR_NAME}"';/;q}
		' "${PARSER_FILE}" > "${f_c}"
	$CPP $CPPFLAGS "${f_c}" > "${f_i}"
	grep -F -q "${m_type}.h" "${f_i}" ||
		continue
	sed -i -e '/DEF_MPERS_TYPE/d' "${f_c}"
	$CC $CFLAGS $CC_ARCH_FLAG "${f_c}" -o "${f_o}"
	$READELF --wide --debug-dump=info "${f_o}" > "${f_d1}"
	sed -E -n '
		/^[[:space:]]*<1>/,/^[[:space:]]*<1><[^>]+>: Abbrev Number: 0/!d
		/^[[:space:]]*<[^>]*><[^>]*>: Abbrev Number: 0/d
		s/^[[:space:]]*<[[:xdigit:]]+>[[:space:]]+//
		s/^[[:space:]]*((<[[:xdigit:]]+>){2}):[[:space:]]+/\1\n/
		s/[[:space:]]+$//
		p' "${f_d1}" > "${f_d2}"
	gawk -v VAR_NAME="$VAR_NAME" -v ARCH_FLAG="${ARCH_FLAG}" \
		-f "$MPERS_AWK" "${f_d2}" > "${f_h}"
done
