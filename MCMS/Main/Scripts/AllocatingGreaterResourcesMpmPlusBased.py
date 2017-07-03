#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BARAK.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpmPlus.txt"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_160_YES_PstnYES_v100.0.cfs"
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_10HD720.xml"

################################################################################################################
# In this test we allocate only HD720 resources and we connect participants that actually consume less resources 
################################################################################################################
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
       
    print "-----------Num retries excided--------------------------"
       
#--------------------------------------------------------------------------------
     

c = McmsConnection()
c.Connect()

print "----------------- Allocating greater resources MPM+  -------------------" 
print "------------------------------------------------------------------------\n"

print "Test 1. 4096k Sharpness connect H264 CIF party"
print "--------------------------------------------------------"

confname = "Conf4096Sharpness"
confFile = 'Scripts/SD/AddVideoCpConf4096kSharpness.xml'
TestResourcesReportPortsType(c, "HD720", 10, 0, 10, 10)
ConfWithOneVideoParty(c, confname, confFile, "H323", "DialIn", "H264(cif)+ALL")
TestResourcesReportPortsType(c, "HD720", 10, 1, 9, 10)
c.DeleteAllConf()
sleep(2)


print "Test 2. 256k Sharpness connect dial out H323 party"
print "--------------------------------------------------------"
confname = "Conf256Sharpness"
confFile = 'Scripts/SD/AddVideoCpConf256kSharpness.xml'
TestResourcesReportPortsType(c, "HD720", 10, 0, 10, 10)
ConfWithOneVideoParty(c, confname, confFile, "H323", "DialOut")
TestResourcesReportPortsType(c, "HD720", 10, 1, 9, 10)
c.DeleteAllConf()
sleep(2)

print "Test 3. 1024k Motion connect SIP Audio Only party"
print "--------------------------------------------------------"
confname = "Conf1024Motion"
confFile = 'Scripts/SD/AddVideoCpConf1024kMotion.xml'
TestResourcesReportPortsType(c, "HD720", 10, 0, 10, 10)
ConfWithOneVideoParty(c, confname, confFile, "SIP", "DialIn", "AudioOnly")
sleep(2)
TestResourcesReportPortsType(c, "HD720", 10, 1, 9, 10)
c.DeleteAllConf()
sleep(2)

c.Disconnect()

