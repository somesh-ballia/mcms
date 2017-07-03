#!/bin/bash
# scriptname - MovePartOfContents.sh
# Description: The script moves block of the file into special position in the file

file=$1
start=$2     # Start of block line number
end=$3       # End of block line number
position=$4  # Position in $file where to move the block to

sed -n "${start},${end}p" ${file} > ${file}.tmp
sed -e "${start},${end}d" -e "${position}r ${file}.tmp" ${file}
rm -f ${file}.tmp
