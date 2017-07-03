#!/bin/sh

if [ -d Links ]; then
  rm -Rf Links/*
else
  mkdir Links
fi

ln -s ../Bin/EndpointsSim Links/EndpointsSim
ln -s ../Bin/GideonSim Links/GideonSim


if [ "$DEBUG_PROCESS" != "" ]
then
    (
	if [ "$BREAK_IN" != "" ]
	    then
	    echo "b main"
	    echo "run"
	    echo "b $BREAK_IN"
	    echo "cont"
	fi
    ) > Links/gdb.txt

    (
	echo "#!/bin/sh"
	echo "ddd Bin/ConfParty" --command=Links/gdb.txt
    ) > Links/$DEBUG_PROCESS
    
    chmod a+x Links/$DEBUG_PROCESS
fi

if test -e $MCU_HOME_DIR/mcms/Cfg/SystemCfgUserTmp.xml; then
  SYSCFG_FILE="$MCU_HOME_DIR/mcms/Cfg/SystemCfgUserTmp.xml"
  
else
 if test -e $MCU_HOME_DIR/mcms/Cfg/SystemCfgUser.xml; then
   SYSCFG_FILE="$MCU_HOME_DIR/mcms/Cfg/SystemCfgUser.xml"
  fi
fi

