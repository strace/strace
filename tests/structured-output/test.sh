#!/bin/sh


if [ $# -eq 0 ]; then
    ARGS="-f -v -T"
else
    ARGS="$*"
fi

for i in $(seq 1 2); do
    CMD=$(cat command$i.txt) || exit 2
    ./strace.sh $ARGS $CMD || exit 2
done
