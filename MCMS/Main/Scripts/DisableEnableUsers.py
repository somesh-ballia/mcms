#!/mcms/python/bin/python

from McmsConnection import *
import os
import sys

#we send two differnt user , default is POLYCOM and the second one is SUPPORT
# this is done inorder to check System session limitation

c1 = McmsConnection()
c1.Connect()
sleep (3)

print "Disabling Auditor ..."
c1.SendXmlFile("Scripts/DisableOperator.xml")
sleep (2)

print "connecting with disabled account ..."
c2 = McmsConnection()
c2.Connect()
c2.LoadXmlFile("Scripts/login.xml")
c2.ModifyXml("LOGIN", "USER_NAME", "AUDIT")
c2.ModifyXml("LOGIN", "PASSWORD", "AUDIT")
c2.Send("Account is disabled")
c2.Disconnect()

print "Enabling Auditor ..."
print "Disabling Auditor ..."
c1.SendXmlFile("Scripts/EnableOperator.xml")
sleep (2)

print "connecting with disabled account ..."
c3 = McmsConnection()
c3.Connect()
c3.LoadXmlFile("Scripts/login.xml")
c3.ModifyXml("LOGIN", "USER_NAME", "AUDIT")
c3.ModifyXml("LOGIN", "PASSWORD", "AUDIT")
c3.Send()
c3.Disconnect()

c1.Disconnect()


