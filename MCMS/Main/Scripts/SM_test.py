#!/mcms/python/bin/python
#*PROCESSES_PROFILE_FOR_SCRIPT=Profile_6

import os
import sys
import shutil

from SysCfgUtils import *

############################################
# 
##############################################
 
c = McmsConnection()
sleep(5)
#r = McmsConnection()
c.Connect()
#c.SendXmlFile("Scripts/GetActiveAlarms.xml")
#fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
#num_of_faults = len(fault_elm_list)
SM_fail = 0
#/////////////////////////////////////Initiation///////////////////////////////////////////////// 
#for index in range(len(fault_elm_list)):
#    do
#    fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
#    print fault_desc
#    if fault_desc == "VOLTAGE_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
#        SM_fail = SM_fail+1    
#    if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
#        SM_fail = SM_fail+1  
#    if fault_desc == "FAILURE_PROBLEM":
#        SM_fail = SM_fail+1   
#    if fault_desc == "OTHER_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "VOLTAGE_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
#        SM_fail = SM_fail+1    
#    if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
#        SM_fail = SM_fail+1  
#    if fault_desc == "FAILURE_PROBLEM":
#        SM_fail = SM_fail+1   
#    if fault_desc == "OTHER_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "VOLTAGE_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
#        SM_fail = SM_fail+1    
#    if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
#        SM_fail = SM_fail+1  
#    if fault_desc == "FAILURE_PROBLEM":
#        SM_fail = SM_fail+1   
#    if fault_desc == "OTHER_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "VOLTAGE_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
#        SM_fail = SM_fail+1    
#    if fault_desc == "FAILURE_PROBLEM":
#        SM_fail = SM_fail+1  
#    if fault_desc == "OTHER_PROBLEM":
#        SM_fail = SM_fail+1   
#    if fault_desc == "VOLTAGE_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "FAILURE_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "POTHER_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "VOLTAGE_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "FAILURE_PROBLEM":
#        SM_fail = SM_fail+1
#    if fault_desc == "OTHER_PROBLEM":
#        SM_fail = SM_fail+1
#    done

if SM_fail <> 0:
    print "Test failed, VOLTAGE_PROBLEM found in Faults BEFORE the test started"
    sys.exit(1)

if SM_fail == 0:
   print "Good Start, Faults found no errors BEFORE the test started"

debug_mode_str= str("Bin/McuCmd set McmsDaemon DEBUG_MODE YES")
os.system(debug_mode_str)
#////////////////////////////////*************MFA********///////////////////////////////////////////////
SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 0 from 2 to 2 15 ")

sleep(2)
SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
#        if fault_desc == "FAILURE_PROBLEM":
#           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1

if SM_fail == 0:
    print "Test failed,  no SM MFA fail Indications found in Faults"
    sys.exit(1)

if SM_fail > 4:
    print "Test failed, More than 4 SM MFA fail Indications"
    sys.exit(1) 
    
if SM_fail == 4:
    print "Good !!! 4 SM MFA fail Indications found in Faults "

SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 0 from 2 to 2 0 ")

sleep(2)
#SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
#        if fault_desc == "FAILURE_PROBLEM":
#           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1
if SM_fail == 4:
    print "Test failed,  the 4 SM MFA fail Indications found in Active alarm, while MFA status is OK"
    sys.exit(1)

if SM_fail > 4:
    print "Test failed, more than 4 SM MFA fail Indications found in Active alarm, while MFA status is OK"
    sys.exit(1) 
    
if SM_fail == 0:
    print "Good !!! The 4 SM MFA fail  active alarm Indications has delete"
#////////////////////////////////*************FAN********///////////////////////////////////////////////
SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 0 from 4 to 4 31 ")

sleep(2)
#SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
        if fault_desc == "FAILURE_PROBLEM":
           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1

if SM_fail == 0:
    print "Test failed,  no SM FAN fail Indications found in Faults"
    sys.exit(1)

if SM_fail > 5:
    print "Test failed, More than 5 SM FAN fail Indications"
    sys.exit(1) 
    
if SM_fail == 5:
    print "Good !!! 5 SM FAN fail Indications found in Faults "

SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 0 from 4 to 4 0 ")   
sleep(2)
#SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
        if fault_desc == "FAILURE_PROBLEM":
           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1
if SM_fail == 5:
    print "Test failed,  the 5 SM FAN fail Indications found in Active alarm, while FAN status is OK"
    sys.exit(1)

if SM_fail > 5:
    print "Test failed, more than 5 SM FAN fail Indications found in Active alarm, while FAN status is OK"
    sys.exit(1) 
    
if SM_fail == 0:
    print "Good !!! The 5 SM FAN fail  active alarm Indications has delete"
#////////////////////////////////*************POWER_SUPPLY********///////////////////////////////////////////////
SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 0 from 5 to 5 31 ")

sleep(2)
#SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
        if fault_desc == "FAILURE_PROBLEM":
           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1

if SM_fail == 0:
    print "Test failed,  no SM POWER_SUPPLY fail Indications found in Faults"
    sys.exit(1)

if SM_fail > 5:
    print "Test failed, More than 5 SM POWER_SUPPLY fail Indications"
    sys.exit(1) 
    
