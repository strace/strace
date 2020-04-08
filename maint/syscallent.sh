#!/bin/sh
# Copyright (c) 1993, 1994, 1995, 1996 Rick Sladkey <jrs@world.std.com>
# Copyright (c) 1996-2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

cat ${1+"$@"} |
	sed -n 's/^#[ 	]*define[ 	][ 	]*SYS_\([^ 	]*\)[ 	]*[^0-9]*\([0-9]*\).*$/\1 \2/p
s/^#[ 	]*define[ 	][ 	]*__NR_\([^ 	]*\)[ 	]*[^0-9]*\([0-9]*\).*$/\1 \2/p
s/^#[ ]*define[ ][ ]*__NR_\([^ ]*\)[ ]*[^0-9()]*(__NR_Linux + \([0-9]*\))$/\1 \2/p' |
	sort -k2n | uniq |
	awk '
	BEGIN {
		tabs = "\t\t\t\t\t\t\t\t"
		call = -1;
	}
	{
		while (++call < $2) {
			f = "printargs"
			n = "SYS_" call
			s = "\t{ MA,\t0,\t"
			s = s f ","
			s = s substr(tabs, 1, 24/8 - int((length(f) + 1)/8))
			s = s "\"" n "\""
			s = s substr(tabs, 1, 16/8 - int((length(n) + 2)/8))
			s = s "}, /* " call " */"
			print s
		}
		f = "sys_" $1
		n = $1
		s = "\t{ MA,\t0,\t"
		s = s f ","
		s = s substr(tabs, 1, 24/8 - int((length(f) + 1)/8))
		s = s "\"" n "\""
		s = s substr(tabs, 1, 16/8 - int((length(n) + 2)/8))
		s = s "}, /* " call " */"
		print s
	}
	END {
		limit = call + 100
		while (++call < limit) {
			f = "printargs"
			n = "SYS_" call
			s = "\t{ MA,\t0,\t"
			s = s f ","
			s = s substr(tabs, 1, 24/8 - int((length(f) + 1)/8))
			s = s "\"" n "\""
			s = s substr(tabs, 1, 16/8 - int((length(n) + 2)/8))
			s = s "}, /* " call " */"
			print s
		}
	}
	'
