#!/bin/sh
#echo "MCMS-Carmel_"`cat base.txt`.`cat build.txt`"_"`date +%e.%b.%Y`
echo $((`cat build.txt` + 1)) > build.txt
