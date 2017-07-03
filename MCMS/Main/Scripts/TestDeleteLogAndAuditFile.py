#!/mcms/python/bin/python

from McmsConnection import *
import os
import sys

c = McmsConnection()
c.Connect()
expected_status="Access denied"
c.LoadXmlFile('Scripts/TestDeleteLogFiles.xml') 
c.Send(expected_status)
c.Disconnect()

c1 = McmsConnection()
c1.Connect()
c1.LoadXmlFile('Scripts/TestDeleteLogFilesDirectory.xml') 
c1.Send(expected_status)
c1.Disconnect()

