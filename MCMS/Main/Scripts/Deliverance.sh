#!/bin/sh

FILE="$MCU_HOME_DIR/tmp/Deliverance.txt"
COUNTER=0

. ./Scripts/ClrSetting.sh

echo
echo
echo $TXTRESET
echo $TXTBOLD$COLORMGNTA
echo "////////////////////////////////////////////////////////////////////////////////////"
echo "           Now please start the Deliverance.exe on your PC (Windows)                "
echo "See the installation instruction on http://isrmatlab/wikimedia/index.php/Deliverance"
echo "////////////////////////////////////////////////////////////////////////////////////"
echo $TXTRESET

if [ -f $FILE ]; then
	rm -f $FILE
fi

while [ $COUNTER -lt  400 ] ;do
	#echo $COUNTER
	if [ -f $FILE ]; then
		TEXT=$( cat $FILE )
		if [[ ${TEXT} == "YES" ]];then
			echo
			echo $TXTBOLD$COLORWHITE$BGCOLORGREEN
			echo "*************************************************"
			echo "       CONNECT ENDPOINTS TEST SUCCEEDED          "
			echo "*************************************************"
			echo $TXTRESET
		else
			echo $TXTBOLD$COLORWHITE$BGCOLORRED
			echo "*************************************************"
			echo "       CONNECT ENDPOINTS TEST FAILED             "
			echo "*************************************************"
			echo $TXTRESET
		fi
		rm -f $FILE
		echo
		break
	else
		sleep 10
		COUNTER=$(( $COUNTER + 1 ))
	fi
done
