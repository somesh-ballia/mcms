#!/bin/sh

# 1. Script name
# 2. Script dir
# 3. Results dir
# 4. Jenkins/Results dir
# 5. MCMS dir
# 6. [Param 1]

echo "Starting $3 Runner..."
uname -n

echo "pwd returned: `pwd -P`"

cd $4

echo "pwd returned: `pwd -P`"


DATE=`date +%F`
if [ -d $DATE/ ];then
	#echo "$DATE exists"
	cd $DATE/;
else
	#echo "$DATE NOT exists"
	mkdir $DATE
	cd $DATE/;
fi

mkdir $3
cd $3

OUT_DIR=`pwd -P`
echo "pwd returned: $OUT_DIR"

cd $5
cd $2
echo "pwd returned: `pwd -P`"

cd $5
echo "pwd returned: `pwd -P`"

$1 $6 | tee $OUT_DIR/ScriptResult.log 2>&1
#./$1 > $OUT_DIR/ScriptResult.log 2>&1
echo "$3 returned: $?" 1>>$OUT_DIR/ScriptResult.log
echo "Finished $3 Runner."
