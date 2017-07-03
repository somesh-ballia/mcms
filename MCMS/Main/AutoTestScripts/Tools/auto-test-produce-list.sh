#!/bin/sh
#
# name  : auto-test-produce-list.sh
# author: david rabkin
# date  : 20100613
#
# produces xml entries for auto-test build forge project file, looks as
#
# <var mode="N" value="PSTNConf" position="2" action="S" name="test001"></var>
# <var mode="N" value="ISDN_DialInDialOut" position="3" action="S" name="test002"></var>
#
# extract test names from file owners.py

SRCFILE='owners.py'

# check file existence
if [ ! -f $SRCFILE ]; then
    echo error: unable to find $SRCFILE
    exit 1
fi

# file owners.py contains list of tests in a view
# "GW_1":"Eitan",
# "ISDNConf":"Olga S.",
#
# extract lines between __BEGIN_TESTS__ and __END_TESTS__
# deletes strings that started with # (sed '/^\#/d')
# the result separates by columns with delimeter quote and takes second
# column (cut -d'"' -f 2)
# the result sorts alphabeticalliy and put to array
#ARR=(`cat $SRCFILE | sed '/^\"/!d' | sed '/,$/!d' | cut -d'"' -f 2 | sort`)
ARR=(`cat $SRCFILE | grep -B 500 __END_TESTS__ | grep -A 500 __BEGIN_TESTS__ | sed '/^\#/d' | cut -d'"' -f 2 | sort`)
LEN=${#ARR[@]}

for ((ii=0;ii<$LEN;ii++))
do
    # start with second positon
    let POS=$ii+2
    NAME=${ARR[${ii}]}

    # format name test to 3 symbols at least
    TAG=test`printf "%.3d\n" $[POS-1]`

    # remove last character in test name, it is \n
    line=`echo $line | sed s/.$//`

    STRING="\
        <var mode=\"N\" \
        value=\"${NAME}\" \
        position=\"$POS\" \
        action=\"S\" \
        name=\"$TAG\"></var>"

    # put some spaces for xml formatting
    echo "   "$STRING
done
