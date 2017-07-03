#!/mcms/python/bin/python

# TODO return back profile 1
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

# For list of profiles look at RunTest.sh
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_MPMRX.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmRx.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_200HD_v100.0.cfs"
#-LONG_SCRIPT_TYPE

from McmsConnection import *
from SystemCapacityFunctions import *

c = McmsConnection()
c.Connect()

## def MaxMeetingRoomsTest(self,max_meetingrooms_to_try=100,num_retries=60, delete_meetingrooms = "TRUE"):

MaxMeetingRoomsTest(c,1000,60)

c.Disconnect()

