#!/bin/sh
# script name: mult_file.sh
# author     : Vasily
# copyright  : Polycom Inc (c) 2014

usage(){
	echo "Usage: mult_file.sh [FILE] [NCOPIES] [DEST], where"
	echo "   [FILE] - name of source file, [NCOPIES] - number of copies, [DEST] - destination folder name"
}

if [ -z "$1" ]; then
	echo "mult_file.sh - file multiplication, duplicates file up to 20000 times"
	usage
	exit -1
fi

if [[ -z $2 || -z "$3" ]]; then
	echo "mult_file.sh error: not enough parameters" 
	usage
	exit -1
fi

if [ $2 -le 0 ]; then
	echo "mult_file.sh - max of copies have to be bigger than 0"
	exit -1
fi

if [ $2 -gt 20000 ]; then
	echo "mult_file.sh - max of copies exceeded (20000)"
	exit -1
fi

FILE="$1"
NCOPIES=$2
DEST_DIR="$3"

if [ ! -f $FILE ]; then
	echo "mult_file.sh error: No source file found: "$FILE
	exit -1
fi

if [ ! -d $DEST_DIR ]; then
	echo "Creating folder "$DEST_DIR
	mkdir $DEST_DIR
fi

SAMPLE_FILENAME="Log_SN0000100121_FMD24022014_FMT101857_LMD24022014_LMT102722_SZ1024105_SUN_CFzlib_NFV02_RTN.log"

i=0
while [ $i -ne $NCOPIES ]
do
	i=`expr $i + 1`
	curr_num=`echo $i | awk '{ printf("%05d",$1) }'`
	curr_file_name=Log_SN00001${curr_num}_FMD24022014_FMT101857_LMD24022014_LMT102722_SZ1024105_SUN_CFzlib_NFV02_RTN.log
	#echo $curr_file_name
	echo -n "."
	cp $FILE $DEST_DIR/$curr_file_name
done
echo 
echo $i" files created successfully"
echo 




