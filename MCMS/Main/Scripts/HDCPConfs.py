#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1

#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_BREEZE.XML" 
#*export SYSTEM_CARDS_MODE_FILE="VersionCfg/SystemCardsMode_breeze.txt" 

from McmsConnection import *
from ResourceUtilities import *
from ISDNFunctions import *
from HDFunctions import *
import os

#------------------------------------------------------------------------------
sampleLayoutTypes1 = "1x1", "1and5","2x1", "1x2Ver","1and2Hor","4x4","1and8Central","2and8"
sampleLayoutTypes2 = "1x2", "3x3","2x2", "1x2Hor","1and4Hor","1and7","1and3Ver", "1and12"

#-----------------------------------------------------------------------------
hdCpResources = 4

#------------------------------------------------------------------------------
def TestConfrencesVideoModes(connection,resourceUtility, confname, confFile, dialInH323CapSet, dialInSIPCapSet, sampleLayoutTypes,totalNumResource, numResources):

    delay = 2
    if(connection.IsProcessUnderValgrind("ConfParty")):
    	delay = 4

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
    num_of_parties = 5   
    #------Dial out H323 party    
    partyname = confname+"_Party_1"
    partyip =  "1.2.3.1" 
    connection.AddVideoParty(confid, partyname, partyip)
    print "Connecting H323 Dial out Party" 
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    sleep(delay)
    #------Dial out SIP party   
    partyname = confname+"_Party_2"
    partyip =  "1.2.3.2" 
    connection.AddVideoParty(confid, partyname, partyip, "true")
    print "Connecting SIP Dial out Party" 
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    sleep(delay)
    #-------Dial in H323 party
    partyname = confname+"_Party_3"
    connection.SimulationAddH323Party(partyname, confname,dialInH323CapSet)
    connection.SimulationConnectH323Party(partyname)
    print "Connecting H323 Dial in Party -" + dialInH323CapSet
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    sleep(delay)
    #-------Dial in SIP party
    partyname = confname+"_Party_4"
    connection.SimulationAddSipParty(partyname, confname,dialInSIPCapSet)
    connection.SimulationConnectSipParty(partyname)
    print "Connecting SIP Dial in Party" + dialInSIPCapSet
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)
    sleep(delay)
    #-------Dial in ISDN party
    #add a new profile    
    connection.LoadXmlFile("Scripts/CreateNewProfile.xml")
    profile_name = "Profile1920"
    profile_rate = "1920"
    if  confname == "HDCPSharpness1024":
    	profile_name = "Profile1024"
    	profile_rate = "1024"  	
    
    
    connection.ModifyXml("RESERVATION","NAME", profile_name)
    connection.ModifyXml("RESERVATION","TRANSFER_RATE", profile_rate)
    connection.Send()
    ProfId = connection.GetTextUnder("RESERVATION","ID")    
    
    #create the target Conf and wait until it connected
    targetEqName = "IsdnEQ"
    eqPhone="3344"
    connection.CreatePSTN_EQ(targetEqName,eqPhone,ProfId)
    eqId, eqNID = connection.WaitMRCreated(targetEqName)
    
    print "Connecting ISDN Dial in Party"
    partyname = confname+"_Party_5"
    SimulationAddIsdnParty(connection,partyname,eqPhone,"30")
    SimulationConnectIsdnParty(connection,partyname)    
    sleep(delay)

    retries = 50
    eqConfId = connection.WaitUntillEQorMRAwakes(targetEqName, 1, retries, True)    
    connection.WaitUntilAllPartiesConnected(eqConfId, 1, retries)
        
    #send the TDMF to the EPsim with the numeric id of the target conf
    targetConfNumericId = connection.GetConfNumericId(confid)
    connection.SimulationH323PartyDTMF(partyname, targetConfNumericId)    
    
    #Sleeping for IVR messages
    print "Sleeping until IVR finishes..."
    sleep(5)
    
    connection.WaitAllOngoingConnected(confid)
    connection.WaitAllOngoingNotInIVR(confid)    
    sleep(delay)

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
    
    #remove the EQ Reservation and Profile
    connection.DeleteConf(eqConfId)
    connection.WaitConfEnd(eqConfId)
    connection.DelReservation(eqId, "Scripts/AwakeMrByUndef/RemoveMr.xml")
    connection.DelProfile(ProfId)
    
    
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



print "----------------- HDCP Tests -------------------" 
command_line = "Bin/McuCmd set ConfParty MAX_CP_RESOLUTION HD" + '\n'
os.system(command_line)

print "---------------------------------------"
print "Test1: HDCP Motion"
print "--------------------------------------"
confname = "HDCPMotion1920"
confFile = 'Scripts/HDCP/AddVideoCpConf1920kMotion.xml'
dialInH323CapSet = "FULL CAPSET"
dialInSIPCapSet  = "FULL CAPSET"
allocatedResources = (int(hdCpResources)*5)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet,sampleLayoutTypes1,num_of_resource, allocatedResources)
sleep(1)


print "\n-------------------------------------"
print "Test2: HDCP Sharpness"
print "--------------------------------------"
confname = "HDCPSharpness1024"
confFile = 'Scripts/HDCP/AddVideoCpConf1024kSharpness.xml'
dialInH323CapSet = "FULL CAPSET"
dialInSIPCapSet  = "FULL CAPSET"
allocatedResources = (int(hdCpResources)*5)
TestConfrencesVideoModes(c, r, confname, confFile, dialInH323CapSet, dialInSIPCapSet, sampleLayoutTypes2,num_of_resource, allocatedResources)
sleep(1)


sleep(1)
r.Disconnect()

sleep(1)
c.Disconnect()
