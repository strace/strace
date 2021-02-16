#!/bin/sh -efu
#
# Update copyright notices for source files.
#
# Copyright (c) 2017-2021 The strace developers.
# All rights reserved.
#
# SPDX-License-Identifier: LGPL-2.1-or-later

DEFAULT_GIT_COMMIT_TEMPLATE="Update copyright headers

Headers updated automatically with

    $0 $*"

: ${COPYRIGHT_NOTICE='The strace developers.'}
: ${COPYRIGHT_MARKER='Copyright'}
: ${COPYRIGHT_PREFIX="${COPYRIGHT_MARKER} (c)"}
: ${VERBOSE=1}
: ${CALL_GIT_ADD=0}
: ${CALL_GIT_COMMIT=0}
: ${GIT_COMMIT_TEMPLATE=$DEFAULT_GIT_COMMIT_TEMPLATE}
LC_TIME=C; export LC_TIME

# These files are only imported into strace and not changed.
# Remove them from the list once they have been changed.
IGNORED_FILES="build-aux/git-set-file-times
build-aux/gitlog-to-changelog
bundled/linux/.*
${ADDITIONAL_IGNORED_FILES-}"

log()   { [ "$VERBOSE" -lt 1 ] || printf '%s\n' "$*"; }
debug() { [ "$VERBOSE" -lt 2 ] || printf '%s\n' "$*"; }

print_help()
{
	cat <<'EOF'
Usage: update_copyright_notices [-v]* [-q]* [-a] [-h] [-j JOBS] [FILES]

If no files provided, process all files in current directory, recursively.
Only git-tracked files are processed.

Script implements hard-coded logic for extracting comments in files:
  ' *'    *.c, *.h (as part of multiline /* ... */ comment)
  '.\"'   *.[1-8] (man pages)
  '#'     Everything else

Options:
  -a  Invoke git add for the changed files.
  -c  Call git commit with message provided in $GIT_COMMIT_TEMPLATE (implies -a).
  -v  Increase verbosity.
  -q  Decrease verbosity.
  -h  Show this help.
  -b  Overwrite beginning year too on update of existing notice.
  -j  Maximum concurrent jobs.

Environment:
  COPYRIGHT_NOTICE  Copyright ownership string.
  COPYRIGHT_MARKER  What strings are considered copyright strings.
  COPYTIGHT_PREFIX  Part of copyright string before the year interval.
  VERBOSE           Verbosity level.
  CALL_GIT_ADD      Whether to invoke 'git add' on processed files.
EOF
}

# $1 - file name
# $2 - return version suitable for using in regular expressions
get_comment_prefix()
{
	case "$1" in
	*.[ch])
		printf '%s' ' *'
		;;
	*.[1-8])
		printf '%s' '.\"'
		;;
	*)
		printf '%s' '#'
		;;
	esac
}

