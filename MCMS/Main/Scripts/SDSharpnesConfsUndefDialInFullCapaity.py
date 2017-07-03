#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#*PROCESSES_NOT_FOR_VALGRIND=ConfParty
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"
#*export LICENSE_FILE="VersionCfg/Keycodes_9251aBc471_40_YES_v100.0.cfs"
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_mpm.txt"

from McmsConnection import *
from ResourceUtilities import *
import os
import sys

#-----------------------------------------------------------------------------
cif30resources = 1
sd15_2cif30resources = 2
sd30resources = 4
#---------------------------------------------------------------------------------
def UndefinedDialIn(connection,r, num_of_parties,num_retries,conf_path,confName,totalNumResource, numResources):
    #confName = confname#"undefConf"
    connection.CreateConf(confName, conf_path)
    confid = connection.WaitConfCreated(confName,num_retries)

    ### Add parties to EP Sim and connect them
    for x in range(num_of_parties):
        partyname = confName+"Party"+str(x+1)
        connection.SimulationAddH323Party(partyname, confName)
        
    for y in range(num_of_parties):
        partyname = confName+"Party"+str(y+1) 
        connection.SimulationConnectH323Party(partyname)
        sleep(2)        
         
    connection.WaitAllPartiesWereAdded(confid,num_of_parties,num_retries*num_of_parties)
    connection.WaitAllOngoingConnected(confid,num_retries*num_of_parties)
    print "Sleep 2"
    sleep(2)
    print "Verify that the resources that were allocated are: " + str(numResources)
    r.TestFreeCarmelParties( int(totalNumResource) - int(numResources))
    
   
   
    party_id_list = [0]*num_of_parties 
    connection.LoadXmlFile('Scripts/UndefinedDialIn/TransConf2.xml')
    connection.ModifyXml("GET","ID",confid)
    connection.Send()
    ongoing_party_list = connection.xmlResponse.getElementsByTagName("ONGOING_PARTY")
    if len(ongoing_party_list) < num_of_parties:
        errMsg= "some parties are lost, find only " +str(len(ongoing_party_list)) + " parties in conf"
        sys.exit(errMsg )
    for index in range(num_of_parties):
        party_id_list[(num_of_parties-index)-1]=ongoing_party_list[index].getElementsByTagName("ID")[0].firstChild.data

    for index in range(num_of_parties):
        partyId = int(party_id_list[index])
        partyIdByParty = partyId+1
        partyName = confName+"Party"+str(partyIdByParty)
        connection.Send()
        connection.SimulationDeleteH323Party(partyName)
        sleep(1)

    sleep(1)
    return confid  
#------------------------------------------------------------------------------


command_line = 'Bin/McuCmd set ConfParty DELAY_BETWEEN_DIAL_OUT_PARTY 0'
os.system(command_line)
sleep(1)
c = McmsConnection()
c.Connect()

os.system("Bin/McuCmd set mcms IS_DOUBLE_DSP YES")
#-------Verify that all the resources are free---------
r = ResourceUtilities()
r.Connect()
r.SendXmlFile("Scripts/Resource2Undef3RealParties/ResourceMonitorCarmel.xml")
num_of_resource = r.GetTotalCarmelParties()
freeresources = r.GetTextUnder("RSRC_REPORT_RMX","FREE")
print "free resource " + str(freeresources)
r.TestFreeCarmelParties(int(num_of_resource))

print "\n-------------------------------------"
print "    Undefined Dial In Full Capacity "
print "---------------------------------------"

#print "Test1: CIF 30 Sharpness"
#print "---------------------------------------"
#numberOfParties = 40
#timeuot = 5
#numRetries = 10
#confFile = 'Scripts/SD/AddVideoCpConf128kSharpness.xml'
#confName = "CIF30Sharp"
#confid = UndefinedDialIn(c,r,numberOfParties,numRetries,confFile,confName)

#c.DeleteConf(confid)
#c.WaitAllConfEnd(20 * timeuot )

#print "---------------------------------------"
print "Test1: SD 15 Sharpness"
print "---------------------------------------"
numberOfParties = 20
numRetries = 10
timeuot = 5
confFile = 'Scripts/SD/AddVideoCpConf256kSharpness.xml'
confName = "SD15Sharp"
confid = UndefinedDialIn(c,r, numberOfParties,numRetries,confFile,confName,num_of_resource, int(sd15_2cif30resources) * int(numberOfParties))

c.DeleteConf(confid)
c.WaitAllConfEnd(20 * timeuot )

print "---------------------------------------"
print "Test2: SD 30 Sharpness"
print "---------------------------------------"
numberOfParties = 10
timeuot = 5
numRetries = 10
confFile = 'Scripts/SD/AddVideoCpConf512kSharpness.xml'
confName = "SD30Sharp"
confid = UndefinedDialIn(c,r,numberOfParties,numRetries,confFile,confName, num_of_resource,int(sd30resources) * int(numberOfParties))

c.DeleteConf(confid)
c.WaitAllConfEnd(20 * timeuot )

c.Disconnect()
