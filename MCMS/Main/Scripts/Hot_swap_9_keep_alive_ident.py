#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2_WITHOUT_RTM.XML"
#-- SKIP_ASSERTS
#-LONG_SCRIPT_TYPE

from McmsConnection import *
from ContentFunctions import *
from HotSwapUtils import *

import string 
#------------------------------------------------------------------------------
def test_if_Alert_exist(self,ProcessName,expected_fault,expected_fault_retries_num,need_delay=0):
      print "<<< test_if_Alert_exis : "+ ProcessName +"("+str(expected_fault_retries_num) +")."
      SM_fail = 0
      fault_elm_list=0
      if need_delay == 1:
         sleep (20)
 #     self.SendXmlFile("Scripts/GetFaults.xml")
      self.SendXmlFile("Scripts/GetActiveAlarms.xml")
      fault_elm_list = self.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
      num_of_faults = len(fault_elm_list)
      for index in range(len(fault_elm_list)):
                fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
                print fault_desc
                if fault_desc == expected_fault:
                   SM_fail = SM_fail+1
      print "SM_fail" + str(SM_fail)
      if SM_fail <> expected_fault_retries_num :
	     print "Test failed,  "+expected_fault+" not found in Faults"
	   #  sys.exit(1)


#------------------------------------------------------------------------------
def TestSimInsertCard(connection):
   HSUtil = HotSwapUtils()
    
    IsCardExist = False
    IsCardExist = HSUtil.IsCardExistInHW_List(1, 1, 'normal')
    if IsCardExist == True:
            print "Good Start !!! Card exist"
    else:
            print "Bad Start !!! Card does not exist"
            exit (1)
    
    print ">>> Sim Remove Card"
    HSUtil.SimDeadCard(1, 1)        
    sleep(300)
        
    j = McmsConnection()
    j.Connect()
    test_if_Alert_exist(j,"Resource","INSUFFICIENT_RESOURCES",1) 
        
    
    IsCardRemoved = HSUtil.IsCardRemovedFromHW_List(1, 1) 
    if IsCardRemoved == True:
            print "Good!!! Card Removed"
    else:
            print "Bad!!! Card does not removed"
            exit(1)
    sleep (3)
    print ">>> Sim Insert Card"
    HSUtil.SimInsertCard(1, 1) 
    IsCardExist = HSUtil.IsCardExistInHW_List(1, 1, 'normal')
    test_if_Alert_exist(j,"Resource","INSUFFICIENT_RESOURCES",1)
    if IsCardExist == True:
            print "Test Passed!!! HotSwap done, the card is there again"
            exit (0)
    else:
            print "Test Failed!!! HotSwap done, the card is  not there"
            exit (1)
        
    return    
 

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestSimInsertCard(c)
#c.Disconnect()

#------------------------------------------------------------------------------
