#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"

from McmsConnection import *
from ResourceUtilities import *
import os

#------------------------------------------------------------------------------
sampleLayoutTypes1 = "1x1", "1and5" 
sampleLayoutTypes2 = "1x2", "3x3"
sampleLayoutTypes3 = "2x1", "1x2Ver"
sampleLayoutTypes4 = "2x2", "1x2Hor"
sampleLayoutTypes5 = "1and2Hor","4x4"
sampleLayoutTypes6 = "1and4Hor","1and7"
sampleLayoutTypes7 = "1and8Central","2and8"
sampleLayoutTypes8 = "1and3Ver", "1and12"
#-----------------------------------------------------------------------------
cif30resources = 1
sd15_2cif30resources = 2
sd30resources = 4

#----------------------------------------------------------------------
def AddVideoPartyWithDefinedRate(connection,confid, partyname, partyip, rate, sip=False):
    if(sip):
        print "Adding SIP Party..." + partyname
        connection.LoadXmlFile('Scripts/SipAddVideoParty1.xml')
    else:
        print "Adding H323 Party..." + partyname
        connection.LoadXmlFile('Scripts/AddVideoParty1.xml')
    #partyip =  "1.2.3.4"
    connection.ModifyXml("PARTY","NAME",partyname)
    connection.ModifyXml("PARTY","IP",partyip)
    connection.ModifyXml("PARTY","VIDEO_BIT_RATE", rate)
    connection.ModifyXml("ADD_PARTY","ID",confid)
    connection.Send()   
           
#----------------------------------------------------------------------
def TestMixRates(connection, resourceUtility, confname, confFile, dialOutH323Rate,dialOutSIPRate,totalNumResource, numResources):
    connection.LoadXmlFile(confFile)
    connection.ModifyXml("RESERVATION","NAME",confname)
    print "Adding Conf " + confname + "  ..."
    connection.Send()
    
    print "Wait untill Conf create...",
    num_retries = 5
    for retry in range(num_retries+1):
        status = connection.SendXmlFile('Scripts/TransConfList.xml',"Status OK")
        confid = connection.GetTextUnder("CONF_SUMMARY","ID")
        if confid != "":
            print
            break
        if (retry == num_retries):
            print connection.xmlResponse.toprettyxml()
            connection.Disconnect()                
            connection.exit("Can not monitor conf:" + status)
        sys.stdout.write(".")
        sys.stdout.flush()

    #connect parties
    print "Start connecting Dial out Parties..."
    num_of_parties = 4   
    #------Dial out H323 party    
    partyname = confname+"_Party_1_"
    partyip =  "1.2.3.1" 
    AddVideoPartyWithDefinedRate(connection,confid, partyname, partyip,dialOutH323Rate )
    print "Connecting H323 Dial out Party, party's rate: "+ str(dialOutH323Rate)
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    sleep(1)
    #------Dial out SIP party   
    partyname = confname+"_Party_2_"
    partyip =  "1.2.3.2" 
    AddVideoPartyWithDefinedRate(connection, confid, partyname, partyip,dialOutSIPRate, "true")
    print "Connecting SIP Dial out Party, party's rate: "+ str(dialOutSIPRate)
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    sleep(1)
    #-------Dial in H323 party
    partyname = confname+"_Party_3_"
    connection.SimulationAddH323Party(partyname, confname)
    connection.SimulationConnectH323Party(partyname)
    print "Connecting H323 Dial in Party -"
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    sleep(1)
    #-------Dial in SIP party
    partyname = confname+"_Party_4"
    connection.SimulationAddSipParty(partyname, confname)
    connection.SimulationConnectSipParty(partyname)
    print "Connecting SIP Dial in Party"
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    sleep(1)
    print "Verify that the resources that were allocated are: " + str(numResources)
    resourceUtility.TestFreeCarmelParties( int(totalNumResource) - int(numResources))
    
    print "Start Changing ConfLayouts Type... "
    #Changing conf layout type to each layout
    for layout in sampleLayoutTypes1:
        connection.ChangeConfLayoutType(confid, layout)
        connection.WaitAllOngoingChangedLayoutType(confid,layout)
        sleep(1)

    print "Start Changing Video Speaker Type... "
    for x in range(num_of_parties): 
        connection.ChangeDialOutVideoSpeaker(confid, x)
        sleep(1)

    print "Start disconnecting Parties..."
    for x in range(num_of_parties-1):
        connection.DeleteParty(confid, x)
        print "current number of connected parties = " + str(num_of_parties-(x+1))
        sleep(1)
  

    print "Deleting Conf..." 
    connection.DeleteConf(confid)   
    connection.WaitAllConfEnd()
    

#-----------------------------------------------------------------------
c = McmsConnection()
c.Connect()
os.system("Bin/McuCmd set mcms IS_DOUBLE_DSP YES")
#-------Verify that all the resources are free---------
r = ResourceUtilities()
r.Connect()
r.SendXmlFile("Scripts/Resource2Undef3RealParties/ResourceMonitorCarmel.xml")
num_of_resource = r.GetTotalCarmelParties()
freeresources = r.GetFreeCarmelParties()
print "free resourcr" + str(freeresources)
r.TestFreeCarmelParties(int(num_of_resource))



print "\n-------------------------------------"
print "Test: SD MIX Video Modes"
print "--------------------------------------"
print "Test 1:Sharpness Conf Mix Party Rates"
print "---------------------------------------"
confname = "SharpnessConfMixPartyRates"
confFile = 'Scripts/SD/AddVideoCpConf256kSharpness.xml'
allocatedResources = (int(sd15_2cif30resources)*2)+int(cif30resources)+int(sd30resources)
TestMixRates(c, r, confname, confFile, 128,512, num_of_resource, allocatedResources)
sleep(1)

print "Test 2: Motion Conf Mix Party Rates"
print "--------------------------------------"
confname = "MotionConfMixPartyRates"
confFile = 'Scripts/SD/AddVideoCpConf128kMotion.xml'
allocatedResources = (int(cif30resources)*2)+int(sd15_2cif30resources)+int(sd30resources)
TestMixRates(c, r, confname, confFile, 1400,512, num_of_resource, allocatedResources)


sleep(1)
r.Disconnect()

sleep(1)
c.Disconnect()
