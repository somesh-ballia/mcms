#!/mcms/python/bin/python
###############################################################################################
# ResourceCause4.py
# Description: Causes for failure in resources of the system. 
#			   fixed-Add Participant. Maximum number of participants is reached.
###############################################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_30SD.xml"

from McmsConnection import *
from ResourceUtilities import *
import os      
#--------------------------------------------------------------------------------

##-----------
## fixed Mode
##-----------
connection = McmsConnection()
connection.Connect()
c = ResourceUtilities()
c.Connect()
c.WaitUntilStartupEnd()

c.SetMode("fixed")
c.CheckModes("fixed","fixed")
c.SetEnhancedConfiguration(0, 160, 0, 0, 0) #audio,CIF,SD,HD720,HD1080

num_of_parties1=1
confName1 = "ConfCif"

profId=c.AddProfileWithRate("ProfRate128",128)
num_of_parties = 161
conf_id1 = c.CreateConfFromProfileWithVideoParties(confName1,profId,num_of_parties)
c.WaitAllOngoingConnected(conf_id1)
print "after CreateConfFromProfileWithVideoParties!!!!"

#connection.DeleteAllConf()
sleep(1)
#connection.Disconnect()
