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
sleep(2)
#ProcessName = "CDR"

c.Connect()
r.Connect()
#r.AddNewUser("SUPPORT","SUPPORT","administrator")
j.Connect("SUPPORT","SUPPORT")
m = SyscfgUtilities()

m.Connect()
m.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","MAX_FAULTS_IN_LIST","10","debug")
sleep (1)
os.environ["CLEAN_CFG"]="NO"    
os.environ["CLEAN_FAULTS"]="YES"
os.system("Scripts/Startup.sh")
c.Connect()
r.Connect()
#r.AddNewUser("SUPPORT","SUPPORT","administrator")
j.Connect("SUPPORT","SUPPORT")
MAX_HLOG_LIST_NUM_OF_ELEMENTS =10
os.system("Bin/McuCmd add_fault Logger 3")
sleep(10)
os.system("Bin/McuCmd add_fault CDR 3")
sleep (10)
os.system("Bin/McuCmd add_fault Cards 3")
sleep (10)


#-------------------count the short list
count_cdr=0
count_logger=0
count_cards=0

r.SendXmlFile("Scripts/GetShortFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Logger":
           temp_msg ="- - - Test 1 - - -"
          # if general_msg== temp_msg:
           count_logger=count_logger+1
        if proc_name=="CDR":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_cdr=count_cdr+1
        if proc_name=="Cards":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_cards=count_cards+1   
print  "count_cdr: "+str(count_cdr)+"-------"               
print  "count_logger: "+str(count_logger)+"-------" 
print  "count_cards: "+str(count_cards)+"-------" 
if count_logger<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/3:
   print "Test failed, short list not updated"
   sys.exit(1)             
if count_cdr<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/3:
   print "Test failed, short list not updated"
   sys.exit(1)       
if count_cards<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/3:
   print "Test failed, short list not updated"
   sys.exit(1) 
#-------------------count the full list
count_cdr=0
count_logger=0
count_cards=0
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
          # if general_msg== temp_msg:
           count_logger=count_logger+1
        if proc_name=="CDR":
           temp_msg ="- - - Test 1 - - -"
          # if general_msg== temp_msg:
           count_cdr=count_cdr+1
        if proc_name=="Cards":
            temp_msg ="- - - Test 1 - - -"
            #if general_msg== temp_msg:
            count_cards=count_cards+1    
print  "count_cdr: "+str(count_cdr)+"-------"               
print  "count_logger: "+str(count_logger)+"-------" 
print  "count_cards: "+str(count_cards)+"-------" 
if count_logger<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/3:
   print "Test failed, full list not updated"
   sys.exit(1)             
if count_cdr<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/3:
   print "Test failed, full list not updated"
   sys.exit(1)       
if count_cards<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/3:
   print "Test failed, short list not updated"
   sys.exit(1) 
#--------------------------------------------------------------------------------------------------------------
os.environ["CLEAN_CFG"]="NO" 
os.environ["CLEAN_FAULTS"]="NO"
os.system("Scripts/Startup.sh")
c.Connect()
r.Connect()
#r.AddNewUser("SUPPORT","SUPPORT","administrator")
j.Connect("SUPPORT","SUPPORT")
MAX_HLOG_LIST_NUM_OF_ELEMENTS =10
print "------------------------------------After startup--------------------------------"
count_cdr=0
count_logger=0
count_cards=0
r.SendXmlFile("Scripts/GetShortFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Logger":
           temp_msg ="- - - Test 1 - - -"
          # if general_msg== temp_msg:
           count_logger=count_logger+1
        if proc_name=="CDR":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_cdr=count_cdr+1
        if proc_name=="Cards":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_cards=count_cards+1   
print  "count_cdr: "+str(count_cdr)+"-------"               
print  "count_logger: "+str(count_logger)+"-------" 
print  "count_cards: "+str(count_cards)+"-------" 
if count_logger<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/3:
   print "Test failed, short list not updated"
   sys.exit(1)             
if count_cdr<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/3:
   print "Test failed, short list not updated"
   sys.exit(1)       
if count_cards<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/3:
   print "Test failed, short list not updated"
   sys.exit(1) 
#-------------------count the full list
count_cdr=0
count_logger=0
count_cards=0
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
          # if general_msg== temp_msg:
           count_logger=count_logger+1
        if proc_name=="CDR":
           temp_msg ="- - - Test 1 - - -"
          # if general_msg== temp_msg:
           count_cdr=count_cdr+1
        if proc_name=="Cards":
            temp_msg ="- - - Test 1 - - -"
            #if general_msg== temp_msg:
            count_cards=count_cards+1    
print  "count_cdr: "+str(count_cdr)+"-------"               
print  "count_logger: "+str(count_logger)+"-------" 
print  "count_cards: "+str(count_cards)+"-------" 
if count_logger<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/3:
   print "Test failed, full list not updated"
   sys.exit(1)             
if count_cdr<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/3:
   print "Test failed, full list not updated"
   sys.exit(1)       
if count_cards<> MAX_HLOG_LIST_NUM_OF_ELEMENTS/3:
   print "Test failed, short list not updated"
   sys.exit(1) 

#-----------------------------------------------------------------------
os.system("Bin/McuCmd add_fault SipProxy 4")
sleep(4)

count_cdr=0
count_cards=0
count_SipProxy=0
r.SendXmlFile("Scripts/GetShortFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Cards":
           temp_msg ="- - - Test 1 - - -"
          # if general_msg== temp_msg:
           count_cards=count_cards+1
        if proc_name=="CDR":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_cdr=count_cdr+1
        if proc_name=="SipProxy":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_SipProxy=count_SipProxy+1  
print  "count_cdr: "+str(count_cdr)+"-------"               
print  "count_cards: "+str(count_cards)+"-------" 
print  "count_SipProxy: "+str(count_SipProxy)+"-------" 

if count_cards<> 3:
   print "Test failed, short list not updated"
   sys.exit(1)             
if count_cdr<> 2:
   print "Test failed, short list not updated"
   sys.exit(1)       
if count_SipProxy<> 4:
   print "Test failed, short list not updated"
   sys.exit(1) 



#-------------------count the short list
count_cdr=0
count_cards=0
count_SipProxy=0
r.SendXmlFile("Scripts/GetShortFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Cards":
           temp_msg ="- - - Test 1 - - -"
          # if general_msg== temp_msg:
           count_cards=count_cards+1
        if proc_name=="CDR":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_cdr=count_cdr+1
        if proc_name=="SipProxy":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_SipProxy=count_SipProxy+1  
print  "count_cdr: "+str(count_cdr)+"-------"               
print  "count_cards: "+str(count_cards)+"-------" 
print  "count_SipProxy: "+str(count_SipProxy)+"-------" 
if count_cards<> 3:
   print "Test failed, short list not updated"
   sys.exit(1)             
if count_cdr<> 2:
   print "Test failed, short list not updated"
   sys.exit(1)       
if count_SipProxy<> 4:
   print "Test failed, short list not updated"
   sys.exit(1) 
#-------------------count the full list
print "------------------------------------Wrong Files !!!!--------------------------------"
sleep(2)
os.system("cp Faults_0008.xml Faults/")
os.system("cp FaultsShort_0008.xml Faults/")
sleep(1)
sys.exit(0) 
#////////////////////////////////////////////////////////////////////
#--------------------------------------------------------------------------------------------------------------
os.environ["CLEAN_CFG"]="NO" 
os.environ["CLEAN_FAULTS"]="NO"
os.system("Scripts/Startup.sh")
c.Connect()
r.Connect()
#r.AddNewUser("SUPPORT","SUPPORT","administrator")
j.Connect("SUPPORT","SUPPORT")
MAX_HLOG_LIST_NUM_OF_ELEMENTS =10
print "------------------------------------After startup--------------------------------"
count_cdr=0
count_cards=0
count_SipProxy=0
r.SendXmlFile("Scripts/GetShortFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Cards":
           temp_msg ="- - - Test 1 - - -"
          # if general_msg== temp_msg:
           count_cards=count_cards+1
        if proc_name=="CDR":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_cdr=count_cdr+1
        if proc_name=="SipProxy":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_SipProxy=count_SipProxy+1  
print  "count_cdr: "+str(count_cdr)+"-------"               
print  "count_cards: "+str(count_cards)+"-------" 
print  "count_SipProxy: "+str(count_SipProxy)+"-------" 

if count_cards<> 3:
   print "Test failed, short list not updated"
   sys.exit(1)             
if count_cdr<> 1:
   print "Test failed, short list not updated"
   sys.exit(1)       
if count_SipProxy<> 4:
   print "Test failed, short list not updated"
   sys.exit(1) 



#-------------------count the short list
count_cdr=0
count_cards=0
count_SipProxy=0
r.SendXmlFile("Scripts/GetShortFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Cards":
           temp_msg ="- - - Test 1 - - -"
          # if general_msg== temp_msg:
           count_cards=count_cards+1
        if proc_name=="CDR":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_cdr=count_cdr+1
        if proc_name=="SipProxy":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_SipProxy=count_SipProxy+1  
print  "count_cdr: "+str(count_cdr)+"-------"               
print  "count_cards: "+str(count_cards)+"-------" 
print  "count_SipProxy: "+str(count_SipProxy)+"-------" 
if count_cards<> 3:
   print "Test failed, short list not updated"
   sys.exit(1)             
if count_cdr<> 1:
   print "Test failed, short list not updated"
   sys.exit(1)       
if count_SipProxy<> 4:
   print "Test failed, short list not updated"
   sys.exit(1) 
#-------------------count the full list
#/////////////////////////////////////////////////////
print "------------------------------------Add QAAPI--------------------------------"
os.system("Bin/McuCmd add_fault QAAPI 3")
sleep(4)
count_cards=0
count_SipProxy=0
count_QAAPI=0
r.SendXmlFile("Scripts/GetShortFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Cards":
           temp_msg ="- - - Test 1 - - -"
          # if general_msg== temp_msg:
           count_cards=count_cards+1
        if proc_name=="SipProxy":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_SipProxy=count_SipProxy+1  
        if proc_name=="QAAPI":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_QAAPI=count_QAAPI+1  
           
            
print  "count_cards: "+str(count_cards)+"-------" 
print  "count_SipProxy: "+str(count_SipProxy)+"-------" 
print  "count_QAAPI: "+str(count_QAAPI)+"-------" 
if count_cards<> 1:
   print "Test failed, short list not updated"
  # sys.exit(1)             
if count_QAAPI<> 3:
   print "Test failed, short list not updated"
  # sys.exit(1)       
if count_SipProxy<> 4:
   print "Test failed, short list not updated"
   #sys.exit(1) 

#//////////////////////////////////////////////////////sys.exit(0)

#-------------------count the short list
count_cards=0
count_SipProxy=0
count_QAAPI=0

r.SendXmlFile("Scripts/GetShortFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Cards":
           temp_msg ="- - - Test 1 - - -"
          # if general_msg== temp_msg:
           count_cards=count_cards+1
        if proc_name=="SipProxy":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_SipProxy=count_SipProxy+1  
        if proc_name=="QAAPI":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_QAAPI=count_QAAPI+1  
           
            
print  "count_cards: "+str(count_cards)+"-------" 
print  "count_SipProxy: "+str(count_SipProxy)+"-------" 
print  "count_QAAPI: "+str(count_QAAPI)+"-------" 
if count_cards<> 1:
   print "Test failed, short list not updated"
 #  sys.exit(1)             
if count_QAAPI<> 3:
   print "Test failed, short list not updated"
  # sys.exit(1)       
if count_SipProxy<> 4:
   print "Test failed, short list not updated"
  # sys.exit(1) 

#///////////////////////////////////////////////////////
os.environ["CLEAN_CFG"]="NO" 
os.environ["CLEAN_FAULTS"]="NO"
os.system("Scripts/Startup.sh")
c.Connect()
r.Connect()
#r.AddNewUser("SUPPORT","SUPPORT","administrator")
j.Connect("SUPPORT","SUPPORT")
MAX_HLOG_LIST_NUM_OF_ELEMENTS =10

print "------------------------------------Add QAAPI after startup--------------------------------"

sleep(4)
count_cards=0
count_SipProxy=0
count_QAAPI=0
r.SendXmlFile("Scripts/GetShortFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Cards":
           temp_msg ="- - - Test 1 - - -"
          # if general_msg== temp_msg:
           count_cards=count_cards+1
        if proc_name=="SipProxy":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_SipProxy=count_SipProxy+1  
        if proc_name=="QAAPI":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_QAAPI=count_QAAPI+1  
           
print  "--------------------------------------------------------------------------------"
print  "count_cards: "+str(count_cards)+"-------" 
print  "count_SipProxy: "+str(count_SipProxy)+"-------" 
print  "count_QAAPI: "+str(count_QAAPI)+"-------" 
#if count_cards<> 1:
#   print "Test failed, short list not updated"
#   sys.exit(1)             
if count_QAAPI<> 3:
   print "Test failed, short list not updated"
   sys.exit(1)       
if count_SipProxy<> 4:
   print "Test failed, short list not updated"
   sys.exit(1) 



#-------------------count the short list
count_cards=0
count_SipProxy=0
count_QAAPI=0

r.SendXmlFile("Scripts/GetShortFaults.xml")
fault_elm_list = r.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
        if proc_name=="Cards":
           temp_msg ="- - - Test 1 - - -"
          # if general_msg== temp_msg:
           count_cards=count_cards+1
        if proc_name=="SipProxy":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_SipProxy=count_SipProxy+1  
        if proc_name=="QAAPI":
           temp_msg ="- - - Test 1 - - -"
           #if general_msg== temp_msg:
           count_QAAPI=count_QAAPI+1  
           
            
print  "count_cards: "+str(count_cards)+"-------" 
print  "count_SipProxy: "+str(count_SipProxy)+"-------" 
print  "count_QAAPI: "+str(count_QAAPI)+"-------" 
#if count_cards<> 1:
#   print "Test failed, short list not updated"
#   sys.exit(1)             
if count_QAAPI<> 3:
   print "Test failed, short list not updated"
   sys.exit(1)       
if count_SipProxy<> 4:
   print "Test failed, short list not updated"
   sys.exit(1) 
print "Test Passed"
sys.exit(0)   



