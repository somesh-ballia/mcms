#!/mcms/python/bin/python

from McmsConnection import *
import os
import sys

command_line = 'Bin/McuCmd set ApacheModule MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_USER 4'
os.system(command_line)
sleep(1)
command_line = 'Bin/McuCmd set ApacheModule MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_SYSTEM 10'
os.system(command_line)
sleep(1)

c1 = McmsConnection()
c1.Connect()

c2 = McmsConnection()
c2.Connect()


c3 = McmsConnection()
c3.Connect()

c4 = McmsConnection()
c4.Connect()


c5 = McmsConnection()
c5.Connect(expected_statuses="User session limitation exceeded")

c4.Disconnect()
c3.Disconnect()
c2.Disconnect()
c1.Disconnect()

