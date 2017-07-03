#!/usr/bin/python
from McmsTargetConnection import *

import os, string
import sys


#from SysCfgUtils import *
#from UsersUtils import *
from McmsConnection import *


class Faults ( McmsConnection ):
#------------------------------------------------------------------------------
    def DoesFaultExist(self ,connection,expected_fault):
        fault_elm_list=0
        connection.SendXmlFile("Scripts/GetFaults.xml")
        fault_elm_list = connection.xmlResponse.getElementsByTagName("LOG_FAULT_ELEMENT")
        num_of_faults = len(fault_elm_list)
        for index in range(len(fault_elm_list)):
		        fault_desc = fault_elm_list[index].getElementsByTagName("DESCRIPTION")[0].firstChild.data
		        if fault_desc ==expected_fault:
                            return 0
                              
   #This means the fault was not found
        return 1

#------------------------------------------------------------------------------
    
