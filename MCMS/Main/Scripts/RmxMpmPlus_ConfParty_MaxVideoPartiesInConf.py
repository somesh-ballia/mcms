#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"

import os
from SystemCapacityFunctions import *

c = McmsConnection()
c.Connect()

command_line = "Bin/McuCmd set ConfParty MAX_CP_RESOLUTION CIF" + '\n'
os.system(command_line)
##  def MaxPartiesTest(self,confFile,partyFile,delayBetweenParticipants=0,max_num_of_parties_in_conf=100,max_party_to_try=2000,num_retries=60,deleteConf="TRUE")
#MaxPartiesTest(c,'Scripts/SystemCapacityTests/AddVideoCpConf.xml','Scripts/AddVideoParty1.xml',0,80,80)
MaxPartiesInConfTest(c,'Scripts/SystemCapacityTests/AddVideoCpConf.xml','Scripts/AddVideoParty1.xml',40,80)
#MaxPartiesInConfTest(c,'Scripts/AddVoipConf.xml','Scripts/AddVoipParty1.xml',200,60,"TRUE")
c.Disconnect()
