#!/mcms/python/bin/python
from McmsTargetConnection import *


#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6

import os, string
import sys
import shutil

from SysCfgUtils import *
from UsersUtils import *
c = McmsTargetConnection()

c.Connect()

from McmsConnection import *

class test_recovery_policy_Utilities ( McmsConnection ):
#------------------------------------------------------------------------------
    def test_segment_fault(self,ProcessName,expected_fault,expected_fault_retries_num,need_delay=0):
        print " Segment fault Test ProcessName : "+ ProcessName +"("+str(expected_fault_retries_num) +")."
        SM_fail = 0
        fault_elm_list=0
        temp_str_dbg= str("Bin/McuCmd set McmsDaemon DEBUG_MODE NO")
        os.system(temp_str_dbg)
        temp_str= str("Bin/McuCmd segment_fault " +ProcessName)
        print temp_str
        #temp_str="Bin/McuCmd segment_fault CDR" 
        os.system(temp_str)
        #os.system("Bin/McuCmd segment_fault" +ProcessName+" ")
        if need_delay == 1:
           sleep (20)
        #if expected_fault_retries_num == 4:
        #   sleep (15)
        self.SendXmlFile("Scripts/GetFaults.xml")
        fault_elm_list = self.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
        num_of_faults = len(fault_elm_list)
        for index in range(len(fault_elm_list)):
		        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
		        print fault_desc
		        #if fault_desc == "TASK_FAILED":
		        if fault_desc ==expected_fault:
		           proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
		           print proc_name
		           if proc_name==ProcessName :
		              general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
		              print general_msg
		              #if general_msg=="Task failed : Manager" :
                              SM_fail = SM_fail+1
                              
        if SM_fail <> expected_fault_retries_num :
		    print "Test failed, TASK_FAILED not found in Faults"
		    sys.exit(1)
        #str_to_rm_core=str(" rm *core*RCVR*")
        #os.system(str_to_rm_core)
        #return(0)
#------------------------------------------------------------------------------
#------------------------------------------------------------------------------
    def test_segment_fault_dbg_mode_no(self,ProcessName,expected_fault,expected_fault_retries_num,need_delay=0):
        print " Segment fault Test ProcessName : "+ ProcessName +"("+str(expected_fault_retries_num) +")."
        SM_fail = 0
        fault_elm_list=0
        temp_str_dbg= str("Bin/McuCmd set McmsDaemon DEBUG_MODE NO")
        os.system(temp_str_dbg)
        temp_str= str("Bin/McuCmd segment_fault " +ProcessName)
        print temp_str
        #temp_str="Bin/McuCmd segment_fault CDR" 
        os.system(temp_str)
        #os.system("Bin/McuCmd segment_fault" +ProcessName+" ")
        if need_delay == 1:
           sleep (20)
        #if expected_fault_retries_num == 4:
        #   sleep (15)
        sleep (30)
        """
        self.SendXmlFile("Scripts/GetFaults.xml")
        fault_elm_list = self.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
        num_of_faults = len(fault_elm_list)
        for index in range(len(fault_elm_list)):
		        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
		        print fault_desc
		        #if fault_desc == "TASK_FAILED":
		        if fault_desc ==expected_fault:
		           proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
		           print proc_name
		           if proc_name==ProcessName :
		              general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
		              print general_msg
		              if general_msg=="Task failed : Manager" :
		                 SM_fail = SM_fail+1
        if SM_fail <> expected_fault_retries_num :
		    print "Test failed, TASK_FAILED not found in Faults"
		    sys.exit(1)
        #str_to_rm_core=str(" rm *core*RCVR*")
        #os.system(str_to_rm_core)
        #return(0)
        """
#------------------------------------------------------------------------------
    def test_process__failfault(self,ProcessName,expected_fault,expected_fault_retries_num,need_delay=0):
      print " test_process__fail fault ProcessName : "+ ProcessName +"("+str(expected_fault_retries_num) +")."
      SM_fail = 0
      fault_elm_list=0
        #temp_str= str("Bin/McuCmd segment_fault " +ProcessName)
        #print temp_str
        #os.system(temp_str)
        #os.system("Bin/McuCmd segment_fault" +ProcessName+" ")
      if need_delay == 1:
         sleep (20)
    #  if expected_fault_retries_num == 1:
    #     sleep (20)   
      self.SendXmlFile("Scripts/GetFaults.xml")
      fault_elm_list = self.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
      num_of_faults = len(fault_elm_list)
      for index in range(len(fault_elm_list)):
                fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
                print fault_desc
		        #if fault_desc == "PROCESS_FAILED":
                if fault_desc ==expected_fault:
		           proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
		           print proc_name
		           if proc_name=="McmsDaemon" :
		              general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
		              print general_msg
		              gnrl_msg_str="Process failed : "+ProcessName
		              #if general_msg ==gnrl_msg_str :
                              SM_fail = SM_fail+1
                              
      print "SM_fail" + str(SM_fail)
      if SM_fail <> expected_fault_retries_num :
	     print "Test failed,  "+expected_fault+" not found in Faults"
	     sys.exit(1)
      #return(0)  
