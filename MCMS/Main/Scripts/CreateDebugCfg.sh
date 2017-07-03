#!/bin/sh
ARGC=$#
if [ $ARGC == 0 ]
then
        flagName=""
        flagData=""
        echo "Debug Cfg file was Created without any flags, you need to go and update the file manually"
        echo "Or you have an option to use the same script and give key and value for the system Cfg Flag"
else
        flagName=$1
        flagData=$2
fi
        echo "
<SYSTEM_CFG>
      <CFG_SECTION>
         <NAME>MCMS_PARAMETERS_DEBUG</NAME>
            <CFG_PAIR>
               <KEY>$flagName</KEY>
               <DATA>$flagData</DATA>
            </CFG_PAIR>
          </CFG_SECTION>
</SYSTEM_CFG>"  > $MCU_HOME_DIR/mcms/Cfg/SystemCfgDebug.xml
