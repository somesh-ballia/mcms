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

m = SyscfgUtilities()
r = UsersUtilities()
sleep(2)
r.Connect()
m.Connect()
sleep (1)
m.UpdateSyscfgFlag("MCMS_PARAMETERS_DEBUG","MAX_FAULTS_IN_LIST","10","debug")
sleep (1)
    
os.environ["CLEAN_FAULTS"]="YES"
os.environ["CLEAN_CFG"]="NO"
os.system("Scripts/Startup.sh")

r.Connect()

#------------------------------------------------------------------------------
def CheckObjToken(connection,objToken):
    #send the objToken and make sure that we get it back in the same value
    connection.LoadXmlFile("Scripts/GetShortFaults.xml") 
    connection.ModifyXml("TRANS_FAULTS_LIST_SHORT","OBJ_TOKEN",objToken)
    connection.Send()
    fault_elm_list = connection.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
    num_of_faults=0
    num_of_faults = len(fault_elm_list)
    print "num_of_faults:"+str(num_of_faults)+ "."
    responsToken = connection.xmlResponse.getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    return responsToken
#----------------------------------------------------------------------------------
#------------------------------------------------------------------------------
def CheckObjTokenAndPrint(connection,objToken,expected_num_of_faults):
    #send the objToken and make sure that we get it back in the same value
    connection.LoadXmlFile("Scripts/GetShortFaults.xml") 
    connection.ModifyXml("TRANS_FAULTS_LIST_SHORT","OBJ_TOKEN",objToken)
    connection.Send()
    fault_elm_list = connection.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
    num_of_faults=0
    num_of_faults = len(fault_elm_list)
    print "num_of_faults:"+str(num_of_faults)+ "."
    if expected_num_of_faults <> num_of_faults:
       print "Error in faults diffrent mechanism: expected_num_of_faults<>num_of_faults"
       sys.exit(1)
    for index in range(len(fault_elm_list)):
        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
        proc_name = fault_elm_list[index].getElementsByTagName("PROCESS_NAME")[0].firstChild.data
        general_msg = fault_elm_list[index].getElementsByTagName("GENERAL_MESSAGE")[0].firstChild.data
        print fault_desc+ " "+proc_name+" "+general_msg
    responsToken = connection.xmlResponse.getElementsByTagName("OBJ_TOKEN")[0].firstChild.data
    return responsToken
#----------------------------------------------------------------------------------
os.system("Bin/McuCmd add_fault Logger 3")
sleep(2)
objToken =0
print "objToken: "+str(objToken) ;
responsToken=CheckObjTokenAndPrint(r,objToken,5)
print  responsToken
if int(responsToken) <> 5:
        print "Error in faults diffrent mechanism"
        sys.exit(1)
     

objToken =1
print "objToken: "+str(objToken) ;
responsToken=CheckObjTokenAndPrint(r,objToken,4)
print  responsToken
if int(responsToken) <> 5:
        print "Error in faults diffrent mechanism"
        sys.exit(1)


sleep(4)
objToken = 4
print "objToken: "+str(objToken) ;
responsToken=CheckObjTokenAndPrint(r,objToken,1)
print  responsToken
if int(responsToken) <> 5:
        print "Error in faults diffrent mechanism"
        sys.exit(1)

os.system("Bin/McuCmd add_fault SipProxy 4")
sleep(4)
objToken =4
print "objToken: "+str(objToken) ;
responsToken=CheckObjTokenAndPrint(r,objToken,5)
if int(responsToken) <> 9:
        print "Error in faults diffrent mechanism"
        sys.exit(1)

objToken =9
print "objToken: "+str(objToken) ;
responsToken=CheckObjTokenAndPrint(r,objToken,0)
if int(responsToken) <> 9:
        print "Error in faults diffrent mechanism"
        sys.exit(1)

os.system("Bin/McuCmd add_fault CDR 3")
sleep (4)
objToken =8
print "objToken: "+str(objToken) ;
responsToken=CheckObjTokenAndPrint(r,objToken,4)
if int(responsToken) <> 12:
        print "Error in faults diffrent mechanism"
        sys.exit(1)
os.system("Bin/McuCmd add_fault Cards 3")
sleep (4)
objToken =12
print "objToken: "+str(objToken) ;
responsToken=CheckObjTokenAndPrint(r,objToken,3)
if int(responsToken) <> 15:
        print "Error in faults diffrent mechanism"
        sys.exit(1)
print "Test faults diffrent mechanism passed"      
sys.exit(0) 
           
 