#------------------------------------------------------------------------------
    def test_process__failfault_dbg_mode_no(self,ProcessName,expected_fault,expected_fault_retries_num,need_delay=0):
      print " test_process__fail fault ProcessName : "+ ProcessName +"("+str(expected_fault_retries_num) +")."
      SM_fail = 0
      fault_elm_list=0
        #temp_str= str("Bin/McuCmd segment_fault " +ProcessName)
        #print temp_str
        #os.system(temp_str)
        #os.system("Bin/McuCmd segment_fault" +ProcessName+" ")
      if need_delay == 1:
         sleep (20)
    #  if expected_fault_retries_num == 1:
    #     sleep (20)  
    """ 
      self.SendXmlFile("Scripts/GetFaults.xml")
      fault_elm_list = self.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
      num_of_faults = len(fault_elm_list)
      for index in range(len(fault_elm_list)):
                fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
                print fault_desc
		        #if fault_desc == "PROCESS_FAILED":
                if fault_desc ==expected_fault:
		           proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
		           print proc_name
		           if proc_name=="McmsDaemon" :
		              general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
		              print general_msg
		              gnrl_msg_str="Process failed : "+ProcessName
		              if general_msg ==gnrl_msg_str :
		                 SM_fail = SM_fail+1
      print "SM_fail" + str(SM_fail)
      if SM_fail <> expected_fault_retries_num :
	     print "Test failed,  "+expected_fault+" not found in Faults"
	     sys.exit(1)
      #return(0) 
    """ 

#------------------------------------------------------------------------------

