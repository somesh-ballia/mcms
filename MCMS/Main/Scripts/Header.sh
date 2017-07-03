#!/bin/sh

echo $COLORBLUE'*******************************************************************************'
echo -n '                    '
echo $1 $2 $3 $4 $5
echo '*******************************************************************************' $TXTRESET

export LD_LIBRARY_PATH=$MCU_HOME_DIR/usr/local/apache2/lib:$MCU_HOME_DIR/mcms/Bin
export SASL_PATH=$MCU_HOME_DIR/mcms/Bin
