#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"

from McmsConnection import *
from ResourceUtilities import *
import os

#-----------------------------------------------------------------------------
cif30resources = 1
sd15_2cif30resources = 2
sd30resources = 4
#-----------------------------------------------------------------------
def ConfWithOneVideoParty(connection, resourceUtility, confname, confFile, protocol, direction, totalNumResource, numResources,deleteConf, dialInCapSetName = "FULL CAPSET"):
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
    print "Verify that the resources that were allocated are: " + str(numResources)
    sleep(2)
    resourceUtility.TestFreeCarmelParties( int(totalNumResource) - int(numResources))
    
    if(deleteConf=="YES"):
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
freeresources = r.GetTextUnder("RSRC_REPORT_RMX","FREE")
print "free resources: " + str(freeresources)
r.TestFreeCarmelParties(int(num_of_resource))



print "----------------- SD Resources Tests -------------------" 
print "\n---------------------------------------"
print "Test 1: Resource Deficiency"
print "---------------------------------------"

print "Test 1.1: 512k Sharpness Conf Enough Resources For SD15 "
print "--------------------------------------------------------"
sleep(20)
command_line = "Bin/McuCmd DisableBoard Resource 1 YES"
os.system(command_line)
sleep(1)
command_line = "Bin/McuCmd DisableBoard Resource 2 YES"
os.system(command_line)
sleep(1)
command_line = "Bin/McuCmd DisableUnit Resource 1 1 NO"
os.system(command_line)
sleep(1)
command_line = "Bin/McuCmd DisableUnit Resource 1 11 NO"
os.system(command_line)
sleep(1)
confname = "Res_Def_SD15"
confFile = 'Scripts/SD/AddVideoCpConf512kSharpness.xml'
ConfWithOneVideoParty(c, r, confname, confFile, "H323", "DialOut", num_of_resource, sd15_2cif30resources, "YES")
sleep(1)

r.TestFreeCarmelParties(int(num_of_resource))
print "Test 1.2: 1024k Motion Conf Enough Resources For 2CIF30 "
print "-----------------------------------------------------"
command_line = "Bin/McuCmd DisableBoard Resource 1 YES"
os.system(command_line)
command_line = "Bin/McuCmd DisableBoard Resource 2 YES"
os.system(command_line)
command_line = "Bin/McuCmd DisableUnit Resource 1 1 NO"
os.system(command_line)
command_line = "Bin/McuCmd DisableUnit Resource 1 11 NO"
os.system(command_line)
confname = "Res_Def_2CIF30"
confFile = 'Scripts/SD/AddVideoCpConf1024kMotion.xml'
ConfWithOneVideoParty(c, r, confname, confFile, "H323", "DialIn", num_of_resource, sd15_2cif30resources,"YES")
sleep(1)
r.TestFreeCarmelParties(int(num_of_resource))
print "Test 1.3: 512k Sharpness Conf Enough Resources For CIF30 Party"
print "--------------------------------------------------------------"
command_line1 = "Bin/McuCmd DisableBoard Resource 1 YES"
os.system(command_line1)
command_line2 = "Bin/McuCmd DisableBoard Resource 2 YES"
os.system(command_line2)
command_line3 = "Bin/McuCmd DisableUnit Resource 1 1 NO"
os.system(command_line3)
command_line4 = "Bin/McuCmd DisableUnit Resource 1 11 NO"
os.system(command_line4)
confname = "Res_Def_CIF30"
confFile = 'Scripts/SD/AddVideoCpConf512kSharpness.xml'
#changed remote cap to H.261 in order to keep just CIF port (and not H.263 4cif) - (Eitan 11/2008)  
ConfWithOneVideoParty(c, r, confname, confFile, "SIP", "DialIn", num_of_resource, cif30resources ,"NO","H261+ALL")
confname = "Res_Def_CIF30_2"
ConfWithOneVideoParty(c, r, confname, confFile, "SIP", "DialIn" , num_of_resource, (int(cif30resources)* 2),"NO")
c.DeleteAllConf()
sleep(2)

print "Test 1.4: 1024k Motion Conf Enough Resources For CIF30 Party"
print "-----------------------------------------------------------"
r.TestFreeCarmelParties(int(num_of_resource))
command_line = "Bin/McuCmd DisableBoard Resource 1 YES";
os.system(command_line)
command_line = "Bin/McuCmd DisableBoard Resource 2 YES";
os.system(command_line)
command_line = "Bin/McuCmd DisableUnit Resource 1 1 NO"
os.system(command_line)
command_line = "Bin/McuCmd DisableUnit Resource 1 11 NO"
os.system(command_line)
confname = "Res_Def_Motion_CIF30"
confFile = 'Scripts/SD/AddVideoCpConf1024kMotion.xml'
#changed remote cap to H.261 in order to keep just CIF port (and not H.263 4cif) - (Eitan 11/2008)
ConfWithOneVideoParty(c, r, confname, confFile, "SIP", "DialIn", num_of_resource, cif30resources ,"NO","H261+ALL")
confname = "Res_Def_CIF30_2"
ConfWithOneVideoParty(c, r, confname, confFile, "H323", "DialOut" , num_of_resource, (int(cif30resources)* 2),"NO")
c.DeleteAllConf()
sleep(1)

print "enable the resources"
command_line = "Bin/McuCmd DisableBoard Resource 1 NO";
os.system(command_line)
command_line = "Bin/McuCmd DisableBoard Resource 2 NO";
os.system(command_line)

print "\n---------------------------------------"
print "Test 2: ReAllocate"
print "---------------------------------------"

