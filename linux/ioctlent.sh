#! /bin/sh

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


