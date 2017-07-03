#!/mcms/python/bin/python
#############################################################################

#############################################################################
# Script which tests the HotSwap Flow of the new card MPMRX. 
# By Rachel.
#############################################################################

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_20
#-LONG_SCRIPT_TYPE

#-- SKIP_ASSERTS

import os
from ResourceUtilities import *
from PSTN_MaxCapacity import *
from ReconnectParty import *

connection = ResourceUtilities()
connection.Connect()


## Check which board has master AC
boardId = GetMasterAudioCntrlrBoardId()
print "Master Audio Controller is on board " + str(boardId)


## Remove card with Master AC 
print "Before Remove :...................................................."
connection.SimRemoveCard(boardId)

## Wait untill remove process finishes
sleep(5) 



print "insert card MPMRX :...................................................."
	

connection.SimInsertCard(15,boardId)

# Insert RTM
print "Reinserting RTM on board " + str(boardId)
connection.LoadXmlFile('Scripts/SimInsertCardEvent.xml')
connection.ModifyXml("INSERT_CARD_EVENT","BOARD_ID",boardId)
connection.ModifyXml("INSERT_CARD_EVENT","SUB_BOARD_ID",2)
connection.Send()  

## Wait untill insert process finishes and parties are disconnected after the remove process (the disconnacting process takes very long)
sleep(5)



connection.Disconnect()
