#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_30SD.xml"
##################################################################
# In this test we configure the resources to SD30 resources only 
# And try to connect CP calls that consume more resources, 
# The excected result is that the call will be max SD30
##################################################################
from McmsConnection import *
import os
#-----------------------------------------------------------------------------
status_fail = 0
status_ok = 1
#------------------------------------------------------------------------------

def ConfWithOneVideoParty(connection, confname, confFile, protocol, direction, dialInCapSetName = "FULL CAPSET"):
    connection.LoadXmlFile(confFile)
    connection.ModifyXml("RESERVATION","NAME",confname)
    print "Adding Conf " + confname + "  ..."
    connection.Send()
    sleep(1)
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
    partyname = confname+"_"+direction+"_"+protocol
    partyip =  "1.2.3.1" 
    if(direction == "DialOut"):
        if(protocol == "H323"):
            #------Dial out H323 party
             connection.AddVideoParty(confid, partyname, partyip)
             print "Connecting H323 Dial out Party" 
             connection.WaitAllOngoingConnected(confid)
             connection.WaitAllOngoingNotInIVR(confid)
             
        if(protocol == "SIP"):   
             #------Dial out SIP party
            connection.AddVideoParty(confid, partyname, partyip, "true")
            print "Connecting SIP Dial out Party" 
            connection.WaitAllOngoingConnected(confid)
            connection.WaitAllOngoingNotInIVR(confid)
    
    if(direction == "DialIn"):
        if(protocol == "H323"):
            #-------Dial in H323 party
            connection.SimulationAddH323Party(partyname, confname,dialInCapSetName)
            connection.SimulationConnectH323Party(partyname)
            print "Connecting H323 Dial in Party"
            connection.WaitAllOngoingConnected(confid)
            connection.WaitAllOngoingNotInIVR(confid)
        if(protocol == "SIP"):  
            #-------Dial in SIP party
            connection.SimulationAddSipParty(partyname, confname,dialInCapSetName)
            connection.SimulationConnectSipParty(partyname)
            print "Connecting SIP Dial in Party" 
            connection.WaitAllOngoingConnected(confid)
            connection.WaitAllOngoingNotInIVR(confid) 
    
    sleep(1)
    
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
       
#--------------------------------------------------------------------------------
     

c = McmsConnection()
c.Connect()

print "----------------- HD720 Resources Tests -------------------" 
print "\n---------------------------------------"
print "Test HD720 30 Resource Deficiency - ONLY SD resources"
print "---------------------------------------"

print "Test 1. 1024k Sharpness Conf Enough Resources For SD30 "
print "--------------------------------------------------------"

confname = "Conf4096Sharpness"
confFile = 'Scripts/SD/AddVideoCpConf4096kSharpness.xml'
TestResourcesReportPortsType(c, "SD", 30, 0, 30, 10)
ConfWithOneVideoParty(c, confname, confFile, "H323", "DialOut")
TestResourcesReportPortsType(c, "SD", 30, 1, 29, 10)
c.DeleteAllConf()
sleep(1)

print "Test 2. 1920k Motion Conf Enough Resources For SD30 "
print "--------------------------------------------------------"
sleep(20)
confname = "Conf1920Motion"
confFile = 'Scripts/SD/AddVideoCpConf1920kMotion.xml'
TestResourcesReportPortsType(c, "SD", 30, 0, 30, 10)
ConfWithOneVideoParty(c, confname, confFile, "H323", "DialOut")
TestResourcesReportPortsType(c, "SD", 30, 1, 29, 10)
c.DeleteAllConf()
sleep(1)

sleep(1)
c.Disconnect()

