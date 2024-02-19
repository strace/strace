#!/bin/sh

CMD=$*

../../src/strace -o strace.log $CMD > /dev/null

echo ../../src/strace -B ocaml -o strace_log.log $CMD -c ls
../../src/strace -B ocaml -o strace_log.log $CMD > /dev/null


echo "
type t =
  | NULL
  | PARTIAL_STRING of string
  | STRING of string
  | INT of int64
  | INDIRECT of t list
  | ARRAY of t list
  | FLAGS of t list
  | STRUCT of ( string * t ) list
  | CALL of string * t list
  | ARG of string * t
  | SHIFT of t * t
  | INDEX of t * t

let s = [
" > strace_log.ml
cat strace_log.log >> strace_log.ml
echo "]" >> strace_log.ml

echo ocamlc -c strace_log.ml
ocamlc -c strace_log.ml || exit 2


echo ../../src/strace -B json -o strace_json.log $CMD -c ls
../../src/strace -B json -o strace_json.log $CMD > /dev/null
echo "[" > strace_log.json
cat strace_json.log >> strace_log.json
echo "null ]" >> strace_log.json
echo jsonlint-php strace_log.json
jsonlint-php strace_log.json || exit 2
