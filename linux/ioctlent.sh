#! /bin/sh
#
# Copyright (c) 2001 Wichert Akkerman <wichert@cistron.nl>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. The name of the author may not be used to endorse or promote products
#    derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
# IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
# NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
# THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#	$Id$
#

dir="/usr/include"
files="linux/* asm/* scsi/*"

# Build the list of all ioctls
regexp='^[[:space:]]*#[[:space:]]*define[[:space:]]\+[A-Z][A-Z0-9_]*[[:space:]]\+_S\?\(IO\|IOW\|IOR\|IOWR\)\>'
(cd $dir ; grep $regexp $files 2>/dev/null ) | \
	sed -ne 's/^\(.*\):[[:space:]]*#[[:space:]]*define[[:space:]]*\([A-Z0-9_]*\)[[:space:]]*_S\?I.*(\([^[,]*\)[[:space:]]*,[[:space:]]*\([^,)]*\).*/	{ "\1",	"\2",	_IOC(_IOC_NONE,\3,\4,0)	},/p' \
	> ioctls.h

# Some use a special base to offset their ioctls on. Extract that as well.
: > ioctldefs.h

bases=$(sed -ne 's/.*_IOC_NONE.*,[[:space:]]*\([A-Z][A-Z0-9_]\+\)[[:space:]+,].*/\1/p' ioctls.h | uniq | sort)
for base in $bases ; do
	echo "Looking for $base"
	regexp="^[[:space:]]*#[[:space:]]*define[[:space:]]\+$base"
	(cd $dir ; grep -h $regexp 2>/dev/null $files ) | \
		grep -v '\<_IO' >> ioctldefs.h
done


