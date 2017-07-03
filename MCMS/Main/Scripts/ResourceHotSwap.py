#!/mcms/python/bin/python
#############################################################################

#############################################################################
# Script which tests the HotSwap Flow. 
# By Ohad.
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_20
#-LONG_SCRIPT_TYPE

#-- SKIP_ASSERTS

import os
from ResourceUtilities import *
from PSTN_MaxCapacity import *
from ReconnectParty import *

numRetries = 20

connection = ResourceUtilities()
connection.Connect()

isMPmMode = connection.IsMPMMode()

## Creating conf with 4 voip parties and 12 pstn parties and connect them
num_of_voip_parties = 4
num_of_pstn_parties = 12
confid = connection.SimpleCreateConfAndParties(num_of_voip_parties)

connection.LoadXmlFile("Scripts/PSTN_Party.xml")
for x in range(num_of_pstn_parties):
    partyname = "Pstn"+str(x+1)
    phone="3333"+str(x+1)
    connection.AddPSTN_DialoutParty(confid,partyname,phone)

connection.WaitAllOngoingConnected(confid,numRetries)

## Check which board has master AC
boardId = GetMasterAudioCntrlrBoardId()
print "Master Audio Controller is on board " + str(boardId)

## Check capacity
capacityMFABeforeRemove = GetCapacity( boardId, 1, 0xFFFF)
capacityRTMBeforeRemove = GetCapacity( boardId, 2, 0xFFFF)

print "Before Remove : Capacity on MFA on board " + boardId + " = " + str(capacityMFABeforeRemove) + " (Promil)"
print "Before Remove : Capacity on RTM on board " + boardId + " = " + str(capacityRTMBeforeRemove) + " (Parties)"

if ( boardId == "1"):
	otherBoardId = "2"
else:
	otherBoardId = "1"
	
capacityOtherMFABeforeRemove = GetCapacity( otherBoardId, 1, 0xFFFF)
capacityOtherRTMBeforeRemove = GetCapacity( otherBoardId, 2, 0xFFFF)

print "Before Remove : Capacity on MFA on board " + str(otherBoardId) + " = " + str(capacityOtherMFABeforeRemove) + " (Promil)"
print "Before Remove : Capacity on RTM on board " + str(otherBoardId) + " = " + str(capacityOtherRTMBeforeRemove) + " (Parties)"

## Remove card with Master AC 
connection.SimRemoveCard(boardId)

## Wait untill remove process finishes
sleep(5) 

## Check capacity
capacityMFAAfterRemove = GetCapacity( boardId, 1, 0xFFFF)
capacityRTMAfterRemove = GetCapacity( boardId, 2, 0xFFFF)

print "After Remove : Capacity on MFA on board " + boardId + " = " + str(capacityMFAAfterRemove) + " (Promil)"
print "After Remove : Capacity on RTM on board " + boardId + " = " + str(capacityRTMAfterRemove) + " (Parties)"

capacityOtherMFAAfterRemove = GetCapacity( otherBoardId, 1, 0xFFFF)
capacityOtherRTMAfterRemove = GetCapacity( otherBoardId, 2, 0xFFFF)

print "After Remove : Capacity on MFA on board " + str(otherBoardId) + " = " + str(capacityOtherMFAAfterRemove) + " (Promil)"
print "After Remove : Capacity on RTM on board " + str(otherBoardId) + " = " + str(capacityOtherRTMAfterRemove) + " (Parties)"

## Check that on removed board there is 0 capacity and on the
## other card the capacity stayed the same
if ( capacityMFAAfterRemove <> 0 ):
	print capacityMFAAfterRemove
	sys.exit("error : capacity on MFA on removed card isn't 0")

if ( capacityRTMAfterRemove <> 0 ):
	sys.exit("error : capacity on RTM on removed card isn't 0")
	
if ( capacityOtherMFABeforeRemove <> capacityOtherMFAAfterRemove ):
	sys.exit("error : capacity on stayed MFA after removing a card is different than before")

if ( capacityOtherRTMBeforeRemove <> capacityOtherRTMAfterRemove ):
	sys.exit("error : capacity on stayed RTM after removing a card is different than before")

## Check that there are no units left on MFA
connection.LoadXmlFile('Scripts/ResourceDetailParty/CardDetailReq.xml')
connection.ModifyXml("GET","BOARD_NUMBER",boardId)
connection.Send()   
unit_summary = connection.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
unit_len =len(unit_summary)
if(unit_len != 0):
	sys.exit("error: There are still some units on MFA on board number " +  str(boardId) + " number of units left: " + str(unit_len))
print "All units removed from MFA"

## Check that there are no units left on RTM
connection.ModifyXml("GET","SUB_BOARD_ID",2)
connection.Send()   
unit_summary = connection.xmlResponse.getElementsByTagName("UNIT_RESOURCE_DESCRIPTOR")
unit_len =len(unit_summary)
if(unit_len != 0):
	sys.exit("error: There are still some units on RTM on board number " +  str(boardId) + " number of units left: " + str(unit_len))
print "All units removed from RTM"

## Check that AC is now on other board
newMasterBoardId = GetMasterAudioCntrlrBoardId()
print "New Master Audio Controller After remove is on board " + str(newMasterBoardId)
if (boardId == newMasterBoardId):
	sys.exit("error: Master Audio Controller didn't move to the other card")
	
## Insert card again (depending on card mode)
if(isMPmMode == 1) :
	connection.SimInsertCard(3,boardId)
else:	
	connection.SimInsertCard(9,boardId)

## Insert RTM
print "Reinserting RTM on board " + str(boardId)
connection.LoadXmlFile('Scripts/SimInsertCardEvent.xml')
connection.ModifyXml("INSERT_CARD_EVENT","BOARD_ID",boardId)
connection.ModifyXml("INSERT_CARD_EVENT","SUB_BOARD_ID",2)
connection.Send()  

## Wait untill insert process finishes and parties are disconnected after the remove process (the disconnacting process takes very long)
sleep(100)

## Reconnect all parties
ConnectDisconnectAllParties(connection,confid,"true")

connection.WaitAllOngoingConnected(confid,numRetries)

## Check resources
capacityMFAAfterReinsert = GetCapacity( boardId, 1, 0xFFFF)
capacityRTMAfterReinsert = GetCapacity( boardId, 2, 0xFFFF)

print "After Reinsert : Capacity on MFA on board " + boardId + " = " + str(capacityMFAAfterReinsert) + " (Promil)"
print "After Reinsert : Capacity on RTM on board " + boardId + " = " + str(capacityRTMAfterReinsert) + " (Parties)"

if ( capacityMFAAfterReinsert <> capacityMFABeforeRemove ):
	sys.exit("error : capacity on MFA after reinserting a card is different than before removal")

if ( capacityRTMAfterReinsert <> capacityRTMBeforeRemove ):
	sys.exit("error : capacity on RTM after reinserting a card is different than before removal")
	
connection.DeleteConf(confid)   
connection.WaitAllConfEnd()

connection.Disconnect()
