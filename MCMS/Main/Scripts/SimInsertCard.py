#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1


from McmsConnection import *
from ContentFunctions import *
import string 
import sys

#------------------------------------------------------------------------------
def TestSimInsertCard(connection):
    
    boardId = int(sys.argv[1])
    subBoardId = int(sys.argv[2])
        
    print "<Insert Card> Board Id: " + str(boardId) + " SubBoard Id: " + str(subBoardId)
    
    
    c.LoadXmlFile('Scripts/SimInsertCardEvent.xml')
    c.ModifyXml("INSERT_CARD_EVENT", "BOARD_ID", boardId)
    c.ModifyXml("INSERT_CARD_EVENT", "SUB_BOARD_ID", subBoardId)    
    c.Send()    
    print ">>> Sim Insert Card"   
    
   
    return

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestSimInsertCard(c)
c.Disconnect()

#------------------------------------------------------------------------------
