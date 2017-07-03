#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1


from McmsConnection import *
from ContentFunctions import *
from HotSwapUtils import *
import string 
import os
import sys

    
#------------------------------------------------------------------------------
def TestSimRemoveCard(connection):    
   
    HSUtil = HotSwapUtils()
    
    IsCardExist = False
    IsCardExist = HSUtil.IsCardExistInHW_List(1, 1, 'normal')
    
    if IsCardExist == True:
        HSUtil.SimRemoveCard(1, 1)        
        sleep(2)
        
        IsCardRemoved = HSUtil.IsCardRemovedInHW_List(1, 1)
        if IsCardRemoved == True:
            print "Card removed"
        else:
            print "Card has not removed"
        
        sleep(2)
        
        IsCardExist = HSUtil.IsCardExistInHW_List(1, 1, 'major_error')
        if IsCardExist == True:
            print "Card exist"
        else:
            print "Card does not exist"
        
            
   
    return

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestSimRemoveCard(c)
c.Disconnect()

#------------------------------------------------------------------------------
