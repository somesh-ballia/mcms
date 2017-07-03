#!/mcms/python/bin/python

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_5

# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=Logger Resource


from ResourceUtilities import *
from ISDNFunctions import *

#num_of_parties = 20
#num_of_resource = 20

#num of resource - supposed dongle limitation
#num of parties <= num of resources (capacity to test)

r = ResourceUtilities()
r.Connect()

sleep(3)

r.SendXmlFile("Scripts/ResourceFullCapacity/ResourceMonitorCarmel.xml")
num_of_resource = r.GetTotalCarmelParties()
num_of_isdn_parties = 10
if num_of_resource < 10:
    num_of_isdn_parties = 0
num_retries = 200

print "RSRC_REPORT_RMX[Total] = " + str(num_of_resource)

r.TestFreeCarmelParties(num_of_resource)

# 2000 needed in order to let sufficient time for all 20 parties to connect

confid = r.SimpleXmlConfPartyTest('Scripts/ResourceFullCapacity/AddVideoCpConf.xml',
                         'Scripts/ResourceFullCapacity/AddVideoParty1.xml',
                         int(num_of_resource)-num_of_isdn_parties,
                         num_retries,
                         "FALSE")

if num_of_isdn_parties > 0:
    TestDialOutISDN(r, confid, num_of_isdn_parties, num_retries, "FALSE")

r.TestFreeCarmelParties( 0 )

r.DeleteAllConf()
r.WaitAllConfEnd(100)


r.TestFreeCarmelParties(int(num_of_resource))

r.Disconnect()




