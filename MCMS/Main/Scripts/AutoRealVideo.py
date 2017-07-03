#!/mcms/python/bin/python


#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_22
#-LONG_SCRIPT_TYPE

from McmsConnection import *
from ISDNFunctions import *

c = McmsConnection()
c.Connect()

sleepBetweenParties = 1
if (c.IsProcessUnderValgrind("ConfParty")):
    sleepBetweenParties = 10  # we need long sleep time in order to let the ip parties to change their content mode from 264 to 263 
    
confid = c.SimpleXmlConfPartyTest('Scripts/AddVideoCpConf.xml',
                         'Scripts/AddVideoParty1.xml',
                         3,
                         60,
			 "TRUE")
#TestDialOutISDN(c, confid, 3, 60, "TRUE", sleepBetweenParties)    # no ISDN in COP version

c.Disconnect()


