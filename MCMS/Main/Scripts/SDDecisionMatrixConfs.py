#!/mcms/python/bin/python

#-LONG_SCRIPT_TYPE

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK_4.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"

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

#------------------------------------------------------------------------------
def TestConfrencesVideoModes(connection,resourceUtility, confname, confFile, dialInH323CapSet, dialInSIPCapSet, sampleLayoutTypes,totalNumResource, numResources):
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
    partyname = confname+"_Party_1"
    partyip =  "1.2.3.1" 
    connection.AddVideoParty(confid, partyname, partyip)
    print "Connecting H323 Dial out Party" 
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    #------Dial out SIP party   
    partyname = confname+"_Party_2"
    partyip =  "1.2.3.2" 
    connection.AddVideoParty(confid, partyname, partyip, "true")
    print "Connecting SIP Dial out Party" 
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)

    #-------Dial in H323 party
    partyname = confname+"_Party_3"
    connection.SimulationAddH323Party(partyname, confname,dialInH323CapSet)
    connection.SimulationConnectH323Party(partyname)
    print "Connecting H323 Dial in Party -" + dialInH323CapSet
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)

    #-------Dial in SIP party
    partyname = confname+"_Party_4"
    connection.SimulationAddSipParty(partyname, confname,dialInSIPCapSet)
    connection.SimulationConnectSipParty(partyname)
    print "Connecting SIP Dial in Party" + dialInSIPCapSet
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    
    sleep(1)
    print "Verify that the resources that were allocated are: " + str(numResources)
    resourceUtility.TestFreeCarmelParties( int(totalNumResource) - int(numResources))
    print "Start Changing ConfLayouts Type... "
    #Changing conf layout type to each layout
    sampleLayoutTypes 
    for layout in sampleLayoutTypes:
        connection.ChangeConfLayoutType(confid, layout)
        connection.WaitAllOngoingChangedLayoutType(confid,layout)
        sleep(1)

    print "Start Changing Video Speaker Type... "
    for x in range(num_of_parties-1): 
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



print "----------------- SD Tests -------------------" 
print "---------------------------------------"
print "Test1: SD30 Sharpness"
print "--------------------------------------"
confname = "Conf512Sharpness"
confFile = 'Scripts/SD/AddVideoCpConf512kSharpness.xml'
dialInH323CapSet = "FULL CAPSET"
dialInSIPCapSet  = "H264(sd30)+ALL"
allocatedResources = (int(sd30resources)*4)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes1,num_of_resource, allocatedResources)
sleep(1)


print "\n-------------------------------------"
print "Test2: SD30 Motion"
print "--------------------------------------"
confname = "Conf1024Motion"
confFile = 'Scripts/SD/AddVideoCpConf1024kMotion.xml'
dialInH323CapSet = "G711+H264sd+Fecc"
dialInSIPCapSet  = "FULL CAPSET"
allocatedResources = (int(sd30resources)*4)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet, sampleLayoutTypes2,num_of_resource, allocatedResources)
sleep(1)


print "\n-------------------------------------"
print "Test3: SD15 Sharpness"
print "--------------------------------------"

print "Test3.1: SD15 Sharpness"
print "-----------------------"
confname = "Conf256Sharpness"
confFile = 'Scripts/SD/AddVideoCpConf256kSharpness.xml'
dialInH323CapSet = "G711+H264sd+Fecc"
dialInSIPCapSet  = "H264(sd15)+ALL"
allocatedResources = (int(sd15_2cif30resources)*4)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes3,num_of_resource, allocatedResources)
sleep(1)

print "Test3.2: SD15 Sharpness: Sharpness 1920k Conf, MAX_CP_RESOLUTION= SD15"
print "----------------------------------------------------------------------"
command_line = "Bin/McuCmd set ConfParty MAX_CP_RESOLUTION SD15" + '\n'
os.system(command_line)
confname = "Conf1920Sharpness"
confFile = 'Scripts/SD/AddVideoCpConf1920kSharpness.xml'
dialInH323CapSet = "FULL CAPSET"
dialInSIPCapSet  = "H264(sd15)+ALL"
allocatedResources = (int(sd15_2cif30resources)*4)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes4,num_of_resource, allocatedResources)
command_line = "Bin/McuCmd set ConfParty MAX_CP_RESOLUTION SD30" + '\n'
sleep(1)


print "\n-------------------------------------"
print "Test4: 2CIF30 Motion"
print "--------------------------------------"
print "Test 4.1: 2CIF30 Motion"
print "------------------------"
confname = "Conf384Motion"
confFile = 'Scripts/SD/AddVideoCpConf384kMotion.xml'
dialInH323CapSet = "H264(2cif30)+ALL"
dialInSIPCapSet = "FULL CAPSET"
allocatedResources = (int(sd15_2cif30resources)*4)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes5,num_of_resource, allocatedResources)
sleep(1)

print"Test 4.2: 2CIF30 Motion: Motion  1920k Conf, MAX_CP_RESOLUTION= SD15"
print "-------------------------------------------------------------------"
command_line = "Bin/McuCmd set ConfParty MAX_CP_RESOLUTION SD15" + '\n'
os.system(command_line)
confname = "Conf1920Motion"
confFile = 'Scripts/SD/AddVideoCpConf1920kMotion.xml'
dialInH323CapSet = "H264(sd30)+ALL"
dialInSIPCapSet  = "H264(2cif30)+ALL"
allocatedResources = (int(sd15_2cif30resources)*4)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes6,num_of_resource, allocatedResources)
command_line = "Bin/McuCmd set ConfParty MAX_CP_RESOLUTION SD30" + '\n'
os.system(command_line)
sleep(1)


print "\n-------------------------------------"
print "Test5: CIF30 Sharpness"
print "--------------------------------------"
confname = "Conf128Sharpness"
confFile = 'Scripts/SD/AddVideoCpConf128kSharpness.xml'
dialInH323CapSet = "H264(cif)+ALL"
dialInSIPCapSet  = "G711+H264cif+H263+Fecc"
allocatedResources = (int(cif30resources)*4)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes7, num_of_resource, allocatedResources)
sleep(1)


print "\n-------------------------------------"
print "Test6: CIF30 Motion"
print "--------------------------------------"
confname = "Conf128Motion"
confFile = 'Scripts/SD/AddVideoCpConf128kMotion.xml'
dialInH323CapSet = "H264(cif)+ALL"
dialInSIPCapSet  = "H264(2cif30)+ALL"
allocatedResources = (int(cif30resources)*4)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes8, num_of_resource, allocatedResources)
sleep(1)


sleep(1)
r.Disconnect()

sleep(1)
c.Disconnect()