if SM_fail == 5:
    print "Good !!! 5 SM POWER_SUPPLY fail Indications found in Faults "

SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 0 from 5 to 5 0 ")   
sleep(2)
#SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
        if fault_desc == "FAILURE_PROBLEM":
           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1
if SM_fail == 5:
    print "Test failed,  the 5 SM POWER_SUPPLY fail Indications found in Active alarm, while POWER_SUPPLY status is OK"
    sys.exit(1)

if SM_fail > 5:
    print "Test failed, more than 5 SM POWER_SUPPLY fail Indications found in Active alarm, while POWER_SUPPLY status is OK"
    sys.exit(1) 
    
if SM_fail == 0:
    print "Good !!! The 5 SM POWER_SUPPLY fail  active alarm Indications has delete"
    
#////////////////////////////////*************BACKPLANE********///////////////////////////////////////////////
SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 0 from 6 to 6 31 ")

sleep(2)
#SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
        if fault_desc == "FAILURE_PROBLEM":
           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1

if SM_fail == 0:
    print "Test failed,  no SM POWER_SUPPLY fail Indications found in Faults"
    sys.exit(1)

if SM_fail > 5:
    print "Test failed, More than 5 SM POWER_SUPPLY fail Indications"
    sys.exit(1) 
    
if SM_fail == 5:
    print "Good !!! 5 SM POWER_SUPPLY fail Indications found in Faults "

SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 0 from 6 to 6 0 ")  
sleep(2)
#SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
        if fault_desc == "FAILURE_PROBLEM":
           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1
        
if SM_fail == 5:
    print "Test failed,  the 5 SM BACKPLANE fail Indications found in Active alarm, while BACKPLANE status is OK"
    sys.exit(1)

if SM_fail > 5:
    print "Test failed, more than 5 SM BACKPLANE fail Indications found in Active alarm, while BACKPLANE status is OK"
    sys.exit(1) 
    
if SM_fail == 0:
    print "Good !!! The 5 SM BACKPLANE fail  active alarm Indications has delete"
    
    
#////////////////////////////////*************MCMS_CPU********///////////////////////////////////////////////
SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 0 from 7 to 7 31 ")

sleep(2)
#SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
        if fault_desc == "FAILURE_PROBLEM":
           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1

if SM_fail == 0:
    print "Test failed,  no SM MCMS_CPU fail Indications found in Faults"
    sys.exit(1)

if SM_fail > 5:
    print "Test failed, More than 5 SM MCMS_CPU fail Indications"
    sys.exit(1) 
    
if SM_fail == 5:
    print "Good !!! 5 SM MCMS_CPU fail Indications found in Faults "

SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 0 from 7 to 7 0 ") 
sleep(2)
#SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
        if fault_desc == "FAILURE_PROBLEM":
           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1
        
if SM_fail == 5:
    print "Test failed,  the 5 SM MCMS_CPU fail Indications found in Active alarm, while MCMS_CPU status is OK"
    sys.exit(1)

if SM_fail > 5:
    print "Test failed, more than 5 SM MCMS_CPU fail Indications found in Active alarm, while MCMS_CPU status is OK"
    sys.exit(1) 
    
if SM_fail == 0:
    print "Good !!! The 5 SM MCMS_CPU fail  active alarm Indications has delete"
 #   sys.exit(0)
    
    
 #////////////////////////////////*************SWITCH**********///////////////////////////////////////////////

print "Test SWITCH Indications...."
SM_fail = 0
fault_elm_list=0
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 0 from 1 to 1 15")

sleep(10)

c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
#        if fault_desc == "FAILURE_PROBLEM":
#           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1
            
if SM_fail == 0:
    print "Test failed,  no SM fail Indications found in Faults"
    sys.exit(1)

if SM_fail > 4:
    print "Test failed, More than 4 SM fail Indications"
    sys.exit(1) 
    
if SM_fail == 4:
    print "Good Start !!! 4 SM fail Indications found in Faults "

SM_fail = 0
fault_elm_list=0 
os.system("Scripts/Set_SM_Fatal_Failure_ind.py 0 from 1 to 1 0 ")
sleep(1)
SM_fail=0
c.SendXmlFile("Scripts/GetActiveAlarms.xml")
fault_elm_list = c.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
num_of_faults = len(fault_elm_list)
for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        print fault_desc
        if fault_desc == "VOLTAGE_PROBLEM":
           SM_fail = SM_fail+1
        if fault_desc == "TEMPERATURE_MAJOR_PROBLEM":
           SM_fail = SM_fail+1    
        if fault_desc == "TEMPERATURE_CRITICAL_PROBLEM":
           SM_fail = SM_fail+1  
#        if fault_desc == "FAILURE_PROBLEM":
#           SM_fail = SM_fail+1   
        if fault_desc == "OTHER_PROBLEM":
           SM_fail = SM_fail+1
            
if SM_fail == 4:
    print "Test failed,  the 4 SM fail Indications found in Active alarm, while switch status is OK"
    sys.exit(1)

if SM_fail > 4:
    print "Test failed, more than 4 SM fail Indications found in Active alarm, while switch status is OK"
    sys.exit(1) 
    
if SM_fail == 0:
    print "Good !!! The 4 SM fail  active alarm Indications has delete"


sys.exit(0)

 
       
