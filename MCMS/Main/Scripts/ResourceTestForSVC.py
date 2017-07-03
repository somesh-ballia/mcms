#!/mcms/python/bin/python

# m_LogicalHD720WeightOfPartyPerType: (AVC)
# +------------+----------+--------+
# | Type       | NonMixed | Mixed  |
# +------------+----------+--------+
# | eAudio     |   0.0833 | 0.0833 |
# | eCif       |   0.3333 |   0.75 |
# | eSD30      |      0.5 |   0.75 |
# | eHD720     |        1 |    1.5 |
# | eHD1080p30 |        2 |      3 |
# | eHD1080p60 |        3 |      5 |
# +------------+----------+--------+
# m_LogicalHD720WeightSVCpartyPerType: (SVC)
# +------------+----------+--------+
# | Type       | NonMixed | Mixed  |
# +------------+----------+--------+
# | eAudio     |   0.3333 | 0.3333 |
# | eCif       |   0.3333 | 0.3333 |
# | eSD30      |   0.3333 | 0.3333 |
# | eHD720     |   0.3333 | 0.3333 |
# | eHD1080p30 |   0.3333 | 0.3333 |
# | eHD1080p60 |   0.3333 | 0.3333 |
# +------------+----------+--------+

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE_4_CARDS.XML"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_120_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_360Video.xml"

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

sleep(5)

c.WaitAllPartiesWereAdded(confid,3,10)
c.WaitAllOngoingConnected(confid,10)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",3,"OCCUPIED")
c.TestResourcesReportPortsType("video",357,"FREE")

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

sleep(5)

c.WaitAllPartiesWereAdded(confid,3,10)
c.WaitAllOngoingConnected(confid,10)

sleep(sleep_seconds)
c.TestResourcesReportPortsType("video",360,"TOTAL")
c.TestResourcesReportPortsType("video",12,"OCCUPIED")
c.TestResourcesReportPortsType("video",349,"FREE")

c.DeleteConf(confid)
c.WaitAllConfEnd()

c.Disconnect()

