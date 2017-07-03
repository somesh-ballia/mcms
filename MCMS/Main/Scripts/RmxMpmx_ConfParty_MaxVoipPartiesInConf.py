#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_FOR_VALGRIND =    Logger MplApi


# For list of profiles look at RunTest.sh
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_breeze.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_60HD_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_800AUDIO.xml"
#-LONG_SCRIPT_TYPE
from SystemCapacityFunctions import *

c = McmsConnection()
c.Connect()

MaxPartiesInConfTest(c,'Scripts/AddVoipConf.xml','Scripts/AddVoipParty1.xml',120,120,60,"TRUE")
c.Disconnect()
