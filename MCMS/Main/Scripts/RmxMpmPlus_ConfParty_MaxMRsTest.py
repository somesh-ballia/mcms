#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"

from McmsConnection import *
from SystemCapacityFunctions import *

c = McmsConnection()
c.Connect()

## def MaxMeetingRoomsTest(self,max_meetingrooms_to_try=100,num_retries=60, delete_meetingrooms = "TRUE"):

MaxMeetingRoomsTest(c,1000,10)

c.Disconnect()
