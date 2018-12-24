#!/bin/sh -efu
#
# Copyright (c) 2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

"$(dirname "$0")"/gen-tag-message.sh |
	sed 's/\([^[:space:]]\)\*/\1\\*/g'

cat <<'EOF'

Downloads
=========

**Please ignore so called "Source code" links provided by github above, they are useless**.
EOF
