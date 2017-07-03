#!/mcms/python/bin/python
###############################################################################################
# ResourceCause7.py
# Description: Causes for failure in resources of the system. 
#			   Fix-Add Participant when no ports left.
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
#-----------------------------------------------------------------------------

def ConnectVideoParties2(connection, conf_id, start_from_indx, num_of_parties, vidMode):
       for x in range(num_of_parties):
          print "x=" + str(x)
          start_from_indx=start_from_indx+1
          #------Dial out H323 party   
          partyname = "Party_"+str(start_from_indx)
          partyip = "1.2.3." + str(start_from_indx)
          print "partyname=" + partyname
          print "partyip=" + str(partyip)
          connection.AddVideoParty(conf_id, partyname, partyip, False, vidMode)
          sleep(3)

       connection.WaitAllPartiesWereAdded(conf_id, num_of_parties, num_retries*2)
       connection.WaitAllOngoingConnected(conf_id)
       connection.WaitAllOngoingNotInIVR(conf_id)
       print "END FUNC ConnectVideoParties"
       
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
c.SetEnhancedConfiguration(0, 144, 0, 0, 2) #audio,CIF,SD,HD720,HD1080

num_retries=1
confName1 = "ConfFixCif"
connection.CreateConf (confName1)
conf_id1  = connection.WaitConfCreated(confName1,num_retries)
ConnectVideoParties2(connection, conf_id1, 0, 120, "fixed") #pure-fix#
print "after ConnectVideoParties!!!!"

#connection.DeleteAllConf()
sleep(1)
#connection.Disconnect()
