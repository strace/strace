#!/bin/sh -e

echo 'enum {'
echo 'SEN_printargs = 0,'
    sed -r -n '/printargs/! s/.*SEN\(([^)]+)\).*/\1/p' |
    LC_COLLATE=C sort -u |
    sed 's/.*/SEN_&,/'
echo '};'
