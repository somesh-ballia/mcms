#!/bin/sh

OTHER_ITEMS=`grep -v search /etc/resolv.conf`
SEARCH_LINE=`echo -n \`grep search /etc/resolv.conf\` `

echo $SEARCH_LINE $1 > $MCU_HOME_DIR/tmp/resolv.conf
grep -v search /etc/resolv.conf | uniq | sort -r >> $MCU_HOME_DIR/tmp/resolv.conf

cp $MCU_HOME_DIR/tmp/resolv.conf /etc/resolv.conf

