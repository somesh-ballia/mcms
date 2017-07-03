#!/mcms/python/bin/python

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
#rsrc_report_rmx_list = r.xmlResponse.getElementsByTagName("RSRC_REPORT_RMX")
#for index in range (len(rsrc_report_rmx_list)):
#	if("video" == r.GetTextUnder("RSRC_REPORT_RMX","RSRC_REPORT_ITEM",index)):
#		num_of_resource = r.GetTextUnder("RSRC_REPORT_RMX","TOTAL",index)
#		freeresources = r.GetTextUnder("RSRC_REPORT_RMX","FREE",index)
#		break
		
#num_of_resource = r.GetTextUnder("RSRC_REPORT_RMX","TOTAL")
#freeresources = r.GetTextUnder("RSRC_REPORT_RMX","FREE")
print "total video resources: " + str(num_of_resource) + " free video resources: " + str(freeresources)
r.TestFreeCarmelVideoParties(int(num_of_resource))

print "\n---------------------------------------"
print "Test SD MIX Video Protocols H263/H264  "
print "---------------------------------------"

print "Test 1: Mix protocols in SD30 Sharpness conf H263/H264"
print "---------------------------------------------------------"
confname = "Conf512SharpMixProtocol"
confFile = 'Scripts/SD/AddVideoCpConf512kSharpness.xml'
dialInH323CapSet = "FULL CAPSET"
dialInSIPCapSet  = "G711+H263+Fecc"
allocatedResources = (int(sd30resources)*3)+ int(sd15_2cif30resources)
##In 512 Sharpness conference H263 is H2634CIF thus it requires resources as SD15(full DSP)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes1,num_of_resource, allocatedResources)
sleep(1)

print "Test 2:  Mix protocols in SD15 Sharpness conf H263/H264"
print "---------------------------------------------------------"
confname = "Conf256SharpMixProtocol"
confFile = 'Scripts/SD/AddVideoCpConf256kSharpness.xml'
dialInH323CapSet = "G711+H263+Fecc"
dialInSIPCapSet  = "FULL CAPSET"
allocatedResources = (int(sd15_2cif30resources)*4) 
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes2,num_of_resource, allocatedResources)
sleep(1)

print "Test 3:  Mix protocols in CIF30 Sharpness conf H263/H264"
print "---------------------------------------------------------"
confname = "Conf128SharpMixProtocol"
confFile = 'Scripts/SD/AddVideoCpConf128kSharpness.xml'
dialInH323CapSet = "G711+H263+Fecc"
dialInSIPCapSet  = "G711+H263+Fecc"
allocatedResources = (int(cif30resources)*4) 
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes3,num_of_resource, allocatedResources)
sleep(1)

print "Test 4:  Mix protocols in SD30 Motion conf H263/H264"
print "---------------------------------------------------------"
confname = "Conf1024MotionMixProtocol"
confFile = 'Scripts/SD/AddVideoCpConf1024kMotion.xml'
dialInH323CapSet = "G711+H263+Fecc"
dialInSIPCapSet  = "FULL CAPSET" 
allocatedResources = (int(sd30resources)*3) + int(cif30resources)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes4,num_of_resource, allocatedResources)
sleep(1)

print "Test 5:  Mix protocols in 2CIF30 Motion conf H263/H264"
print "---------------------------------------------------------"
confname = "Conf384MotionMixProtocol"
confFile = 'Scripts/SD/AddVideoCpConf384kMotion.xml'
dialInH323CapSet = "G711+H263+Fecc"
dialInSIPCapSet  = "G711+H263+Fecc"
allocatedResources = (int(sd15_2cif30resources)* 2 + int(cif30resources)* 2)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes5,num_of_resource, allocatedResources)
sleep(1)

sleep(1)
r.Disconnect()

sleep(1)
c.Disconnect()
