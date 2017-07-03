#!/usr/bin/python
from McmsTargetConnection import *
from McmsConnection import *
#from test_recovery_policy_Utilities import *
#-- SKIP_ASSERTS
#-LONG_SCRIPT_TYPE

#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_No_Valgrind

import os, string
import sys
import shutil
from RRecovery import *
from SysCfgUtils import *
from UsersUtils import *

r = UsersUtilities()
c = UsersUtilities()
j = UsersUtilities()

c.Connect()
r.Connect()

j.Connect("SUPPORT","SUPPORT")
    
os.environ["CLEAN_FAULTS"]="YES"
os.system("Scripts/Startup.sh")
c.Connect()
r.Connect()
#r.AddNewUser("SUPPORT","SUPPORT","administrator")
j.Connect("SUPPORT","SUPPORT")
MAX_HLOG_LIST_NUM_OF_ELEMENTS =10

for index in range(MAX_HLOG_LIST_NUM_OF_ELEMENTS):
    os.system("Bin/McuCmd add_fault Logger 1")
    sleep(1)
    os.system("Bin/McuCmd add_fault_full CDR 1")  
    
sleep(3)
#-------------------count the short list
r.SendXmlFile("Scripts/GetShortFaults.xml")
countshort=0
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Logger":
           temp_msg ="- - - Test 1 - - -"
           if general_msg== temp_msg:
              countshort=countshort+1
print  "countshort: "+str(countshort)+"-------" 
if countshort<> MAX_HLOG_LIST_NUM_OF_ELEMENTS:
   print "Test failed, short list not updated"
   sys.exit(1)   
print "stage 1 passed" 
#-------------------count the full list
count_cdr=0
count_logger=0
exit (0)
r.SendXmlFile("Scripts/GetFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Logger":
           temp_msg ="- - - Test 1 - - -"
           if general_msg== temp_msg:
              count_logger=count_logger+1
        if proc_name=="CDR":
           temp_msg ="- - - Test 1 - - -"
           if general_msg== temp_msg:
              count_cdr=count_cdr+1
print  "count_cdr: "+str(count_cdr)+"-------"               
print  "count_logger: "+str(count_logger)+"-------" 
if count_logger<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/2:
   print "Test failed, full list not updated"
   sys.exit(1)             
if count_cdr<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/2:
   print "Test failed, full list not updated"
   sys.exit(1)
print "stage 2 passed"              
#--------------------------------------------------------------------------------------------------------------
os.environ["CLEAN_FAULTS"]="NO"
os.system("Scripts/Startup.sh")
c.Connect()
r.Connect()
#r.AddNewUser("SUPPORT","SUPPORT","administrator")
j.Connect("SUPPORT","SUPPORT")
MAX_HLOG_LIST_NUM_OF_ELEMENTS =10

#-------------------count the short list
r.SendXmlFile("Scripts/GetShortFaults.xml")
countshort=0
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Logger":
           temp_msg ="- - - Test 1 - - -"
         #  if general_msg== temp_msg:
           countshort=countshort+1
print  "countshort: "+str(countshort)+"-------" 
if countshort< 7 :
   print "Test failed, short list not updated after startup"
   sys.exit(1)   
print "stage 3 passed" 
#-------------------count the full list
count_cdr=0
count_logger=0

r.SendXmlFile("Scripts/GetFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Logger":
           temp_msg ="- - - Test 1 - - -"
           if general_msg== temp_msg:
              count_logger=count_logger+1
        if proc_name=="CDR":
           temp_msg ="- - - Test 1 - - -"
           if general_msg== temp_msg:
              count_cdr=count_cdr+1
print  "count_cdr: "+str(count_cdr)+"-------"               
print  "count_logger: "+str(count_logger)+"-------" 
if count_logger< 4:
   print "Test failed, full list not updated"
   sys.exit(1)             
if count_cdr< 4: 
   print "Test failed, full list not updated after startup"
   sys.exit(1)          
print "test passed"        
sys.exit(0)                    





