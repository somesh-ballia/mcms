#!/mcms/python/bin/python

#############################################################################
#Script which tests all the conference part when there's no scheduler
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*export RESOURCE_SETTING_FILE="VersionCfg/Resource_Setting_80Video.xml"
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

import os
from ResourceUtilities import *
from SysCfgUtils import *

print "-----------------------------------------------------------"
print "Updating system cfg flag of resource scheduler to NO"
r = SyscfgUtilities()
r.Connect()
r.UpdateSyscfgFlag("MCMS_PARAMETERS_USER","INTERNAL_SCHEDULER","NO","user")
r.Disconnect()
os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")
print "-----------------------------------------------------------"
c = ResourceUtilities()
c.Connect()
profId = c.AddProfile("profile")
print "-----------------------------------------------------------"
print "Trying to get reservation list"
c.SendXmlFile('Scripts/AddRemoveReservation/TransResList.xml',"Illegal status")
print "Indeed got Illegal status"
print "-----------------------------------------------------------"
print "Trying to add a reservation in the future, it should start immediately. It has NID 1234"
t = datetime.utcnow( ) 
deltat = timedelta(0,0,0,0,30,1,0)
t = t + deltat
conf_id = c.CreateRes("test1", profId, t, "1234")
c.WaitConfCreated("test1")
conf_number = c.GetNumOfConferencesInList()
print "Number of conferences in list is " + str(conf_number)
if conf_number != 1:
   print "Error: unexpected number of conferences were found in the list, found "+ str(conf_number)+" conferences"
   c.Disconnect()                
   sys.exit("Unexpected number of conferences")
print "-----------------------------------------------------------"
print "Trying to add another 'reservation', at a different time, with the same numeric id (1234) - this should fail because there's no scheduling so all reservation start immediately"
deltat = timedelta(0,0,0,0,30,1,0)
t = t + deltat
c.CreateRes("test2", profId, t, "1234", 0, 0, "Conference ID is in use")
conf_number = c.GetNumOfConferencesInList()
print "Number of conferences in list is " + str(conf_number)
if conf_number != 1:
   print "Error: unexpected number of conferences were found in the list, found "+ str(conf_number)+" conferences"
   c.Disconnect()                
   sys.exit("Unexpected number of conferences")
print "-----------------------------------------------------------"
strMeetingRoomName = "Meeting1"
print "Adding a new MR with numeric id (6789)"
MRId = c.CreateMRWithNumericId(strMeetingRoomName, profId, "6789")
print "-----------------------------------------------------------"
strMeetingRoomName = "Meeting2"
print "Trying to add a new MR with the same numeric id (6789)"
c.CreateMRWithNumericId(strMeetingRoomName, profId, "6789", "Conference ID is in use")
print "-----------------------------------------------------------"
print "Trying to add another conference, with the same numeric id as the meeting room (6789)"
c.CreateRes("test3", profId, t, "6789", 0, 0, "Conference ID is in use")
conf_number = c.GetNumOfConferencesInList()
print "Number of conferences in list is " + str(conf_number)
if conf_number != 1:
   print "Error: unexpected number of conferences were found in the list, found "+ str(conf_number)+" conferences"
   c.Disconnect()                
   sys.exit("Unexpected number of conferences")
print "-----------------------------------------------------------" 
print "Deleting the MR with numeric id (6789)"
c.DelReservation(MRId, 'Scripts//AddRemoveMrNew/DeleteMR.xml')
print "-----------------------------------------------------------" 
print "Trying again to add a conference, with the same numeric id as the meeting room that was now deleted (6789)"
conf_id = c.CreateRes("test3", profId, t, "6789")
c.WaitConfCreated("test3")
conf_number = c.GetNumOfConferencesInList()
print "Number of conferences in list is " + str(conf_number)
if conf_number != 2:
   print "Error: unexpected number of conferences were found in the list, found "+ str(conf_number)+" conferences"
   c.Disconnect()                
   sys.exit("Unexpected number of conferences")
print "-----------------------------------------------------------"    
print "Trying to add a new MR with the same numeric id (6789)"
c.CreateMRWithNumericId(strMeetingRoomName, profId, "6789", "Conference ID is in use")
print "-----------------------------------------------------------"  
print "Deleting conference with NID 6789"
c.DeleteConf(conf_id)
c.WaitConfEnd(conf_id)
conf_number = c.GetNumOfConferencesInList()
print "Number of conferences in list is " + str(conf_number)
if conf_number != 1:
   print "Error: unexpected number of conferences were found in the list, found "+ str(conf_number)+" conferences"
   c.Disconnect()                
   sys.exit("Unexpected number of conferences")
print "-----------------------------------------------------------"  
print "Trying again to add a new MR with the same numeric id (6789). This should succeed since we terminated the conference"
strMeetingRoomName = "Meeting3"
c.CreateMRWithNumericId(strMeetingRoomName, profId, "6789")
print "-----------------------------------------------------------"  
print "Adding dial-in H323 party to the EQ, and move it to the MR"
partyname = "H323_Party"
c.SimulationAddH323Party(partyname,"DefaultEQ")
sleep(1)
c.SimulationConnectH323Party(partyname)
print "Waiting for the EQ to wake up"
numOfParties = 1
eqConfId = c.WaitUntillEQorMRAwakes("DefaultEQ",numOfParties,10,True)
MRNumericId = "6789"
print "sending DTMF from party to move from EQ To MR: " + MRNumericId
c.SimulationH323PartyDTMF(partyname, MRNumericId)
MRConfId = c.WaitConfCreated(strMeetingRoomName,10)
c.WaitAllOngoingConnected(MRConfId)
print "Party indeed moved to MR"
c.DeletePSTNPartyFromSimulation(partyname)
#print "Party has been deleted. Waiting 1 minute for MR to auto-terminate"
c.DeleteConf(MRConfId)
c.WaitConfEnd(MRConfId)
print "Also deleting ongoing EQ"
c.DeleteConf(eqConfId)
c.WaitConfEnd(eqConfId)

c.Disconnect()

