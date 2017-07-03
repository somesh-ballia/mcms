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

# def MaxPartiesInConfTest(self,confFile,partyFile,parties_in_group,max_party_in_conf=800,num_retries=60,deleteConf="TRUE",conf_serial_number = 1):
MaxPartiesInConfTest(c,'Scripts/AddVoipConf.xml','Scripts/AddVoipParty1.xml',200,200,60,"TRUE")
c.Disconnect()
