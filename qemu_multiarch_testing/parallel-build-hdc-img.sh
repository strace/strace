#!/bin/sh

export HDBMEGS=128

build_in_dir()
{
	cd "$1" || exit 1
	rm -f hdb.img 2>/dev/null
	nice -n10 ./native-build.sh ../hdc.img
	rm -f hdb.img 2>/dev/null
}

started=false
for dir; do
	test -d "$dir" || continue
	test -e "$dir/native-build.sh" || continue
	echo "Starting: $dir"
	build_in_dir "$dir" </dev/null >"$dir.log" 2>&1 &
	started=true
done

$started || {
	echo "Give me system-image-ARCH directories on command line"
	exit 1
}

echo "Waiting to finish"
wait
echo "Done, check the logs"