#------------------------------------------------------------------------------
    def test_reset_mcu(self,ProcessName,expected_fault,expected_fault_retries_num,need_delay=0):
      print " test_process__fail fault ProcessName : "+ ProcessName +"("+str(expected_fault_retries_num) +")."
      SM_fail = 0
      fault_elm_list=0
        #temp_str= str("Bin/McuCmd segment_fault " +ProcessName)
        #print temp_str
        #os.system(temp_str)
        #os.system("Bin/McuCmd segment_fault" +ProcessName+" ")
      if need_delay == 1:
         sleep (20)
         #if expected_fault_retries_num == 4:
         #sleep (10)
      self.SendXmlFile("Scripts/GetFaults.xml")
      fault_elm_list = self.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
      num_of_faults = len(fault_elm_list)
      for index in range(len(fault_elm_list)):
                fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
                print fault_desc
                if fault_desc ==expected_fault:
		           proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
		           print proc_name
		           #if proc_name=="McmsDaemon" :
                   #general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
		           #print general_msg
		              #gnrl_msg_str="Process failed : "+ProcessName
		              #if general_msg ==gnrl_msg_str :
		           SM_fail = SM_fail+1
      if SM_fail <> expected_fault_retries_num :
	     print "Test failed, expected_fault"+expected_fault+"not found in Faults"
	     sys.exit(1)
      #return(0)   
      
 #------------------------------------------------------------------------------
 #------------------------------------------------------------------------------
    def test_reset_mcu_dbg_mode_no(self,ProcessName,expected_fault,expected_fault_retries_num,need_delay=0):
      print " test_process__fail fault ProcessName : "+ ProcessName +"("+str(expected_fault_retries_num) +")."
      SM_fail = 0
      fault_elm_list=0
        #temp_str= str("Bin/McuCmd segment_fault " +ProcessName)
        #print temp_str
        #os.system(temp_str)
        #os.system("Bin/McuCmd segment_fault" +ProcessName+" ")
      if need_delay == 1:
         sleep (20)
         #if expected_fault_retries_num == 4:
         #sleep (10)
      self.SendXmlFile("Scripts/GetFaults.xml")
      fault_elm_list = self.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
      num_of_faults = len(fault_elm_list)
      for index in range(len(fault_elm_list)):
                fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
                print fault_desc
                if fault_desc ==expected_fault:
		           proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
		           print proc_name
		           #if proc_name=="McmsDaemon" :
                   #general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
		           #print general_msg
		              #gnrl_msg_str="Process failed : "+ProcessName
		              #if general_msg ==gnrl_msg_str :
		           SM_fail = SM_fail+1
      if SM_fail <> expected_fault_retries_num :
	     print "Test failed, expected_fault"+expected_fault+"not found in Faults"
	     sys.exit(1)
      #return(0)   
      
 #------------------------------------------------------------------------------
    def test_kill_process(self,ProcessName,expected_fault,expected_fault_retries_num):
        print " Segment fault Test ProcessName : "+ ProcessName +"("+str(expected_fault_retries_num) +")."
        SM_fail = 0
        fault_elm_list=0
        temp_str_dbg= str("Bin/McuCmd set McmsDaemon DEBUG_MODE NO")
        os.system(temp_str_dbg)
        sleep(2)
        temp_str= str("Bin/McuCmd kill " +ProcessName)
        print temp_str
        #temp_str="Bin/McuCmd segment_fault CDR" 
        os.system(temp_str)
        #os.system("Bin/McuCmd segment_fault" +ProcessName+" ")
        #if expected_fault_retries_num == 4:
        sleep (20)
         #  self.SendXmlFile("Scripts/GetFaults.xml")
         #  sys.exit(0)
        self.SendXmlFile("Scripts/GetFaults.xml")
        fault_elm_list = self.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
        num_of_faults = len(fault_elm_list)
        for index in range(len(fault_elm_list)):
	                fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
	                print fault_desc
			        #if fault_desc == "PROCESS_FAILED":
	                if fault_desc ==expected_fault:
			           proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
			           print proc_name
			           if proc_name=="McmsDaemon" :
			              general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
			              print general_msg
			              gnrl_msg_str="Process failed : "+ProcessName
			              #if general_msg ==gnrl_msg_str :
                                      SM_fail = SM_fail+1
                                      
        if SM_fail <> expected_fault_retries_num :
		     print "Test failed,  PROCESS_FAIL not found in Faults"
		    # sys.exit(1)
        #return(0)  
         
        
#------------------------------------------------------------------------------
    def tstat_McmsDaemon(self,ProcessName):
        print "Ststat McmsDaemon:"
        SM_fail = 0
        fault_elm_list=0
        temp_str= str("Bin/ stat McmsDaemon")
        print temp_str
        os.system(temp_str)   
#------------------------------------------------------------------------------

    def test_kill_process_dbg_mode_no(self,ProcessName,expected_fault,expected_fault_retries_num):
        print " Segment fault Test ProcessName : "+ ProcessName +"("+str(expected_fault_retries_num) +")."
        SM_fail = 0
        fault_elm_list=0
        temp_str_dbg= str("Bin/McuCmd set McmsDaemon DEBUG_MODE NO")
        os.system(temp_str_dbg)
        sleep(2)
        temp_str= str("Bin/McuCmd kill " +ProcessName)
        print temp_str
        #temp_str="Bin/McuCmd segment_fault CDR" 
        os.system(temp_str)
        #os.system("Bin/McuCmd segment_fault" +ProcessName+" ")
        #if expected_fault_retries_num == 4:
        sleep (30)
        """
         #  self.SendXmlFile("Scripts/GetFaults.xml")
         #  sys.exit(0)
        self.SendXmlFile("Scripts/GetFaults.xml")
        fault_elm_list = self.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
        num_of_faults = len(fault_elm_list)
        for index in range(len(fault_elm_list)):
	                fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
	                print fault_desc
			        #if fault_desc == "PROCESS_FAILED":
	                if fault_desc ==expected_fault:
			           proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
			           print proc_name
			           if proc_name=="McmsDaemon" :
			              general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
			              print general_msg
			              gnrl_msg_str="Process failed : "+ProcessName
			              if general_msg ==gnrl_msg_str :
			                 SM_fail = SM_fail+1
        if SM_fail <> expected_fault_retries_num :
		     print "Test failed,  PROCESS_FAIL not found in Faults"
		    # sys.exit(1)
        #return(0) 
        """ 
         
        
#------------------------------------------------------------------------------

           
