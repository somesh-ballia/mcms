#!/mcms/python/bin/python

#Updated 11/7/2010
#By Judith

# For list of processes not for valgrind look at RunTest.sh
#*PROCESSES_NOT_FOR_VALGRIND=Logger GideonSim MplApi CSApi EndpointsSim
#-LONG_SCRIPT_TYPE

import os
from McmsConnection import *

## ---------------------- Test --------------------------
c = McmsConnection()
c.Connect()

c.LoadXmlFile("Scripts/TLS/DeleteCertificateAuthorithy.xml")
c.Send("Judith")
                 
c.Disconnect()


