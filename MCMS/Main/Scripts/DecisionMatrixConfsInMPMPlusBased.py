#!/mcms/python/bin/python

#-LONG_SCRIPT_TYPE

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_5AUDIO_5CIF30_5SD30_5HD72030_5HD1080.xml"

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
#resources types
cif30Resources = 1
sd30Resources = 4
hd720Resources = 8
hd1080AsymmetricResources = 12
#-----------------------------------------------------------------------------
status_fail = 0
status_ok = 1
#------------------------------------------------------------------------------
def TestConfrencesVideoModes(connection, confname, confFile, dialInH323CapSet, dialInSIPCapSet, sampleLayoutTypes):#,totalNumResource, numResources):
    connection.LoadXmlFile(confFile)
    connection.ModifyXml("RESERVATION","NAME",confname)
    print "Adding Conf " + confname + "  ..."
    connection.Send()
    
    print "Wait untill Conf create...",
    num_retries = 50
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
        sleep(2)
 
#-----------------------------------------------------------------------
def TestResourcesReportPortsType(connection,  testedResourcePortType, expectedTotalPorts, expectedOccupiedPorts, expectedFreePorts, numRetries):
	status = status_ok
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


                    
				else:
					print "TestResourcesReportPortsType OK resourceType = " + resourceType
					return status_ok
        print "i = " + str(i)
        print "numRetries = " + str(numRetries)
   	print "-----------Num retries excided--------------------------"
   	
          
#-----------------------------------------------------------------------
c = McmsConnection()
c.Connect()
os.system("Bin/McuCmd set mcms IS_DOUBLE_DSP YES")
#-------Verify that all the resources are free---------
TestResourcesReportPortsType(c, "CIF", 5, 0, 5, 10)
TestResourcesReportPortsType(c, "SD", 5, 0, 5, 10)
TestResourcesReportPortsType(c, "HD720", 5, 0, 5, 10)
TestResourcesReportPortsType(c, "HD1080", 5, 0, 5, 10)

print "----------------- Decision Matrix test for MPM+ based system -------------------" 
print "---------------------------------------"
print "Test1: HD1080 30fps Sharpness"
print "--------------------------------------"
confname = "Conf4096Sharpness"
confFile = 'Scripts/SD/AddVideoCpConf4096kSharpness.xml'
dialInH323CapSet = "FULL CAPSET"
dialInSIPCapSet  = "FULL CAPSET"
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes1)
TestResourcesReportPortsType(c, "HD1080", 5, 3, 2, 10)
TestResourcesReportPortsType(c, "HD720", 5, 1, 4, 10)#sip dial out in asymmetric call sends the same caps as RMX (SD30)
c.DeleteAllConf()
sleep(3)
TestResourcesReportPortsType(c, "HD720", 5, 0, 5, 10)
sleep(1)

print "---------------------------------------"
print "Test2: HD720 60fps Motion"
print "--------------------------------------"
confname = "Conf1920Motion"
confFile = 'Scripts/SD/AddVideoCpConf1920kMotion.xml'
dialInH323CapSet = "FULL CAPSET"
dialInSIPCapSet  = "FULL CAPSET"
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes1)
TestResourcesReportPortsType(c, "HD1080", 5, 3, 2, 10)
TestResourcesReportPortsType(c, "HD720", 5, 1, 4, 10)#sip dial out in asymmetric call sends the same caps as RMX (SD30)
c.DeleteAllConf()
sleep(3)
TestResourcesReportPortsType(c, "HD720", 5, 0, 5, 10)
sleep(1)


print "---------------------------------------"
print "Test3: HD720 30fps Sharpness"
print "--------------------------------------"
confname = "Conf1920Sharpness"
confFile = 'Scripts/SD/AddVideoCpConf1920kSharpness.xml'
dialInH323CapSet = "FULL CAPSET"
dialInSIPCapSet  = "H264(hd720)+ALL"
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes1)
TestResourcesReportPortsType(c, "HD720", 5, 4, 1, 10)
c.DeleteAllConf()
sleep(3)
TestResourcesReportPortsType(c, "HD720", 5, 0, 5, 10)
sleep(1)


