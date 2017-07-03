#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1


from McmsConnection import *
from ContentFunctions import *
import string 
import sys

#------------------------------------------------------------------------------
def TestSimDeadCard(connection):
    
    boardId = int(sys.argv[1])
    subBoardId = int(sys.argv[2])
        
    print "Board Id: " + str(boardId) + " SubBoard Id: " + str(subBoardId)
    
    
    #connection.SendXmlFile('Scripts/SimRemoveCardEvent.xml')
    c.LoadXmlFile('Scripts/SimRemoveCardEvent.xml')
    c.ModifyXml("REMOVE_CARD_EVENT", "BOARD_ID", boardId)
    c.ModifyXml("REMOVE_CARD_EVENT", "SUB_BOARD_ID", subBoardId)
    c.ModifyXml("REMOVE_CARD_EVENT", "IS_HANDLE_REMOVE", "false")
    c.Send()    
    print ">>> Sim Dead Card"
   
   
    print ">>> Sleep for 30 seconds"
    sleep(30)
        
        
    #connection.SendXmlFile('Scripts/SimInsertCardEvent.xml')
    c.LoadXmlFile('Scripts/SimInsertCardEvent.xml')
    c.ModifyXml("INSERT_CARD_EVENT", "BOARD_ID", boardId)
    c.ModifyXml("INSERT_CARD_EVENT", "SUB_BOARD_ID", subBoardId)    
    c.Send()    
    print ">>> Sim Insert Card"
     
    
    return

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestSimDeadCard(c)
c.Disconnect()

#------------------------------------------------------------------------------
