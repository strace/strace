#!/bin/gawk -f
#
# Copyright (c) 2017-2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

# External variables:
#   COMMENT_MARKER    - marks beginning of the comment on the line
#   COMMENT_MARKER_RE - the same as previous, but in form usable
#                       as a part of a regular expression
#   COPYRIGHT_MARKER  - text inside comment
#   COPYRIGHT_NOTICE  - copyright notice text to insert

BEGIN {
	# States:
	#   0 - before finding copyright notice
	#   1 - in copyright notice or its continuation
	#   2 - right after the end of copyright notice
	#   3 - copyright notice added
	state = 0
	prefix = " "

	comment_re = "^" COMMENT_MARKER_RE
	copyright_re = comment_re "([[:space:]]*)" COPYRIGHT_MARKER
	copyright_cont_re = copyright_re
}

state <= 1 && match($0, copyright_re, a) {
	state = 1
	prefix = a[1]
	# set copyright notice continuation
	copyright_cont_re = comment_re a[1] "[[:space:]]"
}

# this is neither copyright notice nor its continuation
state == 1 && ($0 !~ copyright_re) && ($0 !~ copyright_cont_re) {
	state = 2
}

state == 2 {
	print COMMENT_MARKER prefix COPYRIGHT_NOTICE
	state = 3
}

{
	print
}

END {
	exit 3 - state
}
