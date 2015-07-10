#!/bin/sh -e

echo 'enum {'
echo 'SEN_printargs = 0,'
    sed -n '/printargs/! s/.*SEN(\([^)]*\)).*/\1/p' |
    sort -u |
    sed 's/.*/SEN_&,/'
echo '};'
