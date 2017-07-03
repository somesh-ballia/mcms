#!/mcms/python/bin/python

from McmsConnection import *
import os
import sys

command_line = 'Bin/McuCmd set ApacheModule MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_SYSTEM 4'
os.system(command_line)
sleep(1)
command_line = 'Bin/McuCmd set ApacheModule MAX_NUMBER_OF_MANAGEMENT_SESSIONS_PER_USER 50'
os.system(command_line)
sleep(1)

#we send two differnt user , default is POLYCOM and the second one is SUPPORT
# this is done inorder to check System session limitation

c1 = McmsConnection()
c1.Connect(user="SUPPORT",password="SUPPORT")

c2 = McmsConnection()
c2.Connect(user="SUPPORT",password="SUPPORT")


c3 = McmsConnection()
c3.Connect()

c4 = McmsConnection()
c4.Connect()


c5 = McmsConnection()
c5.Connect(expected_statuses="System session limitation exceeded")

c4.Disconnect()
c3.Disconnect()
c2.Disconnect()
c1.Disconnect()
