#!/mcms/python/bin/python

# m_LogicalHD720WeightOfPartyPerType: (AVC)
# +------------+----------+--------+
# | Type       | NonMixed | Mixed  |
# +------------+----------+--------+
# | eAudio     |   0.0833 | 0.6666 |
# | eCif       |      0.5 |      1 |
# | eSD30      |      0.5 |      1 |
# | eHD720     |        1 |    1.5 |
# | eHD1080p30 |        2 |    2.5 |
# | eHD1080p60 |        4 |    4.5 |
# +------------+----------+--------+
# m_LogicalHD720WeightSVCpartyPerType: (SVC)
# +------------+----------+-------+
# | Type       | NonMixed | Mixed |
# +------------+----------+-------+
# | eAudio     |   0.3333 |   0.5 |
# | eCif       |   0.3333 |   0.5 |
# | eSD30      |   0.3333 |   0.5 |
# | eHD720     |   0.3333 |   0.5 |
# | eHD1080p30 |   0.3333 |   0.5 |
# | eHD1080p60 |   0.3333 |   0.5 |
# +------------+----------+-------+

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_MPMRX.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmRx.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_200HD_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_600Video.xml"
   
import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

sleep_seconds = 1

print "---------------------- svc only test -------------------"
profId = c.AddProfile("SVC_PROFILE", "Scripts/ConfMediaType/NewSvcProfile.xml")

# Creating conference
confname = "conf_svc_only"
c.CreateConfFromProfile(confname, profId)
confid = c.WaitConfCreated(confname)
partyname = "SVCParty"+str(1)
c.SimulationAddSipParty(partyname, confname, "FULL CAPSET SVC")
c.SimulationConnectSipParty(partyname)

#Add the 2nd party
partyname = "SVCParty"+str(2)
c.SimulationAddSipParty(partyname, confname, "FULL CAPSET SVC")
c.SimulationConnectSipParty(partyname)

#Add the 3nd party
partyname = "SVCParty"+str(3)
c.SimulationAddSipParty(partyname, confname, "FULL CAPSET SVC")
c.SimulationConnectSipParty(partyname)

sleep(2)

c.WaitAllPartiesWereAdded(confid,3,10)
c.WaitAllOngoingConnected(confid,10)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",400,"TOTAL")
c.TestResourcesReportPortsType("video",2,"OCCUPIED")
c.TestResourcesReportPortsType("video",398,"FREE")

c.DeleteConf(confid)
c.WaitAllConfEnd()


print "---------------------- mix conf test -------------------"
profId = c.AddProfile("MIX_PROFILE", "Scripts/ConfMediaType/NewMixAvcSvcProfile.xml")

# Creating conference
confname = "conf_svc_avc"
c.CreateConfFromProfile(confname, profId)
confid = c.WaitConfCreated(confname)
partyname = "MixSVCParty"+str(1)
c.SimulationAddSipParty(partyname, confname, "FULL CAPSET SVC")
c.SimulationConnectSipParty(partyname)

#Add the 2nd party
partyname = "MixSVCParty"+str(2)
c.SimulationAddSipParty(partyname, confname, "FULL CAPSET SVC")
c.SimulationConnectSipParty(partyname)

#Add the 3nd party
partyname = "MixAvcParty"+str(3)
c.SimulationAddSipParty(partyname, confname)
c.SimulationConnectSipParty(partyname)

sleep(2)

c.WaitAllPartiesWereAdded(confid,3,10)
c.WaitAllOngoingConnected(confid,10)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",400,"TOTAL")
c.TestResourcesReportPortsType("video",8,"OCCUPIED")
c.TestResourcesReportPortsType("video",392,"FREE")

c.DeleteConf(confid)
c.WaitAllConfEnd()


c.Disconnect()

