#!/bin/sh

rm -Rf Cfg
mkdir -p Cfg
Kernel=`uname -r | cut -d '-' -f 1`

export COLORBLUE=$(tput setaf 4)    #  blue      / light blue
export TXTRESET=$(tput sgr0)        # Reset

# CentOS 5.8
if [[ "$Kernel" == "2.6.18" ]]; then
	for TestBinary in Bin.i32cent56/*.Test
	 do echo "$COLORBLUE=== Running $TestBinary ===  $TXTRESET"
	 export LD_LIBRARY_PATH=Bin.i32cent56:Bin.i32ptx 
	 export SASL_PATH=Bin.i32ptx 
	 echo "$TestBinary"
	 $TestBinary || exit 102
	 done
# CentOs 6.3
elif [[ "$Kernel" == "2.6.32" ]]; then
	for TestBinary in Bin.i32cent6x/*.Test 
	 do echo "$COLORBLUE=== Running $TestBinary ===  $TXTRESET" 
	 export LD_LIBRARY_PATH=Bin.i32cent6x:Bin.i32ptx 
	 export SASL_PATH=Bin.i32ptx 
	 $TestBinary || exit 102
	 done
else
	for TestBinary in Bin.i32cent56/*.Test 
	 do echo "$COLORBLUE=== Running $TestBinary ===  $TXTRESET" 
	 export LD_LIBRARY_PATH=Bin.i32cent56:Bin.i32ptx
	 export SASL_PATH=Bin.i32ptx
	 $TestBinary || exit 102
	 done
fi

