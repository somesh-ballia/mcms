#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind


# For list of profiles look at RunTest.sh
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_breeze.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_60HD_v100.0.cfs"
#-LONG_SCRIPT_TYPE

from SystemCapacityFunctions import *

c = McmsConnection()
c.Connect()

#def MaxPartiesInConfTest(self,confFile,partyFile,parties_in_group,max_party_in_conf=800,num_retries=60,deleteConf="TRUE",conf_serial_number = 1):


MaxPartiesInConfTest(c,'Scripts/SystemCapacityTests/AddVideoCpConf.xml','Scripts/AddVideoParty1.xml',120,120)

c.Disconnect()
