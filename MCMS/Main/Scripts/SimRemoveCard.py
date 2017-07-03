#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1


from McmsConnection import *
from ContentFunctions import *
import string 
import sys

#------------------------------------------------------------------------------
def TestSimRemoveCard(connection):
    
    boardId = int(sys.argv[1])
    subBoardId = int(sys.argv[2])
        
    print "<Remove Card> Board Id: " + str(boardId) + " SubBoard Id: " + str(subBoardId)
    
    
    c.LoadXmlFile('Scripts/SimRemoveCardEvent.xml')
    c.ModifyXml("REMOVE_CARD_EVENT", "BOARD_ID", boardId)
    c.ModifyXml("REMOVE_CARD_EVENT", "SUB_BOARD_ID", subBoardId)    
    c.Send()    
    print ">>> Sim Remove Card"   
    
   
    return

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestSimRemoveCard(c)
c.Disconnect()

#------------------------------------------------------------------------------
