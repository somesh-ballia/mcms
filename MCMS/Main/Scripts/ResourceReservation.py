#!/mcms/python/bin/python

#############################################################################
#Script which tests reservation feature
#it will also run under valgrind
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_4

import os
from ResourceUtilities import *

c = ResourceUtilities()
c.Connect()

profId = c.AddProfile("profile_for_reservations")
print "-----------------------------------------------------------"
print "Adding a reservation with numeric id (1234)"
t = datetime.utcnow( ) 
deltat = timedelta(0,0,0,0,30,1,0)
t = t + deltat
c.CreateRes("test1", profId, t, "1234")
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list is " + str(res_number)
if res_number != 1:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Unexpected number of reservations")
print "-----------------------------------------------------------"
print "Trying to add another reservation, at the same time, with the same numeric id (1234)"
c.CreateRes("test2", profId, t, "1234", 0, 0, "Conference ID is in use")
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list is " + str(res_number)
if res_number != 1:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Unexpected number of reservations")
print "-----------------------------------------------------------"
print "Trying to add another reservation, at a different time, with the same numeric id (1234)"
deltat = timedelta(0,0,0,0,30,1,0)
t = t + deltat
c.CreateRes("test2", profId, t, "1234")
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list is " + str(res_number)
if res_number != 2:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Unexpected number of reservations")
print "-----------------------------------------------------------"
strMeetingRoomName = "MeetingRoom"
print "Adding a new MR with numeric id (6789)"
MRId = c.CreateMRWithNumericId(strMeetingRoomName, profId, "6789")
print "-----------------------------------------------------------"
print "Trying to add another reservation, with the same numeric id as the meeting room (6789)"
c.CreateRes("test3", profId, t, "6789", 0, 0, "Conference ID is in use")
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list is " + str(res_number)
if res_number != 2:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Unexpected number of reservations")
print "-----------------------------------------------------------" 
print "Deleting the MR with numeric id (6789)"
c.DelReservation(MRId, 'Scripts//AddRemoveMrNew/DeleteMR.xml')
print "-----------------------------------------------------------" 
print "Trying again to add a reservation, with the same numeric id as the meeting room that was now deleted (6789)"
res_id = c.CreateRes("test3", profId, t, "6789")
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list is " + str(res_number)
if res_number != 3:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Unexpected number of reservations")
print "-----------------------------------------------------------"    
print "Trying to add a new MR with the same numeric id (6789)"
c.CreateMRWithNumericId(strMeetingRoomName, profId, "6789", "Conference ID is in use")
print "-----------------------------------------------------------"  
print "Trying to update reservation - changing it's NID of the reservation from 6789 to 1234 - this should fail"
c.UpdateReservation("test3",res_id,"1234",profId,t,0,0,"Conference ID is in use")
print "Indeed failed" 
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list is " + str(res_number)
if res_number != 3:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Unexpected number of reservations")
print "-----------------------------------------------------------"  
print "Updating reservation - Changing the NID of the reservation from 6789 to 9876 - this should suceed"
c.UpdateReservation("test3",res_id,"9876",profId,t)
print "Succeeded"
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list is " + str(res_number)
if res_number != 3:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Unexpected number of reservations")
print "-----------------------------------------------------------"  
print "Trying again to add a new MR with the same numeric id (6789). This should succeed since we changed the reservation"
c.CreateMRWithNumericId(strMeetingRoomName, profId, "6789")
print "-----------------------------------------------------------"  
print "Delete the reservation"
c.DelConfReservation(res_id)
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list is " + str(res_number)
if res_number != 2:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Unexpected number of reservations")
print "-----------------------------------------------------------"  
print "Adding a reservation that should start now"
t = datetime.utcnow( ) 
res_id = c.CreateRes("start_now", profId, t)
sleep(1)
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list is " + str(res_number)
if res_number != 2:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Unexpected number of reservations")
conf_number = c.GetNumOfConferencesInList()
print "Number of conferences in list is " + str(conf_number)
if conf_number != 1 :
   print "Error: unexpected number of conferences were found in the list, found "+ str(conf_number)+" conferences"
   c.Disconnect()                
   sys.exit("Unexpected number of conferences")
print "-----------------------------------------------------------"  
print "Adding a reservation that should start in a minute"
t = datetime.utcnow( ) 
deltat = timedelta(0,0,0,0,1,0,0)
t = t + deltat
res_id = c.CreateRes("start_in_a_minute", profId, t)
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list is " + str(res_number)
if res_number != 3:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Unexpected number of reservations")
conf_number = c.GetNumOfConferencesInList()
print "Number of conferences in list is " + str(conf_number)
if conf_number != 1 :
   print "Error: unexpected number of conferences were found in the list, found "+ str(conf_number)+" conferences"
   c.Disconnect()                
   sys.exit("Unexpected number of conferences")
print "Waiting a little more than a minute for the reservation to start"   
sleep(70)
c.Connect()
res_number = c.GetNumOfReservationsInList()
print "Number of reservations in list is " + str(res_number)
if res_number != 2:
   print "Error: unexpected number of reservations were found in the list, found "+ str(res_number)+" reservations"
   c.Disconnect()                
   sys.exit("Unexpected number of reservations")
conf_number = c.GetNumOfConferencesInList()
print "Number of conferences in list is " + str(conf_number)
if conf_number != 2 :
   print "Error: unexpected number of conferences were found in the list, found "+ str(conf_number)+" conferences"
   c.Disconnect()                
   sys.exit("Unexpected number of conferences")
print "-----------------------------------------------------------" 
targetEqName = "testEQ"
eqPhone="3344"
c.CreatePSTN_EQ(targetEqName, eqPhone, 1)
eqId, eqNID = c.WaitMRCreated(targetEqName)
print "-----------------------------------------------------------"  
print "Adding dial-in PSTN party to the EQ, and move it to the MR"
partyname = "PSTN_party"
c.SimulationAddPSTNParty(partyname,eqPhone)
sleep(1)
c.SimulationConnectPSTNParty(partyname)
print "Waiting for the EQ to wake up"
numOfParties = 1
eqConfId = c.WaitUntillEQorMRAwakes(targetEqName,numOfParties,10,True)

MRNumericId = "6789"
print "sending DTMF from party " + partyname + " to move from EQ To MR: " + MRNumericId
c.SimulationH323PartyDTMF(partyname, MRNumericId)
MRConfId = c.WaitConfCreated(strMeetingRoomName,10)
c.WaitAllOngoingConnected(MRConfId)
sleep(2)
print "Party indeed moved to MR"
c.DeletePSTNPartyFromSimulation(partyname)
c.WaitAllOngoingDisConnected(MRConfId)
print "Party has been deleted. Waiting 1 minute for MR to auto-terminate"
sleep(70)
c.Connect()
c.WaitConfEnd(MRConfId)
print "Also deleting ongoing EQ"
c.DeleteConf(eqConfId)
c.WaitConfEnd(eqConfId)
print "-----------------------------------------------------------"  

c.Disconnect()