# $1 - file
process_file()
{
	local f p r span p_quoted r_quoted year_re start_note
	local last_commit_year first_commit_year
	local copyright_year copyright_year_raw copyright_notice
	local existing_notice_re existing_notice_year

	f="$1"

	p=$(get_comment_prefix "$f")
	r=$(printf '%s' "$p" | sed 's/[].*&^$[\/]/\\&/g')

	year_re="[12][0-9][0-9][0-9]"
	copyright_year_raw=$(sed -n \
		"/^${r}  *${COPYRIGHT_MARKER}"'/s/.*[- ]\('"${year_re}"'\)\( .*\)\?$/\1/p' \
			< "$f")

	if [ -z "$copyright_year_raw" ]; then
		debug "Copyright notices haven't been found, skipping: $f"
		return
	fi

	last_commit_year=$(date -u +%Y -d "@$(git log --format=format:%at -- "$f" |
						sort -rn |head -n1)")
	first_commit_year=$(date -u +%Y -d "@$(git log --format=format:%at -- "$f" |
						sort -n |head -n1)")
	copyright_year=$(printf '%s' "$copyright_year_raw" |
		sort -r -n | head -n 1)
	start_note='from git log'

	existing_notice_re="^\(${r}  *${COPYRIGHT_MARKER}.* \)\(\(${year_re}\)\([-, ]*${year_re}\)*\)\( ${COPYRIGHT_NOTICE}\)$"
	existing_notice_year=$(sed -n \
		"/${existing_notice_re}/s//\\3/p" "$f")
	# assume copyright notice is still relevant
	if [ "$last_commit_year" = "$copyright_year" ]; then
		debug "Does not need update, skipping: $f"
		return
	else
		debug "Needs update ('$copyright_year' != '$last_commit_year'): $f"
	fi

	# avoid gaps not covered by copyright
	[ -n "$copyright_year" ] &&
	[ "$first_commit_year" -lt "$copyright_year" ] || {
		start_note='from last copyright year'
		first_commit_year="$copyright_year"
	}

	# if there is existing notice and its starting year is earlier
	# than the year of the first commit, the former takes precedence
	if [ -n "$existing_notice_year" ] &&
	   [ "$existing_notice_year" -le "$first_commit_year" ]; then
		start_note='from existing copyright notice'
		first_commit_year="$existing_notice_year"
	fi

	if [ "$first_commit_year" = "$last_commit_year" ]; then
		span="$last_commit_year"
	else
		span="$first_commit_year-$last_commit_year"
	fi

	copyright_notice="${COPYRIGHT_PREFIX} ${span} ${COPYRIGHT_NOTICE}"
	p_quoted="$(printf '%s' "$p" | sed 's/\\/\\\\/g')" \
	r_quoted="$(printf '%s' "$r" | sed 's/\\/\\\\/g')" \

	if [ -n "$existing_notice_year" ]; then
		# update existing notice, avoid touching starting date
		sed -i "/${existing_notice_re}/s//\\1${span} ${COPYRIGHT_NOTICE}/" "$f" &&
			log "Updated copyright notice in $f (start year $start_note)"
	else
		awk \
			-v COMMENT_MARKER="$p_quoted" \
			-v COMMENT_MARKER_RE="$r_quoted" \
			-v COPYRIGHT_NOTICE="$copyright_notice" \
			-v COPYRIGHT_MARKER="$COPYRIGHT_MARKER" \
			-f $(dirname "$0")/update_copyright_years.awk \
			"$f" > "$f.out" && {
				cat "$f.out" > "$f"
				log "Added copyright notice to $f (start year $start_note)"
			} || debug "No changes performed (exit code $?), skipping: $f"

			rm -f "$f.out"
	fi

	[ "$CALL_GIT_ADD" = 0 ] || git add "$f"
}

MAX_JOBS="$(getconf _NPROCESSORS_ONLN)"
: $(( MAX_JOBS *= 2 ))

while [ -n "${1-}" ]; do
	case "$1" in
	"-v")
		VERBOSE=$(($VERBOSE + 1))
		;;
	"-q")
		VERBOSE=$(($VERBOSE - 1))
		;;
	"-h")
		print_help
		exit 1
		;;
	"-a")
		CALL_GIT_ADD=1
		;;
	"-c")
		CALL_GIT_ADD=1
		CALL_GIT_COMMIT=1
		;;
	"-j")
		shift
		MAX_JOBS="$1"
		;;
	*)
		break
		;;
	esac

	shift
done

jobs=0
pids=
[ 1 -le "${MAX_JOBS}" ] || MAX_JOBS=2

git ls-files -- "$@" | grep -vx "$IGNORED_FILES" | while read f; do
	process_file "$f" &
	pids="$pids $!"
	: $(( jobs += 1 ))
	if [ "${jobs}" -gt "$MAX_JOBS" ]; then
		read wait_pid rest
		pids="$rest"
		wait -n 2>/dev/null || wait "$wait_pid"
		: $(( jobs -= 1 ))
	fi <<- EOF
	$pids
	EOF
done

wait

[ "$CALL_GIT_COMMIT" = 0 ] || git commit -m "$GIT_COMMIT_TEMPLATE"
