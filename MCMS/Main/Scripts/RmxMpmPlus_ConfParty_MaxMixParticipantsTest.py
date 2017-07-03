#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_400AUDIO_80CIF30.xml"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"
#-LONG_SCRIPT_TYPE

import os
from SystemCapacityFunctions import *

c = McmsConnection()
c.Connect()

command_line = "Bin/McuCmd set ConfParty MAX_CP_RESOLUTION CIF" + '\n'
os.system(command_line)
##def MaxMixPartiesTest(self,confFile,partyFile,delayBetweenParticipants=0,max_num_of_parties_in_conf=80,max_num_of_video_parties_in_conf=80, max_party_to_try=400,max_video_parties_to_try =80,num_retries=60,deleteConf="TRUE"):
MaxMixPartiesTest(c,'Scripts/AddVoipConf.xml','Scripts/AddVoipParty1.xml',0,48,8,480,80)

c.Disconnect()
