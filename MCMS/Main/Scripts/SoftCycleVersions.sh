#!/bin/sh

SOFE_CYCLE_LOG=$MCU_HOME_DIR/tmp/startup_logs/softCycleVersions.log	

if [ "$2" != "" ]
  then
  SOFE_CYCLE_LOG=$2
fi

echo  "I am in cycle version script" >> $SOFE_CYCLE_LOG

sync
NEW_VERSION=$1".bin"
if [ "$NEW_VERSION" = ".bin" ]
then
    exit 1
fi
    
RUNNING=$(cat /version.txt).bin
CURRENT=$(readlink $MCU_HOME_DIR/data/current)
FACTORY=$(readlink $MCU_HOME_DIR/data/factory)
FALLBACK=$(readlink $MCU_HOME_DIR/data/fallback)

if [ -f $MCU_HOME_DIR/data/new_version/new_version.bin ]
  then
  echo "new_version.bin is" $NEW_VERSION >> $SOFE_CYCLE_LOG
else
  echo "new_version.bin is not exist. upgrade failed" >> $SOFE_CYCLE_LOG
  exit 1
fi

if [ "$RUNNING" != "$CURRENT" ]
    then
    echo "Running[$RUNNING] package is not current package[$CURRENT]" >> $SOFE_CYCLE_LOG
    if [ "$CURRENT" != "$FALLBACK" ]
       then
       echo "Upgrade: remove current[$CURRENT]" >> $SOFE_CYCLE_LOG
       rm -f $CURRENT
    fi
    echo "Upgrade: current->newVersion[$NEW_VERSION]" >> $SOFE_CYCLE_LOG
    if [ -f $MCU_HOME_DIR/data/$NEW_VERSION ]
      then
      rm -f $MCU_HOME_DIR/data/new_version/new_version.bin
    else
      mv $MCU_HOME_DIR/data/new_version/new_version.bin $MCU_HOME_DIR/data/$NEW_VERSION
    fi
    ln -sf $NEW_VERSION $MCU_HOME_DIR/data/current
    rm -Rf $MCU_HOME_DIR/mcms/Links/*
    cd $MCU_HOME_DIR/data
    rm -f $(ls RMX* |  grep -v `readlink current` | grep -v `readlink fallback` | grep -v factory | grep -v `readlink factory`)
    sync
    cd /
    exit 0
fi

if [ "$NEW_VERSION" = "$CURRENT" ]
    then
    echo $CURRENT "is already installed and active"
    echo "current[$CURRENT] is already installed and active" >> $SOFE_CYCLE_LOG
    exit 2
fi


if [ "$FALLBACK" != "$FACTORY" ]
    then
    if [ "$FALLBACK" != "$CURRENT" ]
	then
		rm -f $MCU_HOME_DIR/data/$FALLBACK
		echo "fallback[$FALLBACK] is removed" >> $SOFE_CYCLE_LOG
    fi
fi

echo "Upgrade normally. fallback->current[$CURRENT]" >> $SOFE_CYCLE_LOG
echo "Upgrade normally. current->newVersion[$NEW_VERSION]" >> $SOFE_CYCLE_LOG

if [ -f $MCU_HOME_DIR/data/$NEW_VERSION ]
  then
  rm -f $MCU_HOME_DIR/data/new_version/new_version.bin
else
  mv $MCU_HOME_DIR/data/new_version/new_version.bin $MCU_HOME_DIR/data/$NEW_VERSION
fi

chown root:root $MCU_HOME_DIR/data/*
chmod a-w $MCU_HOME_DIR/data/*.bin


ln -sf $CURRENT $MCU_HOME_DIR/data/fallback.tmp
ln -sf $NEW_VERSION $MCU_HOME_DIR/data/current.tmp

mv $MCU_HOME_DIR/data/fallback.tmp $MCU_HOME_DIR/data/fallback
mv $MCU_HOME_DIR/data/current.tmp $MCU_HOME_DIR/data/current 

rm -Rf $MCU_HOME_DIR/mcms/Links/*

cd $MCU_HOME_DIR/data

rm -f $(ls RMX* |  grep -v `readlink current` | grep -v `readlink fallback` | grep -v factory | grep -v `readlink factory`)

sync
cd /
exit 0
