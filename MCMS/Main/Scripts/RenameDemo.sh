#!/bin/sh
newname=$(echo $1 | sed 's/Demo/'$2'/g')
tmpnewname=$newname.tmp
mv $1 $tmpnewname
chmod u+w $tmpnewname
cat $tmpnewname | sed 's/Demo/'$2'/g' > $newname
rm -f $tmpnewname



