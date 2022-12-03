#!/bin/bash

# exit when any command fails
set -e

# WORKING:

./test.sh -f

./test.sh -v
./test.sh -T
./test.sh -TT
./test.sh -v -TT
./test.sh -v -TT

./test.sh -f -v
./test.sh -f -T
./test.sh -f -TT
./test.sh -f -v -TT
./test.sh -f -v -TT

./test.sh -f -v -TT -r

./test.sh -f -v -TT -t -r
./test.sh -f -v -TT -tt -r
./test.sh -f -v -TT -ttt -r
./test.sh -f -v -TT -i
./test.sh -f -v -T -Y
./test.sh -f -v -T -y
./test.sh -f -v -T -yy


# NOT SUPPORTED

# ./test.sh -f -v -TT -k
# 16 | STRUCT ["pid", INT (2346256L)STRING strace"cmd", STRING "execve"; "args", ARRAY [STRING "/bin/sh"; ARRAY [STRING "sh"; STRING "-c"; STRING "ls"]; ARRAY [STRING "USER=lefessan"; STRING "LC_TIME=fr_FR.UTF-8"; STRING "SSH_AGENT_PID=1742"; STRING "XDG_SESSION_TYPE=x11";
