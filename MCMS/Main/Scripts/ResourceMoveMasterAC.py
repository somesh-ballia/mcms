#!/mcms/python/bin/python
#################################################################
#  Script tests the moving Master AC system behaviour  
#################################################################
# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6

# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=Logger Resource GideonSim EndpointsSim MplApi CSApi

from ResourceUtilities import *

#################################################################
def SetACunitStatus(boardId, status):
    print "Send Set unit status to BOARD_ID:" + str(boardId) + ", UNIT_ID:14, STATUS:" + str(status) + " to influence keep alive indication..."	 ## ( (status == 3) ? "(eFatal)" : "(eOk)" ) 
    c.LoadXmlFile('Scripts/keep_allive/SetUnitStatus.xml') 
    c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","BOARD_ID",boardId)
    c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","UNIT_ID",14)
    c.ModifyXml("SET_UNIT_STATUS_FOR_KEEP_ALIVE","STATUS",status)
    c.Send()
    sleep (10)

#################################################################

c = ResourceUtilities()
c.Connect()

## Display Reserved Audio Controllers 
os.system("Bin/McuCmd ShowReservedAC Resource")

numOfBoards = 4

for index in range(numOfBoards-1):
    boardId = GetMasterAudioCntrlrBoardId() ## Check which board has master AC
    print "\nMaster Audio Controller is on board " + str(boardId)
    SetACunitStatus(boardId, 3) ## Disable AC unit on this board


## Check which board has master AC
boardID = GetMasterAudioCntrlrBoardId()
print "\nMaster Audio Controller is on board " + str(boardID) + "\n"


## Remove card with Master AC 
c.SimRemoveCard(boardID)

## Wait untill remove process finishes
sleep(5) 

## Check which board has master AC => NOTE: must be 0xFFFF
print "Master Audio Controller is on board " + str(GetMasterAudioCntrlrBoardId()) + "\n"

boardID = 1

## Enable some AC unit
SetACunitStatus(boardID, 0)

## Check which board has master AC
boardID = GetMasterAudioCntrlrBoardId() ## must be boardID=1
print "Master Audio Controller is on board " + str(boardID) + "\n"

## Remove card with Master AC 
c.SimRemoveCard(boardID)

## Check which board has master AC => on this step it must be 0xFFFF
print "Master Audio Controller is on board " + str(GetMasterAudioCntrlrBoardId()) + "\n"

## Wait untill remove process finishes
sleep(5) 

## Insert RTM
c.SimInsertCard(9,boardID)

## Wait untill insert process finishes
sleep(5)

boardID = GetMasterAudioCntrlrBoardId() ## Check which board has master AC => should be boardID=1
print "Master Audio Controller is on board " + str(boardID) + "\n"

print "wait 10 seconds..."
sleep (10)

c.Disconnect()
