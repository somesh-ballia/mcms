#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_5AUDIO_5CIF30_5SD30_5HD72030.xml"

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
def TestConfrencesVideoModes(connection, confname, confFile, dialInH323CapSet, dialInSIPCapSet, sampleLayoutTypes,dialoutH323VideoAlgo, dialoutSipVideoAlgo):
    connection.LoadXmlFile(confFile)
    connection.ModifyXml("RESERVATION","NAME",confname)
    print "Adding Conf " + confname + "  ..."
    connection.Send()
    
    confid = connection.WaitConfCreated(confname)
    	
    #connect parties
    print "Start connecting Dial out Parties..."
    num_of_parties = 4   
    #------Dial out H323 party    
    partyname = confname+"_Party_1"
    partyip =  "1.2.3.1" 
    connection.AddVideoParty(confid, partyname, partyip,False, dialoutH323VideoAlgo)
    print "Connecting H323 Dial out Party" 
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    #------Dial out SIP party   
    partyname = confname+"_Party_2"
    partyip =  "1.2.3.2" 
    connection.AddVideoParty(confid, partyname, partyip, "true", dialoutSipVideoAlgo)
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

  
#----------------------------------------------------------------------
def TestResourcesReportPortsType(connection, testedResourcePortType, expectedTotalPorts, expectedOccupiedPorts, expectedFreePorts, numRetries):
    i = 0
    for i in range(numRetries):
        connection.SendXmlFile("Scripts/Resource2Undef3RealParties/ResourceMonitorCarmel.xml")
        resourceType = connection.GetTextUnder("RSRC_REPORT_RMX","RSRC_REPORT_ITEM",i)
        if resourceType!="":
            if resourceType==testedResourcePortType:
                #print "resourceType = " + resourceType
                totalPorts = connection.GetTextUnder("RSRC_REPORT_RMX","TOTAL",i)
                occupiedPorts = connection.GetTextUnder("RSRC_REPORT_RMX","OCCUPIED",i)
                freePorts = connection.GetTextUnder("RSRC_REPORT_RMX","FREE",i)
                if expectedTotalPorts!=int(totalPorts) or expectedOccupiedPorts!= int(occupiedPorts) or expectedFreePorts != int(freePorts):
                    print "Tested Resource Port TYPE = " + resourceType
                    print "----------------------------------------------"
                    print "total ports = " + str(totalPorts) + ", expected total ports = " + str(expectedTotalPorts)
                    print "occupied ports = " + str(occupiedPorts) + ", expected occupied ports = " + str(expectedOccupiedPorts)
                    print "free ports = " + str(freePorts) + ", expected free ports = " + str(expectedFreePorts)
                    print connection.xmlResponse.toprettyxml(encoding="utf-8")
                    connection.Disconnect()                
                    connection.ScriptAbort("Abort!")
                    
                else:
                    print "TestResourcesReportPortsType OK resourceType = " + resourceType
                    return
    print "numRetries = " + str(numRetries)
    print "-----------Num retries excided--------------------------"
    sys.stdout.write(".")
    sys.stdout.flush()   
 
#-----------------------------------------------------------------------
c = McmsConnection()
c.Connect()
os.system("Bin/McuCmd set mcms IS_DOUBLE_DSP YES")
#-------Verify that all the resources are free---------
TestResourcesReportPortsType(c, "CIF", 5, 0, 5, 10)
TestResourcesReportPortsType(c, "SD", 5, 0, 5, 10)
TestResourcesReportPortsType(c, "HD720", 5, 0, 5, 10)

print "\n---------------------------------------"
print "Test MIX Video Protocols H261/H263/H264 MPM+ "
print "---------------------------------------"

print "Test 1: Mix protocols in HD720 Sharpness conf H263/H264"
print "---------------------------------------------------------"
confname = "Conf1920SharpMixProtocol"
confFile = 'Scripts/SD/AddVideoCpConf1920kSharpness.xml'
dialOutH323VidProtocol = "auto"
dialOutSipVidProtocol = "h263"
dialInH323CapSet = "FULL CAPSET"
dialInSIPCapSet  = "G711+H263+Fecc" #H263 CIF SIP EpSim sends the same cap as the RMX


TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes1, dialOutH323VidProtocol, dialOutSipVidProtocol)
TestResourcesReportPortsType(c, "CIF", 5, 1, 4, 10)
#h263 4cif = sd30 resources in MPM+ mode
TestResourcesReportPortsType(c, "SD", 5, 1, 4, 10)
TestResourcesReportPortsType(c, "HD720", 5, 2, 3, 10)
c.DeleteAllConf()
sleep(1)

print "Test 2:  Mix protocols in SD30 Sharpness conf H263/H264"
print "---------------------------------------------------------"
confname = "Conf384SharpMixProtocol"
confFile = 'Scripts/SD/AddVideoCpConf384kSharpness.xml'
dialOutH323VidProtocol = "h261"
dialOutSipVidProtocol = "auto"
dialInH323CapSet = "G711+H263+Fecc"
dialInSIPCapSet  = "FULL CAPSET"
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes2,dialOutH323VidProtocol,dialOutSipVidProtocol)
TestResourcesReportPortsType(c, "CIF", 5, 1, 4, 10)
#h263 4cif = sd30 resources in MPM+ mode
TestResourcesReportPortsType(c, "SD", 5, 3, 2, 10)
TestResourcesReportPortsType(c, "HD720", 5, 0, 5, 10)
c.DeleteAllConf()
sleep(1)

print "Test 3:  Mix protocols in CIF30 Sharpness conf H263/H264"
print "---------------------------------------------------------"
confname = "Conf128SharpMixProtocol"
confFile = 'Scripts/SD/AddVideoCpConf128kSharpness.xml'
dialOutH323VidProtocol = "auto"
dialOutSipVidProtocol = "auto"
dialInH323CapSet = "G711+H263+Fecc"
dialInSIPCapSet  = "G711+H263+Fecc"
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes3,dialOutH323VidProtocol,dialOutSipVidProtocol)
TestResourcesReportPortsType(c, "CIF", 5, 4, 1, 10)
TestResourcesReportPortsType(c, "SD", 5, 0, 5, 10)
TestResourcesReportPortsType(c, "HD720", 5, 0, 5, 10)
c.DeleteAllConf()
sleep(1)

print "Test 4:  Mix protocols in HD720 Motion conf H263/H264"
print "---------------------------------------------------------"
confname = "Conf1920MotionMixProtocol"
confFile = 'Scripts/SD/AddVideoCpConf1920kMotion.xml'
dialOutH323VidProtocol = "h263"
dialOutSipVidProtocol = "auto"
dialInH323CapSet = "G711+H263+Fecc"
dialInSIPCapSet  = "FULL CAPSET" 
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes4,dialOutH323VidProtocol,dialOutSipVidProtocol)
TestResourcesReportPortsType(c, "CIF", 5, 2, 3, 10)
TestResourcesReportPortsType(c, "SD", 5, 0, 5, 10)
TestResourcesReportPortsType(c, "HD720", 5, 2, 3, 10)
c.DeleteAllConf()
sleep(1)

print "Test 5:  Mix protocols in SD60 Motion conf H263/H264"
print "---------------------------------------------------------"
confname = "Conf1024MotionMixProtocol"
confFile = 'Scripts/SD/AddVideoCpConf1024kMotion.xml'
dialOutH323VidProtocol = "auto"
dialOutSipVidProtocol = "auto"
dialInH323CapSet = "H261+ALL"
dialInSIPCapSet  = "G711+H263+Fecc"
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes5,dialOutH323VidProtocol,dialOutSipVidProtocol)
TestResourcesReportPortsType(c, "CIF", 5, 2, 3, 10)
TestResourcesReportPortsType(c, "SD", 5, 0, 5, 10)
TestResourcesReportPortsType(c, "HD720", 5, 2, 3, 10)
c.DeleteAllConf()


sleep(1)
c.Disconnect()
