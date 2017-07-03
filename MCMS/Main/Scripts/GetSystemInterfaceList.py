#!/mcms/python/bin/python

#Updated 13/9/2012
#By Judith

# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=Logger GideonSim MplApi CSApi EndpointsSim
#-LONG_SCRIPT_TYPE

import os
from McmsConnection import *

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()
c.SendXmlFile("Scripts/GetSystemInterfaceList.xml", "Judith")
                 
c.Disconnect()


