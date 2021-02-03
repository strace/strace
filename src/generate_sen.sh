#!/bin/sh -e
#
# Copyright (c) 2015-2019 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

[ "x${D:-0}" != x1 ] || set -x

echo 'enum {'
echo 'SEN_printargs = 0,'
    sed -r -n '/printargs/! s/.*SEN\(([^)]+)\).*/\1/p' |
    LC_COLLATE=C sort -u |
    sed 's/.*/SEN_&,/'
echo '};'
