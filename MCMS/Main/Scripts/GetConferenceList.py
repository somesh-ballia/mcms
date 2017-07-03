#!/mcms/python/bin/python

#Updated 8/2/2006
#By Yoella

# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=Logger GideonSim MplApi CSApi EndpointsSim
#-LONG_SCRIPT_TYPE

import os
from McmsConnection import *

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

for List_index in range(2000):
	c.LoadXmlFile("Scripts/GetConferenceListMonitorTemplate.xml")
	c.Send()
	
	print "GetConferenceList"+str(List_index+1)
                    
#c.Disconnect()


