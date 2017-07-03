#!/mcms/python/bin/python

# For list of profiles look at RunTest.sh
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_1
#*export MPL_SIM_FILE="VersionCfg/MPL_SIM_2.XML"

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
	   
def RunCommand(commandToRun):
    print "----- run " + commandToRun
    os.system(commandToRun)
def num_of_running_cards_tasks(card_task_name,num_of_task):
    temp_str= str("Bin/McuCmd @tasks cards > tasts.tmp")
    os.system(temp_str)
    command = "cat tasts.tmp | grep MfaTask | wc -l > tests1.tmp" 
    RunCommand(command)
    command = "cat tests1.tmp"
    RunCommand(command)
    output = open("tests1.tmp")
    num_of_Tasks = output.readline()
    output.close()
    num_of_Tasks_i = int(num_of_Tasks)
    print "num_of_Tasks"+str(num_of_Tasks_i)
    if (num_of_Tasks_i<>num_of_task):
       print "wrong card_tasks_number"
       exit (1)

#------------------------------------------------------------------------------
def TestSimInsertCard(connection):
    
    HSUtil = HotSwapUtils()
    
    IsCardExist = False
    IsCardExist = HSUtil.IsCardExistInHW_List(1, 1, 'normal')
    num_of_running_cards_tasks("MfaTask",2)
    
    if IsCardExist == True:
    
        print ">>> Sim Insert Card"
        HSUtil.SimInsertCard(1, 1) 
        num_of_running_cards_tasks("MfaTask",2)
        IsCardExist = HSUtil.IsCardExistInHW_List(1, 1, 'normal')
        test_if_Alert_exist(connection,"Resource","INSUFFICIENT_RESOURCES",1)
        
    
    
       
    
    return

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

TestSimInsertCard(c)
#c.Disconnect()

#------------------------------------------------------------------------------