print "Test 2.1: 512k Sharpness Conf Connect party that supports SD15 "
print "--------------------------------------------------------------"
confname = "Realloc_SD30_To_SD15"
confFile = 'Scripts/SD/AddVideoCpConf512kSharpness.xml'
ConfWithOneVideoParty(c, r, confname, confFile, "H323", "DialIn", num_of_resource, sd15_2cif30resources, "YES","H264(sd15)+ALL")
sleep(1)

print "Test 2.2: 1024k Motion Conf Connect party that supports 2CIF30 "
print "-------------------------------------------------------------"
confname = "Realloc_SD30_To_2CIF30"
confFile = 'Scripts/SD/AddVideoCpConf1024kMotion.xml'
ConfWithOneVideoParty(c, r, confname, confFile, "H323", "DialIn", num_of_resource, sd15_2cif30resources,"YES","H264(2cif30)+ALL")
sleep(1)

print "Test 2.3: 512k Sharpness Conf Connect party that supports CIF30 "
print "----------------------------------------------------------------"
confname = "Realloc_SD30_To_CIF30_Sharp"
confFile = 'Scripts/SD/AddVideoCpConf512kSharpness.xml'
#changed remote cap to H.261 in order to keep just CIF port (and not H.263 4cif) - (Eitan 11/2008)
ConfWithOneVideoParty(c, r, confname, confFile, "H323", "DialIn", num_of_resource, cif30resources,"YES","H261+ALL")
sleep(1)

print "Test 2.4: 1024k Motion Conf Connect party that supports CIF30 "
print "-------------------------------------------------------------"
confname = "Realloc_SD30_To_CIF30_Motion"
confFile = 'Scripts/SD/AddVideoCpConf1024kMotion.xml'
#changed remote cap to H.261 in order to keep just CIF port (and not H.263 4cif) - (Eitan 11/2008)
ConfWithOneVideoParty(c, r, confname, confFile, "H323", "DialIn", num_of_resource, cif30resources,"YES","H261+ALL")
sleep(1)


print "Test 2.5: 256k Sharpness Conf Connect party that supports CIF30 "
print "---------------------------------------------------------------"
confname = "Realloc_SD15_To_CIF30"
confFile = 'Scripts/SD/AddVideoCpConf256kSharpness.xml'
#changed remote cap to H.261 in order to keep just CIF port (and not H.263 4cif) - (Eitan 11/2008)
ConfWithOneVideoParty(c, r, confname, confFile, "H323", "DialIn", num_of_resource, cif30resources,"YES","H261+ALL")
sleep(1)

print "Test 2.6: 384k Motion Conf Connect party that supports CIF30 "
print "--------------------------------------------------------------"
confname = "Realloc_2CIF30_To_CIF30"
confFile = 'Scripts/SD/AddVideoCpConf384kMotion.xml'
#changed remote cap to H.261 in order to keep just CIF port (and not H.263 4cif) - (Eitan 11/2008)
ConfWithOneVideoParty(c, r, confname, confFile, "H323", "DialIn", num_of_resource, cif30resources,"YES","H261+ALL")
sleep(1)

print "\n-------------------------------------------------------------------"
print "Test 3: SIP DialIn with capabilitis lower then conference video mode"
print "--------------------------------------------------------------------"

print "Test 3.1: 512k Sharpness Conf Connect SIP Dial Party that supports SD15 "
print "-----------------------------------------------------------------------"
confname = "SIPIN_SD15"
confFile = 'Scripts/SD/AddVideoCpConf512kSharpness.xml'
ConfWithOneVideoParty(c, r, confname, confFile, "SIP", "DialIn", num_of_resource, sd15_2cif30resources, "YES","H264(sd15)+ALL")
sleep(1)
print "Test 3.2: 1024k Motion Conf Connect party that supports 2CIF30 "
print "-------------------------------------------------------------"
confname = "SIPIN_2CIF30"
confFile = 'Scripts/SD/AddVideoCpConf1024kMotion.xml'
ConfWithOneVideoParty(c, r, confname, confFile, "SIP", "DialIn", num_of_resource, sd15_2cif30resources,"YES","H264(2cif30)+ALL")
sleep(1)

print "Test 3.3: 512k Sharpness Conf Connect party that supports CIF30 "
print "----------------------------------------------------------------"
confname = "SD30CONFSharp_SIPINCIF30"
onfFile = 'Scripts/SD/AddVideoCpConf512kSharpness.xml'
#changed remote cap to H.261 in order to keep just CIF port (and not H.263 4cif) - (Eitan 11/2008)
ConfWithOneVideoParty(c, r, confname, confFile, "SIP", "DialIn", num_of_resource, cif30resources,"YES","H261+ALL")
sleep(1)

print "Test 3.4: 1024k Motion Conf Connect party that supports CIF30 "
print "-------------------------------------------------------------"
confname = "SD30CONFMotion_SIPINCIF30"
confFile = 'Scripts/SD/AddVideoCpConf1024kMotion.xml'
ConfWithOneVideoParty(c, r, confname, confFile, "SIP", "DialIn", num_of_resource, cif30resources,"YES","H264(cif)+ALL")
sleep(1)


print "Test 3.5: 256k Sharpness Conf Connect party that supports CIF30 "
print "----------------------------------------------------------------"
confname = "SD15CONFSharp_SIPINCIF30"
confFile = 'Scripts/SD/AddVideoCpConf256kSharpness.xml'
#changed remote cap to H.261 in order to keep just CIF port (and not H.263 4cif) - (Eitan 11/2008)
ConfWithOneVideoParty(c, r, confname, confFile, "SIP", "DialIn", num_of_resource, cif30resources,"YES","H261+ALL")
sleep(1)

r.Disconnect()
sleep(1)
c.Disconnect()
