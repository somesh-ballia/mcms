#!/mcms/python/bin/python
###############################################################################################
# ResourceCause1.py
# Description: Causes for failure in resources of the system. 
#			   downgrade video allocated type since available space in sys is in separate units.
###############################################################################################

from McmsConnection import *
from ResourceUtilities import *
import os

##-----------
## Auto Mode:
##-----------
#----------------------------------------------------------------------------------------------
#	    Auto-Downgrade-Add HD1080 party when left HD720/SD/CIF -> use script. get 50% of units free and add HD1080
#		until no space left on units. it will downgrade according to left capacity. (create HD by create hd conf
#		profile-4096kbps and when adding party to this conf - party req is for HD1080.
#----------------------------------------------------------------------------------------------

connection = McmsConnection()
connection.Connect()
c = ResourceUtilities()
c.Connect()
c.WaitUntilStartupEnd()

c.SetMode("auto")
c.CheckModes("auto","auto")
c.SetAutoPortConfiguration(0)

num_of_parties1=1
confName1 = "ConfAutoCif"
profId=c.AddProfileWithRate("ProfRate128",128)
num_of_parties = 160

conf_id1 = c.CreateConfFromProfileWithVideoParties(confName1,profId,num_of_parties)
c.WaitAllOngoingConnected(conf_id1)
print "After CreateConfFromProfileWithVideoParties!!!!"

#connection.DeleteAllConf()
sleep(1)
#connection.Disconnect()
