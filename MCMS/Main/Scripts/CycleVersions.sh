#!/bin/sh

sync
NEW_VERSION=$1".bin"
if [ "$NEW_VERSION" = ".bin" ]
then
    exit 1
#	NEW_VERSION=$($MCU_HOME_DIR/mcms/Scripts/TestFirmware.sh $MCU_HOME_DIR/data/new_version/new_version.bin).bin
#	if [ "$?" != "0" ]
#	then
#	    exit $?
#	fi
fi
    
RUNNING=$(cat /version.txt).bin
CURRENT=$(readlink $MCU_HOME_DIR/data/current)
FACTORY=$(readlink $MCU_HOME_DIR/data/factory)
FALLBACK=$(readlink $MCU_HOME_DIR/data/fallback)


if [ "$RUNNING" != "$CURRENT" ]
    then
    rm -f $CURRENT
    mv $MCU_HOME_DIR/data/new_version/new_version.bin $MCU_HOME_DIR/data/$NEW_VERSION
    ln -sf $NEW_VERSION $MCU_HOME_DIR/data/current
    rm -Rf $MCU_HOME_DIR/mcms/Links/*
    cd $MCU_HOME_DIR/data
    rm -f $(ls RMX* |  grep -v `readlink current` | grep -v `readlink fallback` | grep -v factory)
    sync
    exit 0
fi

if [ "$NEW_VERSION" = "$CURRENT" ]
    then
    echo $CURRENT "is already installed and active"
    exit 1
fi


if [ "$FALLBACK" != "$FACTORY" ]
    then
    if [ "$FALLBACK" != "$CURRENT" ]
	then
		rm -f $MCU_HOME_DIR/data/$FALLBACK
    fi
fi

mv $MCU_HOME_DIR/data/new_version/new_version.bin $MCU_HOME_DIR/data/$NEW_VERSION
chown root:root $MCU_HOME_DIR/data/*
chmod a-w $MCU_HOME_DIR/data/*.bin


ln -sf $CURRENT $MCU_HOME_DIR/data/fallback.tmp
ln -sf $NEW_VERSION $MCU_HOME_DIR/data/current.tmp

mv $MCU_HOME_DIR/data/fallback.tmp $MCU_HOME_DIR/data/fallback
mv $MCU_HOME_DIR/data/current.tmp $MCU_HOME_DIR/data/current 

rm -Rf $MCU_HOME_DIR/mcms/Links/*

cd $MCU_HOME_DIR/data
#ls RMX* | grep -v `readlink current` | grep -v `readlink fallback` | grep -v `readlink factory` | xargs rm -Rf 
#rm $FALLBACK

rm -f $(ls RMX* |  grep -v `readlink current` | grep -v `readlink fallback` | grep -v factory)

sync
exit 0
