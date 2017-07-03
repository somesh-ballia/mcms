#!/mcms/python/bin/python


# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_MPMRX.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmRx.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_200HD_v100.0.cfs"

#-LONG_SCRIPT_TYPE
from SystemCapacityFunctions import *

c = McmsConnection()
c.Connect()

##  def MaxPartiesTest(self,confFile,partyFile,delayBetweenParticipants=0,max_num_of_parties_in_conf=100,max_party_to_try=2000,num_retries=60,deleteConf="TRUE")
MaxPartiesTest1(c,'Scripts/AddVoipConf.xml','Scripts/AddVoipParty1.xml',0,50,200,10,"TRUE")

c.Disconnect()
