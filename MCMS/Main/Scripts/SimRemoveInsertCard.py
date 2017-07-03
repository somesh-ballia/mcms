#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1


from McmsConnection import *
from ContentFunctions import *
import string 
import sys

#------------------------------------------------------------------------------
def TestSimRemoveInsertCard(connection):
    
    list = sys.argv[1:]
    listSize = len(sys.argv[1:])    
    #print list
    #print listSize    
       
    if listSize < 2:
        print "Error in number of arguments: " + str(listSize)
        return
    
    index = 0
        
    currentBoardId = list[index]
    index = index + 1
    currentSubBoardId = list[index]
    index = index + 1
    
    #print currentBoardId
    #print currentSubBoardId
    #print listSize/2
    
    for i in range(listSize/2):

        print ">>> Sim Remove Card: " + str(currentBoardId) + " " + str(currentSubBoardId)
        c.LoadXmlFile('Scripts/SimRemoveCardEvent.xml')
        c.ModifyXml("REMOVE_CARD_EVENT", "BOARD_ID", currentBoardId)
        c.ModifyXml("REMOVE_CARD_EVENT", "SUB_BOARD_ID", currentSubBoardId)    
        c.Send()    
        #print ">>> Sim Remove Card"
       
       
        #print ">>> Sleep for 2 seconds"
        sleep(2)
        
        print ">>> Sim Insert Card: " + str(currentBoardId) + " " + str(currentSubBoardId)
        c.LoadXmlFile('Scripts/SimInsertCardEvent.xml')
        c.ModifyXml("INSERT_CARD_EVENT", "BOARD_ID", currentBoardId)
        c.ModifyXml("INSERT_CARD_EVENT", "SUB_BOARD_ID", currentSubBoardId)    
        c.Send()    
        #print ">>> Sim Insert Card"
        
        #print ">>> Sleep for 2 seconds"
        sleep(2)
        
        
        if index == (listSize):   
            print "Done"
            return 
        
        currentBoardId = list[index]
        index = index + 1        
        currentSubBoardId = list[index]
        index = index + 1
        
        print ">>> currentBoardId: " + str(currentBoardId)
        print ">>> currentSubBoardId: " + str(currentSubBoardId)

 
          
    return

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestSimRemoveInsertCard(c)
c.Disconnect()

#------------------------------------------------------------------------------