print "\n-------------------------------------"
print "Test4: WSD60 60fps Motion"
print "--------------------------------------"
confname = "Conf1024Motion"
confFile = 'Scripts/SD/AddVideoCpConf1024kMotion.xml'
dialInH323CapSet = "H264(hd720)+ALL"
dialInSIPCapSet  = "FULL CAPSET"
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet, sampleLayoutTypes2)
TestResourcesReportPortsType(c, "HD720", 5, 4, 1, 10)
c.DeleteAllConf()
sleep(3)
TestResourcesReportPortsType(c, "HD720", 5, 0, 5, 10)
sleep(1)


print "\n-------------------------------------"
print "Test5: SD 30fps Sharpness"
print "--------------------------------------"
confname = "Conf256Sharpness"
confFile = 'Scripts/SD/AddVideoCpConf256kSharpness.xml'
dialInH323CapSet = "G711+H264sd+Fecc"
dialInSIPCapSet  = "H264(sd30)+ALL"
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes3)
TestResourcesReportPortsType(c, "SD", 5, 4, 1, 10)
c.DeleteAllConf()
sleep(3)
TestResourcesReportPortsType(c, "SD", 5, 0, 5, 10)
sleep(1)


print "\n-------------------------------------"
print "Test6: WCIF 60fps Motion"
print "--------------------------------------"
confname = "Conf7684Motion_2"
confFile = 'Scripts/SD/AddVideoCpConf768kMotion.xml'
dialInH323CapSet = "H264(sd30)+ALL"
dialInSIPCapSet = "FULL CAPSET"
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes5)
TestResourcesReportPortsType(c, "SD", 5, 4, 1, 10)
c.DeleteAllConf()
sleep(3)
TestResourcesReportPortsType(c, "SD", 5, 0, 5, 10)
sleep(1)

print "\n-------------------------------------"
print"Test7: CIF 60fps Motion"
print "-------------------------------------------------------------------"
confname = "Conf256Motion"
confFile = 'Scripts/SD/AddVideoCpConf256kMotion.xml'
dialInH323CapSet = "H264(sd30)+ALL"
dialInSIPCapSet  = "H264(sd30)+ALL"
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes6)
TestResourcesReportPortsType(c, "SD", 5, 4, 1, 10)
c.DeleteAllConf()
sleep(3)
TestResourcesReportPortsType(c, "SD", 5, 0, 5, 10)
sleep(1)


print "\n-------------------------------------"
print "Test8: CIF30 Sharpness"
print "--------------------------------------"
confname = "Conf128Sharpness"
confFile = 'Scripts/SD/AddVideoCpConf128kSharpness.xml'
dialInH323CapSet = "H264(cif)+ALL"
dialInSIPCapSet  = "G711+H264cif+H263+Fecc"
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes7)
TestResourcesReportPortsType(c, "CIF", 5, 4, 1, 10)
c.DeleteAllConf()
sleep(3)
TestResourcesReportPortsType(c, "CIF", 5, 0, 5, 10)
sleep(1)

print "\n-------------------------------------"
print "Test9: CIF30 Motion"
print "--------------------------------------"
confname = "Conf128Motion"
confFile = 'Scripts/SD/AddVideoCpConf128kMotion.xml'
dialInH323CapSet = "H264(cif)+ALL"
dialInSIPCapSet  = "H264(2cif30)+ALL"
TestConfrencesVideoModes(c, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes8)
TestResourcesReportPortsType(c, "CIF", 5, 4, 1, 10)
c.DeleteAllConf()
sleep(1)
TestResourcesReportPortsType(c, "CIF", 5, 0, 5, 10)


sleep(1)
c.Disconnect()
