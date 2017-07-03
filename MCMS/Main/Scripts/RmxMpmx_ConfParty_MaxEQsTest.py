#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

# For list of profiles look at RunTest.sh
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_breeze.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_60HD_v100.0.cfs"
#-LONG_SCRIPT_TYPE

from McmsConnection import *
from SystemCapacityFunctions import *

c = McmsConnection()
c.Connect()

##  def MaxEntryQueuesTest(self,max_eqs_to_try=20,num_retries=60, delete_eqs = "TRUE"):
MaxEntryQueuesTest(c,40,10)

c.Disconnect()
