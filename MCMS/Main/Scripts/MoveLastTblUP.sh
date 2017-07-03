#!/bin/bash
# scriptname - MoveLastTblUP.sh
# Description: The script finds last block of text and moves it to any place in the file

#$1 is 'path to NightMain.html'
#$2 is "<table border="
#$3 is "</table>"

#echo $COLORBROWN"MoveLastTblUP: prm2 $2, prm3 $3"$TXTRESET 

file=$1
start=0
end=0
position=0
flag=0

for lineNum in `grep "$2" $file -n | cut -f1 -d":"`
do
	if [ $flag -eq 0 ]
	then
		position=$[$lineNum - 1]
		flag=1
	else
		start=$lineNum
	fi
done

for lineNum in `grep "$3" $file -n | cut -f1 -d":"`
do
	end=$[$lineNum + 1]
done

#echo $COLORBROWN"./Scripts/MovePartOfContents.sh $file $start $end $position"$TXTRESET

./Scripts/MovePartOfContents.sh $file $start $end $position > tmp.tmp
mv tmp.tmp $file
