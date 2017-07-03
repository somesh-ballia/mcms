#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_MPMRX.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmRx.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_200HD_v100.0.cfs"
#-LONG_SCRIPT_TYPE

from McmsConnection import *
from SystemCapacityFunctions import *

c = McmsConnection()
c.Connect()

##  def MaxConfrencesTest(self,confFile,max_cofrences_to_try=2000,num_retries=60,deleteConf="TRUE"):
MaxConfrencesTest(c,'Scripts/AddVoipConf.xml',100)

c.Disconnect()

