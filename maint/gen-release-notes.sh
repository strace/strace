#!/bin/sh -efu

echo '<pre>'
"$(dirname "$0")"/gen-tag-message.sh
echo '</pre>'
