#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

# For list of profiles look at RunTest.sh
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_breeze.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_60HD_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_800AUDIO.xml"
#-LONG_SCRIPT_TYPE
from SystemCapacityFunctions import *

c = McmsConnection()
c.Connect()

##  def MaxPartiesTest(self,confFile,partyFile,delayBetweenParticipants=0,max_num_of_parties_in_conf=100,max_party_to_try=2000,num_retries=60,deleteConf="TRUE")
MaxPartiesTest1(c,'Scripts/AddVoipConf.xml','Scripts/AddVoipParty1.xml',0,60,120,10,"TRUE")

c.Disconnect()