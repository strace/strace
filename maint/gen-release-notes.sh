#!/bin/sh -efu
#
# Copyright (c) 2018 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

echo '<pre>'
"$(dirname "$0")"/gen-tag-message.sh
echo '</pre>'
