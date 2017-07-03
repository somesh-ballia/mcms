#!/usr/bin/python

#-LONG_SCRIPT_TYPE
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4
#-- EXPECTED_ASSERT(1)=Call rejected

from McmsConnection import *
from string import *
from ConfPkgUtil import *
from pprint import pprint

c = McmsConnection()
c.Connect()


##-----------SVC Conf----------------------
num_party = 1
num_retries = 20

#add a new profile
profId = c.AddProfile("SVC_PROFILE", "Scripts/ConfMediaType/NewSvcProfile.xml")

# Creating conference
confname = "conf1"
c.CreateConfFromProfile(confname, profId)
confid = c.WaitConfCreated(confname)

c.LoadXmlFile('Scripts/TransConf2.xml')
c.ModifyXml("GET","ID",confid)
c.Send()

confType = c.GetTextUnder("RESERVATION","CONF_MEDIA_TYPE")
print
print "New CONF MEDIA TYPE reservation is: " + confType


# Add AVC Sip party to EP Sim and connect him
partyname = "Party"+str(1)
c.SimulationAddSipParty(partyname, confname)
c.SimulationConnectSipParty(partyname)

sleep(2)   

c.DeleteConf(confid)

c.Disconnect()

